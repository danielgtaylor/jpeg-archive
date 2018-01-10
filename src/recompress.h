/*
    Recompress JPEG images
*/
#ifndef RECOMPRESS_H
#define RECOMPRESS_H

#include <stdbool.h>

#include "util.h"

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

#endif
