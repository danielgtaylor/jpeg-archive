
#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "edit.h"
#include "iqa/include/iqa.h"
#include "recompress.h"
#include "smallfry.h"
#include "util.h"

#ifdef _WIN32
    #include <fcntl.h>
    #include <io.h>
#endif

const char *COMMENT = "Compressed by jpeg-recompress";

// Open a file for writing
static FILE *openOutput(const char *name) {
    if (strcmp("-", name) == 0) {
        #ifdef _WIN32
            setmode(fileno(stdout), O_BINARY);
        #endif

        return stdout;
    } else {
        return fopen(name, "wb");
    }
}

// Logs an informational message, taking quiet mode into account
static void info(char quiet, const char *format, ...) {
  va_list argptr;

  if (!quiet) {
    va_start(argptr, format);
    vfprintf(stderr, format, argptr);
    fprintf(stderr, "\n");
    va_end(argptr);
  }
}

static float getTargetFromPreset(enum METHOD method,
                                 enum QUALITY_PRESET preset) {
    switch (method) {
        case METHOD_SSIM:
            switch (preset) {
                case QUALITY_LOW:
                    return 0.999;
                case QUALITY_MEDIUM:
                    return 0.9999;
                case QUALITY_HIGH:
                    return 0.99995;
                case QUALITY_VERYHIGH:
                    return 0.99999;
            }
            break;
        case METHOD_MS_SSIM:
            switch (preset) {
                case QUALITY_LOW:
                    return 0.85;
                case QUALITY_MEDIUM:
                    return 0.94;
                case QUALITY_HIGH:
                    return 0.96;
                case QUALITY_VERYHIGH:
                    return 0.98;
            }
            break;
        case METHOD_SMALLFRY:
            switch (preset) {
                case QUALITY_LOW:
                    return 100.75;
                case QUALITY_MEDIUM:
                    return 102.25;
                case QUALITY_HIGH:
                    return 103.8;
                case QUALITY_VERYHIGH:
                    return 105.5;
            }
            break;
        case METHOD_MPE:
            switch (preset) {
                case QUALITY_LOW:
                    return 1.5;
                case QUALITY_MEDIUM:
                    return 1.0;
                case QUALITY_HIGH:
                    return 0.8;
                case QUALITY_VERYHIGH:
                    return 0.6;
            }
            break;
        case METHOD_UNKNOWN:
            // Fails below.
            break;
    }
    assert(0 && "should be unreachable. All cases handled?");
    return 0.0;
}

// Constructs the error struct used for reporting and return values to the
// console. This function returns `false` as a convenience for the `recompress`
// function.
static bool err(recompress_error_t **error, const int statusCode,
                const char *format, ...) {
    va_list argptr;

    if (!error) {
        return false;
    }

    *error = malloc(sizeof(recompress_error_t));
    assert(*error);

    (*error)->statusCode = statusCode;
    va_start(argptr, format);
    int s = vsnprintf((*error)->message, ERROR_MESSAGE_MAX_LENGTH,
                      format, argptr);
    (*error)->message[s] = '\0';  // Just to be safe.
    va_end(argptr);
    return false;
}

bool recompress(const char *input, const char *output,
                const recompress_options_t *options,
                recompress_error_t **error) {
    assert(input && strlen(input) > 0);
    assert(output && strlen(output) > 0);
    assert(options);
    assert(options->method != METHOD_UNKNOWN);

    unsigned char *buf;
    long bufSize = 0;
    unsigned char *original;
    long originalSize = 0;
    unsigned char *originalGray = NULL;
    long originalGraySize = 0;
    unsigned char *compressed = NULL;
    unsigned long compressedSize = 0;
    unsigned char *compressedGray;
    long compressedGraySize = 0;
    unsigned char *tmpImage;
    int width, height;
    unsigned char *metaBuf;
    unsigned int metaSize = 0;
    FILE *file;

    bufSize = readFile(input, (void **)&buf);

    /* Detect input file type. */
    enum filetype inputFiletype = options->inputFiletype == FILETYPE_AUTO
            ? detectFiletypeFromBuffer(buf, bufSize)
            : options->inputFiletype;

    // No target passed, use preset!
    float target = !options->target
            ? getTargetFromPreset(options->method, options->preset)
            : options->target;

    /*
     * Read original image and decode. We need the raw buffer contents and
     * its size to obtain meta data and the original file size later.
     */
    originalSize = decodeFileFromBuffer(buf, bufSize, &original, inputFiletype,
                                        &width, &height, JCS_RGB);
    if (!originalSize) {
        return err(error, 1, "invalid input file: %s", input);
    }

    if (options->defishStrength) {
        info(options->quiet, "Defishing...");
        tmpImage = malloc(width * height * 3);
        assert(tmpImage);
        defish(original, tmpImage, width, height, 3, options->defishStrength,
               options->defishZoom);
        free(original);
        original = tmpImage;
    }

    // Convert RGB input into Y
    originalGraySize = grayscale(original, &originalGray, width, height);

    if (inputFiletype == FILETYPE_JPEG) {
        // Read metadata (EXIF / IPTC / XMP tags)
        if (getMetadata(buf, bufSize, &metaBuf, &metaSize, COMMENT)) {
            if (options->copyFiles) {
                info(options->quiet,
                     "File already processed by jpeg-recompress!");
                file = openOutput(output);
                if (file == NULL) {
                    return err(error, 1, "Could not open output file.");
                }

                fwrite(buf, bufSize, 1, file);
                fclose(file);

                free(buf);

                return true;
            } else {
                err(error, 2, "File already processed by jpeg-recompress!");
                free(buf);
                return false;
            }
        }
    }

    if (options->strip) {
        // Pretend we have no metadata
        metaSize = 0;
    } else {
        info(options->quiet, "Metadata size is %ukb", metaSize / 1024);
    }

    if (!originalSize || !originalGraySize) {
        return err(error, 1, "Missing original sizes");
    }

    // Do a binary search to find the optimal encoding quality for the
    // given target SSIM value.
    int min = options->jpegMin, max = options->jpegMax;
    for (int attempt = options->attempts - 1; attempt >= 0; --attempt) {
        float metric;
        int quality = min + (max - min) / 2;
        int progressive = attempt ? 0 : !options->noProgressive;
        int optimize = options->accurate ? 1 : (attempt ? 0 : 1);

        // Recompress to a new quality level, without optimizations (for speed)
        compressedSize =
            encodeJpeg(&compressed, original, width, height, JCS_RGB, quality,
                        progressive, optimize, options->subsample);

        // Load compressed luma for quality comparison
        compressedGraySize =
            decodeJpeg(compressed, compressedSize, &compressedGray, &width,
                        &height, JCS_GRAYSCALE);

        if (!compressedGraySize) {
            return err(error, 1,
                       "Unable to decode file that was just encoded!");
        }

        if (!attempt) {
            info(options->quiet, "Final optimized ");
        }

        // Measure quality difference
        switch (options->method) {
            case METHOD_MS_SSIM:
                metric = iqa_ms_ssim(originalGray, compressedGray, width,
                                    height, width, 0);
                info(options->quiet, "ms-ssim");
                break;
            case METHOD_SMALLFRY:
                metric = smallfry_metric(originalGray, compressedGray, width,
                                         height);
                info(options->quiet, "smallfry");
                break;
            case METHOD_MPE:
                metric =
                    meanPixelError(originalGray, compressedGray, width, height,
                                   1);
                info(options->quiet, "mpe");
                break;
            case METHOD_SSIM:
            default:
                metric = iqa_ssim(originalGray, compressedGray, width, height,
                                  width, 0, 0);
                info(options->quiet, "ssim");
                break;
        }

        if (attempt) {
            info(options->quiet, " at q=%i (%i - %i): %f",
                 quality, min, max, metric);
        } else {
            info(options->quiet, " at q=%i: %f", quality, metric);
        }

        if (metric < target) {
            if (compressedSize >= bufSize) {
                free(compressed);
                free(compressedGray);

                if (options->copyFiles) {
                    info(options->quiet,
                         "Output file would be larger than input!");
                    file = openOutput(output);
                    if (file == NULL) {
                        return err(error, 1, "Could not open output file.");
                    }

                    fwrite(buf, bufSize, 1, file);
                    fclose(file);

                    free(buf);

                    return true;
                } else {
                    err(error, 1, "Output file would be larger than input!");
                    free(buf);
                    return false;
                }
            }

            switch (options->method) {
                case METHOD_SSIM:
                case METHOD_MS_SSIM:
                case METHOD_SMALLFRY:
                    // Too distorted, increase quality
                    min = quality + 1;
                    break;
                case METHOD_MPE:
                    // Higher than required, decrease quality
                    max = quality - 1;
                    break;
                case METHOD_UNKNOWN:
                    // min and max values remain unchanged.
                    break;
            }
        } else {
            switch (options->method) {
                case METHOD_SSIM:
                case METHOD_MS_SSIM:
                case METHOD_SMALLFRY:
                    // Higher than required, decrease quality
                    max = quality - 1;
                    break;
                case METHOD_MPE:
                    // Too distorted, increase quality
                    min = quality + 1;
                    break;
                case METHOD_UNKNOWN:
                    // min and max values remain unchanged.
                    break;
            }
        }

        // If we aren't done yet, then free the image data
        if (attempt) {
            free(compressed);
            free(compressedGray);
        }
    }

    free(buf);

    // Calculate and show savings, if any
    int percent = (compressedSize + metaSize) * 100 / bufSize;
    unsigned long saved =
        (bufSize > compressedSize) ? bufSize - compressedSize - metaSize : 0;
    info(options->quiet, "New size is %i%% of original (saved %lu kb)",
         percent, saved / 1024);

    if (compressedSize >= bufSize) {
        return err(error, 1, "Output file is larger than input, aborting!");
    }

    // Open output file for writing
    file = openOutput(output);
    if (file == NULL) {
        return err(error, 1, "Could not open output file.");
    }

    /* Check that the metadata starts with a SOI marker. */
    if (!checkJpegMagic(compressed, compressedSize)) {
        return err(error, 1, "Missing SOI marker, aborting!");
    }

    /* Make sure APP0 is recorded immediately after the SOI marker. */
    if (compressed[2] != 0xff || compressed[3] != 0xe0) {
        return err(error, 1, "Missing APP0 marker, aborting!");
    }

    /* Write SOI marker and APP0 metadata to the output file. */
    int app0_len = (compressed[4] << 8) + compressed[5];
    fwrite(compressed, 4 + app0_len, 1, file);

    /*
     * Write comment (COM metadata) so we know not to reprocess this file in
     * the future if it gets passed in again.
     */
    fputc(0xff, file);
    fputc(0xfe, file);
    fputc(0x00, file);
    fputc(strlen(COMMENT) + 2, file);
    fwrite(COMMENT, strlen(COMMENT), 1, file);

    /* Write additional metadata markers. */
    if (inputFiletype == FILETYPE_JPEG && !options->strip) {
        fwrite(metaBuf, metaSize, 1, file);
    }

    /* Write image data. */
    fwrite(compressed + 4 + app0_len, compressedSize - 4 - app0_len, 1, file);
    fclose(file);

    if (inputFiletype == FILETYPE_JPEG && !options->strip) {
        free(metaBuf);
    }

    free(compressed);
    free(original);
    free(originalGray);

    return true;
}
