/*
    Recompress a JPEG file while attempting to keep visual quality the same
    by using structural similarity (SSIM) as a metric. Does a binary search
    between JPEG quality 40 and 95 to find the best match. Also makes sure
    that huffman tables are optimized if they weren't already.
*/
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "src/commander.h"
#include "src/edit.h"
#include "src/iqa/include/iqa.h"
#include "src/recompress.h"
#include "src/smallfry.h"
#include "src/util.h"

#ifdef _WIN32
    #include <io.h>
    #include <fcntl.h>
#endif

const char *COMMENT = "Compressed by jpeg-recompress";

recompress_options_t options = {
    .method = METHOD_SSIM,
    .attempts = 6,
    .target = 0,
    .preset = QUALITY_MEDIUM,
    .jpegMin = 40,
    .jpegMax = 95,
    .strip = false,
    .noProgressive = false,
    .defishStrength = 0.0,
    .defishZoom = 1.0,
    .inputFiletype = FILETYPE_AUTO,
    .copyFiles = true,
    .accurate = false,
    .subsample = SUBSAMPLE_DEFAULT,
    .quiet = false
};

static void setAttempts(command_t *self) {
    options.attempts = atoi(self->arg);
}

static void setTarget(command_t *self) {
    options.target = atof(self->arg);
}

static void setQuality(command_t *self) {
    if (!strcmp("low", self->arg)) {
        options.preset = QUALITY_LOW;
    } else if (!strcmp("medium", self->arg)) {
        options.preset = QUALITY_MEDIUM;
    } else if (!strcmp("high", self->arg)) {
        options.preset = QUALITY_HIGH;
    } else if (!strcmp("veryhigh", self->arg)) {
        options.preset = QUALITY_VERYHIGH;
    } else {
        fprintf(stderr, "Unknown quality preset '%s'!\n", self->arg);
    }
}

static void setMethod(command_t *self) {
    if (!strcmp("ssim", self->arg)) {
        options.method = METHOD_SSIM;
    } else if (!strcmp("ms-ssim", self->arg)) {
        options.method = METHOD_MS_SSIM;
    } else if (!strcmp("smallfry", self->arg)) {
        options.method = METHOD_SMALLFRY;
    } else if (!strcmp("mpe", self->arg)) {
        options.method = METHOD_MPE;
    } else {
        options.method = METHOD_UNKNOWN;
    }
}

static void setNoProgressive(command_t *self) {
    options.noProgressive = true;
}

static void setMinimum(command_t *self) {
    options.jpegMin = atoi(self->arg);
}

static void setMaximum(command_t *self) {
    options.jpegMax = atoi(self->arg);
}

static void setStrip(command_t *self) {
    options.strip = true;
}

static void setDefish(command_t *self) {
    options.defishStrength = atof(self->arg);
}

static void setZoom(command_t *self) {
    options.defishZoom = atof(self->arg);
}

static void setInputFiletype(command_t *self) {
    if (!strcmp("auto", self->arg))
        options.inputFiletype = FILETYPE_AUTO;
    else if (!strcmp("jpeg", self->arg))
        options.inputFiletype = FILETYPE_JPEG;
    else if (!strcmp("ppm", self->arg))
        options.inputFiletype = FILETYPE_PPM;
    else
        options.inputFiletype = FILETYPE_UNKNOWN;
}

static void setPpm(command_t *self) {
    options.inputFiletype = FILETYPE_PPM;
}

static void setCopyFiles(command_t *self) {
    options.copyFiles = false;
}

static void setAccurate(command_t *self) {
    options.accurate = true;
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
    }
    assert(("should be unreachable. All cases handled?",0));
    return 0.0;
}

static void setSubsampling(command_t *self) {
    if (!strcmp("default", self->arg)) {
        options.subsample = SUBSAMPLE_DEFAULT;
    } else if (!strcmp("disable", self->arg)) {
        options.subsample = SUBSAMPLE_444;
    } else {
        fprintf(stderr, "Unknown sampling method '%s', using default!\n", self->arg);
    }
}

static void setQuiet(command_t *self) {
    options.quiet = true;
}

// Open a file for writing
FILE *openOutput(char *name) {
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
void info(const char *format, ...) {
    va_list argptr;

    if (!options.quiet) {
        va_start(argptr, format);
        vfprintf(stderr, format, argptr);
        va_end(argptr);
    }
}

int main (int argc, char **argv) {
    enum filetype inputFiletype = FILETYPE_UNKNOWN;
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

    // Parse commandline options
    command_t cmd;
    command_init(&cmd, argv[0], VERSION);
    cmd.usage = "[options] input.jpg compressed-output.jpg";
    command_option(&cmd, "-t", "--target [arg]", "Set target quality [0.9999]", setTarget);
    command_option(&cmd, "-q", "--quality [arg]", "Set a quality preset: low, medium, high, veryhigh [medium]", setQuality);
    command_option(&cmd, "-n", "--min [arg]", "Minimum JPEG quality [40]", setMinimum);
    command_option(&cmd, "-x", "--max [arg]", "Maximum JPEG quality [95]", setMaximum);
    command_option(&cmd, "-l", "--loops [arg]", "Set the number of runs to attempt [6]", setAttempts);
    command_option(&cmd, "-a", "--accurate", "Favor accuracy over speed", setAccurate);
    command_option(&cmd, "-m", "--method [arg]", "Set comparison method to one of 'mpe', 'ssim', 'ms-ssim', 'smallfry' [ssim]", setMethod);
    command_option(&cmd, "-s", "--strip", "Strip metadata", setStrip);
    command_option(&cmd, "-d", "--defish [arg]", "Set defish strength [0.0]", setDefish);
    command_option(&cmd, "-z", "--zoom [arg]", "Set defish zoom [1.0]", setZoom);
    command_option(&cmd, "-r", "--ppm", "Parse input as PPM", setPpm);
    command_option(&cmd, "-c", "--no-copy", "Disable copying files that will not be compressed", setCopyFiles);
    command_option(&cmd, "-p", "--no-progressive", "Disable progressive encoding", setNoProgressive);
    command_option(&cmd, "-S", "--subsample [arg]", "Set subsampling method. Valid values: 'default', 'disable'. [default]", setSubsampling);
    command_option(&cmd, "-T", "--input-filetype [arg]", "Set input file type to one of 'auto', 'jpeg', 'ppm' [auto]", setInputFiletype);
    command_option(&cmd, "-Q", "--quiet", "Only print out errors.", setQuiet);
    command_parse(&cmd, argc, argv);

    if (cmd.argc < 2) {
        command_help(&cmd);
        return 255;
    }

    if (options.method == METHOD_UNKNOWN) {
        fprintf(stderr, "Invalid method!");
        command_help(&cmd);
        return 255;
    }

    // No target passed, use preset!
    if (!options.target) {
        options.target = getTargetFromPreset(options.method, options.preset);
    }

    /* Detect input file type. */
    if (options.inputFiletype == FILETYPE_AUTO) {
        inputFiletype = detectFiletype((char *)cmd.argv[0]);
    } else {
        inputFiletype = options.inputFiletype;
    }

    /*
     * Read original image and decode. We need the raw buffer contents and its
     * size to obtain meta data and the original file size later.
     */
    bufSize = readFile((char *) cmd.argv[0], (void **) &buf);
    originalSize = decodeFile((char *) cmd.argv[0], &original, inputFiletype, &width, &height, JCS_RGB);
    if (!originalSize) {
        fprintf(stderr, "invalid input file: %s\n", cmd.argv[0]);
        return 1;
    }

    if (options.defishStrength) {
        info("Defishing...\n");
        tmpImage = malloc(width * height * 3);
        defish(original, tmpImage, width, height, 3, options.defishStrength,
               options.defishZoom);
        free(original);
        original = tmpImage;
    }

    // Convert RGB input into Y
    originalGraySize = grayscale(original, &originalGray, width, height);

    if (inputFiletype == FILETYPE_JPEG) {
        // Read metadata (EXIF / IPTC / XMP tags)
        if (getMetadata(buf, bufSize, &metaBuf, &metaSize, COMMENT)) {
            if (options.copyFiles) {
                info("File already processed by jpeg-recompress!\n");
                file = openOutput(cmd.argv[1]);
                if (file == NULL) {
                    fprintf(stderr, "Could not open output file.");
                    return 1;
                }

                fwrite(buf, bufSize, 1, file);
                fclose(file);

                free(buf);

                return 0;
            } else {
                fprintf(stderr, "File already processed by jpeg-recompress!\n");
                free(buf);
                return 2;
            }
        }
    }

    if (options.strip) {
        // Pretend we have no metadata
        metaSize = 0;
    } else {
        info("Metadata size is %ukb\n", metaSize / 1024);
    }

    if (!originalSize || !originalGraySize) { return 1; }

    // Do a binary search to find the optimal encoding quality for the
    // given target SSIM value.
    int min = options.jpegMin, max = options.jpegMax;
    for (int attempt = options.attempts - 1; attempt >= 0; --attempt) {
        float metric;
        int quality = min + (max - min) / 2;
        int progressive = attempt ? 0 : !options.noProgressive;
        int optimize = options.accurate ? 1 : (attempt ? 0 : 1);

        // Recompress to a new quality level, without optimizations (for speed)
        compressedSize = encodeJpeg(&compressed, original, width, height,
                                    JCS_RGB, quality, progressive, optimize,
                                    options.subsample);

        // Load compressed luma for quality comparison
        compressedGraySize = decodeJpeg(compressed, compressedSize, &compressedGray, &width, &height, JCS_GRAYSCALE);

        if (!compressedGraySize) {
          fprintf(stderr, "Unable to decode file that was just encoded!\n");
          return 1;
        }

        if (!attempt) {
            info("Final optimized ");
        }

        // Measure quality difference
        switch (options.method) {
            case METHOD_MS_SSIM:
                metric = iqa_ms_ssim(originalGray, compressedGray, width, height, width, 0);
                info("ms-ssim");
                break;
            case METHOD_SMALLFRY:
                metric = smallfry_metric(originalGray, compressedGray, width, height);
                info("smallfry");
                break;
            case METHOD_MPE:
                metric = meanPixelError(originalGray, compressedGray, width, height, 1);
                info("mpe");
                break;
            case METHOD_SSIM: default:
                metric = iqa_ssim(originalGray, compressedGray, width, height, width, 0, 0);
                info("ssim");
                break;
        }

        if (attempt) {
            info(" at q=%i (%i - %i): %f\n", quality, min, max, metric);
        } else {
            info(" at q=%i: %f\n", quality, metric);
        }

        if (metric < options.target) {
            if (compressedSize >= bufSize) {
                free(compressed);
                free(compressedGray);

                if (options.copyFiles) {
                    info("Output file would be larger than input!\n");
                    file = openOutput(cmd.argv[1]);
                    if (file == NULL) {
                        fprintf(stderr, "Could not open output file.");
                        return 1;
                    }

                    fwrite(buf, bufSize, 1, file);
                    fclose(file);

                    free(buf);

                    return 0;
                } else {
                    fprintf(stderr, "Output file would be larger than input!\n");
                    free(buf);
                    return 1;
                }
            }

            switch (options.method) {
                case METHOD_SSIM: case METHOD_MS_SSIM: case METHOD_SMALLFRY:
                    // Too distorted, increase quality
                    min = quality + 1;
                    break;
                case METHOD_MPE:
                    // Higher than required, decrease quality
                    max = quality - 1;
                    break;
            }
        } else {
            switch (options.method) {
                case METHOD_SSIM: case METHOD_MS_SSIM: case METHOD_SMALLFRY:
                    // Higher than required, decrease quality
                    max = quality - 1;
                    break;
                case METHOD_MPE:
                    // Too distorted, increase quality
                    min = quality + 1;
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
    unsigned long saved = (bufSize > compressedSize) ? bufSize - compressedSize - metaSize : 0;
    info("New size is %i%% of original (saved %lu kb)\n", percent, saved / 1024);

    if (compressedSize >= bufSize) {
        fprintf(stderr, "Output file is larger than input, aborting!\n");
        return 1;
    }

    // Open output file for writing
    file = openOutput(cmd.argv[1]);
    if (file == NULL) {
        fprintf(stderr, "Could not open output file.");
        return 1;
    }

    /* Check that the metadata starts with a SOI marker. */
    if (!checkJpegMagic(compressed, compressedSize)) {
        fprintf(stderr, "Missing SOI marker, aborting!\n");
        return 1;
    }

    /* Make sure APP0 is recorded immediately after the SOI marker. */
    if (compressed[2] != 0xff || compressed[3] != 0xe0) {
        fprintf(stderr, "Missing APP0 marker, aborting!\n");
        return 1;
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
    if (inputFiletype == FILETYPE_JPEG && !options.strip) {
        fwrite(metaBuf, metaSize, 1, file);
    }

    /* Write image data. */
    fwrite(compressed + 4 + app0_len, compressedSize - 4 - app0_len, 1, file);
    fclose(file);

    /* Cleanup. */
    command_free(&cmd);

    if (inputFiletype == FILETYPE_JPEG && !options.strip) {
        free(metaBuf);
    }

    free(compressed);
    free(original);
    free(originalGray);

    return 0;
}
