/*
    Recompress JPEG images
*/
#ifndef RECOMPRESS_H
#define RECOMPRESS_H

#include <stdbool.h>

#include "util.h"

#define ERROR_MESSAGE_MAX_LENGTH 1024

// Comparison method
enum METHOD {
    METHOD_UNKNOWN,
    METHOD_SSIM,
    METHOD_MS_SSIM,
    METHOD_SMALLFRY,
    METHOD_MPE
};

// Target quality (SSIM) value
enum QUALITY_PRESET {
    QUALITY_LOW,
    QUALITY_MEDIUM,
    QUALITY_HIGH,
    QUALITY_VERYHIGH
};

/*
 *  Options used to recompress images.
 */

typedef struct {
    enum METHOD method;
    // Number of binary search steps
    int attempts;
    float target;
    enum QUALITY_PRESET preset;
    // Min/max JPEG quality
    int jpegMin;
    int jpegMax;
    // Strip metadata from the file?
    bool strip;
    // Disable progressive mode?
    bool noProgressive;
    // Defish the image?
    float defishStrength;
    float defishZoom;
    // Input format
    enum filetype inputFiletype;
    // Whether to copy files that cannot be compressed
    bool copyFiles;
    // Whether to favor accuracy over speed
    bool accurate;
    // Chroma subsampling method
    enum SUBSAMPLING_METHOD subsample;
    // Quiet mode (less output)
    bool quiet;
} recompress_options_t;

/*
 * Error information for a failed recompress.
 */

typedef struct {
    int statusCode;
    char message[ERROR_MESSAGE_MAX_LENGTH];
} recompress_error_t;

/*
    Recompress a JPEG file while attempting to keep visual quality the same
    by using structural similarity (SSIM) as a metric. Does a binary search
    between JPEG quality 40 and 95 to find the best match. Also makes sure
    that huffman tables are optimized if they weren't already.
*/

bool recompress(const char *input, const char *output,
                const recompress_options_t *options,
                recompress_error_t **error);

#endif
