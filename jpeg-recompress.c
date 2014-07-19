/*
    Recompress a JPEG file while attempting to keep visual quality the same
    by using structural similarity (SSIM) as a metric. Does a binary search
    between JPEG quality 40 and 95 to find the best match. Also makes sure
    that huffman tables are optimized if they weren't already.
*/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "src/commander.h"
#include "src/edit.h"
#include "src/iqa/include/iqa.h"
#include "src/smallfry.h"
#include "src/util.h"

const char *COMMENT = "Compressed by jpeg-recompress";

// Comparison method
enum METHOD {
    UNKNOWN,
    SSIM,
    MS_SSIM,
    SMALLFRY,
    MPE
};

int method = SSIM;

// Number of binary search steps
int attempts = 6;

// Target quality (SSIM) value
enum QUALITY_PRESET {
    LOW,
    MEDIUM,
    HIGH,
    VERYHIGH
};

float target = 0;
int preset = MEDIUM;

// Min/max JPEG quality
int jpegMin = 40;
int jpegMax = 95;

// Strip metadata from the file?
int strip = 0;

// Defish the image?
float defishStrength = 0.0;
float defishZoom = 1.0;

// Input format
int ppm = 0;

// Whether to copy files that cannot be compressed
int copyFiles = 1;

// Whether to favor accuracy over speed
int accurate = 0;

static void setAttempts(command_t *self) {
    attempts = atoi(self->arg);
}

static void setTarget(command_t *self) {
    target = atof(self->arg);
}

static void setQuality(command_t *self) {
    if (!strcmp("low", self->arg)) {
        preset = LOW;
    } else if (!strcmp("medium", self->arg)) {
        preset = MEDIUM;
    } else if (!strcmp("high", self->arg)) {
        preset = HIGH;
    } else if (!strcmp("veryhigh", self->arg)) {
        preset = VERYHIGH;
    } else {
        fprintf(stderr, "Unknown quality preset '%s'!\n", self->arg);
    }
}

static void setMethod(command_t *self) {
    if (!strcmp("ssim", self->arg)) {
        method = SSIM;
    } else if (!strcmp("ms-ssim", self->arg)) {
        method = MS_SSIM;
    } else if (!strcmp("smallfry", self->arg)) {
        method = SMALLFRY;
    } else if (!strcmp("mpe", self->arg)) {
        method = MPE;
    } else {
        method = UNKNOWN;
    }
}

static void setMinimum(command_t *self) {
    jpegMin = atoi(self->arg);
}

static void setMaximum(command_t *self) {
    jpegMax = atoi(self->arg);
}

static void setStrip(command_t *self) {
    strip = 1;
}

static void setDefish(command_t *self) {
    defishStrength = atof(self->arg);
}

static void setZoom(command_t *self) {
    defishZoom = atof(self->arg);
}

static void setPpm(command_t *self) {
    ppm = 1;
}

static void setCopyFiles(command_t *self) {
    copyFiles = 0;
}

static void setAccurate(command_t *self) {
    accurate = 1;
}

static void setTargetFromPreset() {
    switch (method) {
        case SSIM:
            switch (preset) {
                case LOW:
                    target = 0.999;
                    break;
                case MEDIUM:
                    target = 0.9999;
                    break;
                case HIGH:
                    target = 0.99995;
                    break;
                case VERYHIGH:
                    target = 0.99999;
                    break;
            }
            break;
        case MS_SSIM:
            switch (preset) {
                case LOW:
                    target = 0.85;
                    break;
                case MEDIUM:
                    target = 0.94;
                    break;
                case HIGH:
                    target = 0.96;
                    break;
                case VERYHIGH:
                    target = 0.98;
                    break;
            }
            break;
        case SMALLFRY:
            switch (preset) {
                case LOW:
                    target = 100.75;
                    break;
                case MEDIUM:
                    target = 102.25;
                    break;
                case HIGH:
                    target = 103.8;
                    break;
                case VERYHIGH:
                    target = 105.5;
                    break;
            }
            break;
        case MPE:
            switch (preset) {
                case LOW:
                    target = 1.5;
                    break;
                case MEDIUM:
                    target = 1.0;
                    break;
                case HIGH:
                    target = 0.8;
                    break;
                case VERYHIGH:
                    target = 0.6;
                    break;
            }
            break;
    }
}

// Open a file for writing
FILE *openOutput(char *name) {
    if (strcmp("-", name) == 0) {
        return stdout;
    } else {
        return fopen(name, "wb");
    }
}

int main (int argc, char **argv) {
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
    command_init(&cmd, argv[0], "1.1.0");
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
    command_option(&cmd, "-r", "--ppm", "Parse input as PPM instead of JPEG", setPpm);
    command_option(&cmd, "-c", "--no-copy", "Disable copying files that will not be compressed", setCopyFiles);
    command_parse(&cmd, argc, argv);

    if (cmd.argc < 2) {
        command_help(&cmd);
        return 255;
    }

    if (method == UNKNOWN) {
        fprintf(stderr, "Invalid method!");
        command_help(&cmd);
        return 255;
    }

    // No target passed, use preset!
    if (!target) {
        setTargetFromPreset();
    }

    // Read original
    bufSize = readFile((char *) cmd.argv[0], (void **) &buf);

    if (!bufSize) { return 1; }

    if (!ppm) {
        // Decode the JPEG
        originalSize = decodeJpeg(buf, bufSize, &original, &width, &height, JCS_RGB);
    } else {
        // Decode the PPM
        originalSize = decodePpm(buf, bufSize, &original, &width, &height);
    }

    if (defishStrength) {
        fprintf(stderr, "Defishing...\n");
        tmpImage = malloc(width * height * 3);
        defish(original, tmpImage, width, height, 3, defishStrength, defishZoom);
        free(original);
        original = tmpImage;
    }

    // Convert RGB input into Y
    originalGraySize = grayscale(original, &originalGray, width, height);

    if (!ppm) {
        // Read metadata (EXIF / IPTC / XMP tags)
        if (getMetadata(buf, bufSize, &metaBuf, &metaSize, COMMENT)) {
            fprintf(stderr, "File already processed by jpeg-recompress!\n");
            if (copyFiles) {
                file = openOutput(cmd.argv[1]);
                fwrite(buf, bufSize, 1, file);
                fclose(file);

                free(buf);

                return 0;
            } else {
                free(buf);
                return 2;
            }
        }
    }

    if (strip) {
        // Pretend we have no metadata
        metaSize = 0;
    } else {
        fprintf(stderr, "Metadata size is %ukb\n", metaSize / 1024);
    }

    if (!originalSize || !originalGraySize) { return 1; }

    // Do a binary search to find the optimal encoding quality for the
    // given target SSIM value.
    int min = jpegMin, max = jpegMax;
    for (int attempt = attempts - 1; attempt >= 0; --attempt) {
        float metric;
        int quality = min + (max - min) / 2;

        // Recompress to a new quality level, without optimizations (for speed)
        compressedSize = encodeJpeg(&compressed, original, width, height, JCS_RGB, quality, accurate ? 1 : (attempt ? 0 : 1));

        // Load compressed luma for quality comparison
        compressedGraySize = decodeJpeg(compressed, compressedSize, &compressedGray, &width, &height, JCS_GRAYSCALE);

        if (!attempt) {
            fprintf(stderr, "Final optimized ");
        }

        // Measure quality difference
        switch (method) {
            case SSIM:
                metric = iqa_ssim(originalGray, compressedGray, width, height, width, 0, 0);
                fprintf(stderr, "ssim");
                break;
            case MS_SSIM:
                metric = iqa_ms_ssim(originalGray, compressedGray, width, height, width, 0);
                fprintf(stderr, "ms-ssim");
                break;
            case SMALLFRY:
                metric = smallfry_metric(originalGray, compressedGray, width, height);
                fprintf(stderr, "smallfry");
                break;
            case MPE:
                metric = meanPixelError(originalGray, compressedGray, width, height, 1);
                fprintf(stderr, "mpe");
                break;
        }

        if (attempt) {
            fprintf(stderr, " at q=%i (%i - %i): %f\n", quality, min, max, metric);
        } else {
            fprintf(stderr, " at q=%i: %f\n", quality, metric);
        }

        if (metric < target) {
            if (compressedSize >= bufSize) {
                fprintf(stderr, "Output file would be larger than input!\n");
                free(compressed);
                free(compressedGray);

                if (copyFiles) {
                    file = openOutput(cmd.argv[1]);
                    fwrite(buf, bufSize, 1, file);
                    fclose(file);

                    free(buf);

                    return 0;
                } else {
                    free(buf);
                    return 1;
                }
            }

            switch (method) {
                case SSIM: case MS_SSIM: case SMALLFRY:
                    // Too distorted, increase quality
                    min = quality + 1;
                    break;
                case MPE:
                    // Higher than required, decrease quality
                    max = quality - 1;
                    break;
            }
        } else {
            switch (method) {
                case SSIM: case MS_SSIM: case SMALLFRY:
                    // Higher than required, decrease quality
                    max = quality - 1;
                    break;
                case MPE:
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
    fprintf(stderr, "New size is %i%% of original (saved %lu kb)\n", percent, saved / 1024);

    if (compressedSize >= bufSize) {
        fprintf(stderr, "Output file is larger than input, aborting!\n");
        return 1;
    }

    // Open output file for writing
    file = openOutput(cmd.argv[1]);

    // Write output
    fwrite(compressed, 20, 1, file); /* 0xffd8 and JFIF marker */

    // Write comment so we know not to reprocess this file
    // in the future if it gets passed in again.
    // 0xfffe (COM marker), two-byte big-endian length, string
    fputc(0xff, file);
    fputc(0xfe, file);
    fputc(0x00, file);
    fputc(32, file);
    fwrite(COMMENT, 30, 1, file);

    // Write metadata markers
    if (!strip && !ppm) {
        fwrite(metaBuf, metaSize, 1, file);
    }

    // Write image data
    fwrite(compressed + 20, compressedSize - 20, 1, file);
    fclose(file);

    // Cleanup
    command_free(&cmd);

    if (!strip && !ppm) {
        free(metaBuf);
    }

    free(compressed);
    free(original);
    free(originalGray);

    return 0;
}
