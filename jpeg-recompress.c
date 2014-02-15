/*
    Recompress a JPEG file while attempting to keep visual quality the same
    by using structural similarity (SSIM) as a metric. Does a binary search
    between JPEG quality 40 and 95 to find the best match. Also makes sure
    that huffman tables are optimized if they weren't already.

    TODO: Include jpegrescan functionality? It doesn't save much...
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "src/commander.h"
#include "src/edit.h"
#include "src/iqa/include/iqa.h"
#include "src/util.h"

const char *COMMENT = "Compressed by jpeg-recompress";

// Number of binary search steps
int attempts = 6;

// Target quality (SSIM) value
float target = 0.9999;

// Min/max JPEG quality
int jpegMin = 40;
int jpegMax = 95;

// Progressive image output
int progressive = 0;

// Strip metadata from the file?
int strip = 0;

// Defish the image?
float defishStrength = 0.0;
float defishZoom = 1.0;

static void setAttempts(command_t *self) {
    attempts = atoi(self->arg);
}

static void setTarget(command_t *self) {
    target = atof(self->arg);
}

static void setQuality(command_t *self) {
    if (!strcmp("low", self->arg)) {
        target = 0.999;
    } else if (!strcmp("medium", self->arg)) {
        target = 0.9999;
    } else if (!strcmp("high", self->arg)) {
        target = 0.99995;
    } else if (!strcmp("veryhigh", self->arg)) {
        target = 0.99999;
    } else {
        fprintf(stderr, "Unknown quality preset '%s'!\n", self->arg);
    }
}

static void setMinimum(command_t *self) {
    jpegMin = atoi(self->arg);
}

static void setMaximum(command_t *self) {
    jpegMax = atoi(self->arg);
}

static void setProgressive(command_t *self) {
    progressive = 1;
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

int main (int argc, char **argv) {
    unsigned char *buf;
    long bufSize = 0;
    unsigned char *original;
    long originalSize = 0;
    unsigned char *originalGray;
    long originalGraySize = 0;
    unsigned char *compressed = NULL;
    unsigned long compressedSize = 0;
    unsigned char *compressedGray;
    long compressedGraySize = 0;
    unsigned char *tmpImage;
    int width, height;
    unsigned char *metaBuf;
    unsigned int metaSize = 0;

    // Parse commandline options
    command_t cmd;
    command_init(&cmd, argv[0], "1.0.1");
    cmd.usage = "[options] input.jpg compressed-output.jpg";
    command_option(&cmd, "-t", "--target [arg]", "Set target SSIM [0.9999]", setTarget);
    command_option(&cmd, "-q", "--quality [arg]", "Set a quality preset: low, medium, high, veryhigh [medium]", setQuality);
    command_option(&cmd, "-n", "--min [arg]", "Minimum JPEG quality [40]", setMinimum);
    command_option(&cmd, "-m", "--max [arg]", "Maximum JPEG quality [95]", setMaximum);
    command_option(&cmd, "-l", "--loops [arg]", "Set the number of runs to attempt [6]", setAttempts);
    command_option(&cmd, "-p", "--progressive", "Set progressive JPEG output", setProgressive);
    command_option(&cmd, "-s", "--strip", "Strip metadata", setStrip);
    command_option(&cmd, "-d", "--defish [arg]", "Set defish strength [0.0]", setDefish);
    command_option(&cmd, "-z", "--zoom [arg]", "Set defish zoom [1.0]", setZoom);
    command_parse(&cmd, argc, argv);

    if (cmd.argc < 2) {
        command_help(&cmd);
        return 255;
    }

    // Read original
    bufSize = readFile((char *) cmd.argv[0], (void **) &buf);

    if (!bufSize) { return 1; }

    // Decode the JPEG
    originalSize = decodeJpeg(buf, bufSize, &original, &width, &height, JCS_RGB);

    if (defishStrength) {
        fprintf(stderr, "Defishing...\n");
        tmpImage = malloc(width * height * 3);
        defish(original, tmpImage, width, height, 3, defishStrength, defishZoom);
        free(original);
        original = tmpImage;
    }

    // Convert RGB input into Y
    originalGraySize = width * height;
    originalGray = malloc(originalGraySize);
    int stride = width * 3;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Y = 0.299R + 0.587G + 0.114B
            originalGray[y * width + x] = original[y * stride + x * 3] * 0.299 +
                                          original[y * stride + x * 3 + 1] * 0.587 +
                                          original[y * stride + x * 3 + 2] * 0.114 + 0.5;
        }
    }

    // Read metadata (EXIF / IPTC / XMP tags)
    if (getMetadata(buf, bufSize, &metaBuf, &metaSize, COMMENT)) {
        fprintf(stderr, "File already processed by jpeg-recompress!\n");
        return 2;
    }

    if (strip) {
        // Pretend we have no metadata
        metaSize = 0;
    } else {
        fprintf(stderr, "Metadata size is %ukb\n", metaSize / 1024);
    }

    if (!originalSize || !originalGraySize) { return 1; }
    free(buf);

    // Do a binary search to find the optimal encoding quality for the
    // given target SSIM value.
    int min = jpegMin, max = jpegMax;
    for (int attempt = attempts - 1; attempt >= 0; --attempt) {
        int quality = min + (max - min) / 2;

        // Recompress to a new quality level (progressive only on last run if
        // it was requested, as this saves time)
        compressedSize = encodeJpeg(&compressed, original, width, height, JCS_RGB, quality, (attempt == 0) ? progressive : 0);

        // Load compressed luma for quality comparison
        compressedGraySize = decodeJpeg(compressed, compressedSize, &compressedGray, &width, &height, JCS_GRAYSCALE);

        // Measure structural similarity (SSIM)
        float ssim = iqa_ssim(originalGray, compressedGray, width, height, width, 0, 0);
        fprintf(stderr, "ssim at q=%i (%i - %i): %f\n", quality, min, max, ssim);

        if (ssim < target) {
            if (compressedSize >= bufSize) {
                fprintf(stderr, "Output file would be larger than input, aborting!\n");
                free(compressed);
                free(compressedGray);
                return 1;
            }

            // Too distorted, increase quality
            min = quality + 1;
        } else {
            // Higher SSIM than required, decrease quality
            max = quality - 1;
        }

        // If we aren't done yet, then free the image data
        if (attempt) {
            free(compressed);
            free(compressedGray);
        }
    }

    // Calculate and show savings, if any
    int percent = (compressedSize + metaSize) * 100 / bufSize;
    unsigned long saved = (bufSize > compressedSize) ? bufSize - compressedSize - metaSize : 0;
    fprintf(stderr, "New size is %i%% of original (saved %lu kb)\n", percent, saved / 1024);

    if (compressedSize >= bufSize) {
        fprintf(stderr, "Output file is larger than input, aborting!\n");
        return 1;
    }

    // Write output
    FILE *file;

    if (strcmp("-", cmd.argv[1]) == 0) {
        file = stdout;
    } else {
        file = fopen(cmd.argv[1], "wb");
    }

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
    if (!strip) {
        fwrite(metaBuf, metaSize, 1, file);
    }

    // Write image data
    fwrite(compressed + 20, compressedSize - 20, 1, file);
    fclose(file);

    // Cleanup
    command_free(&cmd);

    if (!strip) {
        free(metaBuf);
    }

    free(compressed);
    free(original);
    free(originalGray);
    free(compressedGray);

    return 0;
}
