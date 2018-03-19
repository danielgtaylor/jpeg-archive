/*
    Utility functions
*/
#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <sys/types.h>
#include <jpeglib.h>

extern const char *VERSION;

// Subsampling method, which defines how much of the data from
// each color channel is included in the image per 2x2 block.
// A value of 4 means all four pixels are included, while 2
// means that only two of the four are included (hence the term
// subsampling). Subsampling works really well for photos, but
// can have issues with crisp colored borders (e.g. red text).
enum SUBSAMPLING_METHOD {
    // Default is 4:2:0
    SUBSAMPLE_DEFAULT,
    // Using 4:4:4 is more detailed and will prevent fine text
    // from getting blurry (e.g. screenshots)
    SUBSAMPLE_444
};

enum filetype {
    FILETYPE_UNKNOWN,
    FILETYPE_AUTO,
    FILETYPE_JPEG,
    FILETYPE_PPM
};

/*
    Read a file into a buffer and return the length.
*/
long readFile(const char *name, void **buffer);

/*
    Decode a buffer into a JPEG image with the given pixel format.
    Returns the size of the image pixel array.
    See libjpeg.txt for a (very long) explanation.
*/
int checkJpegMagic(const unsigned char *buf, unsigned long size);
unsigned long decodeJpeg(unsigned char *buf, unsigned long bufSize, unsigned char **image, int *width, int *height, int pixelFormat);

/*
    Decode buffer into a PPM image.
    Returns the size of the image pixel array.
*/
int checkPpmMagic(const unsigned char *buf, unsigned long size);
unsigned long decodePpm(unsigned char *buf, unsigned long bufSize, unsigned char **image, int *width, int *height);

/*
    Encode a buffer of image pixels into a JPEG.
*/
unsigned long encodeJpeg(unsigned char **jpeg, unsigned char *buf, int width, int height, int pixelFormat, int quality, int progressive, int optimize, int subsample);

/* Automatically detect the file type of a given file. */
enum filetype detectFiletype(const char *filename);
enum filetype detectFiletypeFromBuffer(unsigned char *buf, long bufSize);

/* Decode an image file with a given format. */
unsigned long decodeFile(const char *filename, unsigned char **image, enum filetype type, int *width, int *height, int pixelFormat);
unsigned long decodeFileFromBuffer(unsigned char *buf, long bufSize, unsigned char **image, enum filetype type, int *width, int *height, int pixelFormat);

/*
    Get JPEG metadata (EXIF, IPTC, XMP, etc) and return a buffer
    with just this data, suitable for writing out to a new file.
    Reads in all APP0-APP15 markers as well as COM markers.
    Reads up to 20 markers.

    If comment is not NULL, then returns 1 if the comment is
    encountered, allowing scripts to detect if they have previously
    modified the file.
*/
int getMetadata(const unsigned char *buf, unsigned int bufSize, unsigned char **meta, unsigned int *metaSize, const char *comment);

#endif
