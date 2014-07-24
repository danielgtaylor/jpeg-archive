/*
    Utility functions
*/
#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <sys/types.h>
#include <jpeglib.h>

/*
    Read a file into a buffer and return the length.
*/
long readFile(char *name, void **buffer);

/*
    Decode a file or buffer into a JPEG image with the given pixel format.
    Returns the size of the image pixel array.
    See libjpeg.txt for a (very long) explanation.
*/
unsigned long decodeJpegFile(const char *filename, unsigned char **image, int *width, int *height, int pixelFormat);
unsigned long decodeJpeg(unsigned char *buf, unsigned long bufSize, unsigned char **image, int *width, int *height, int pixelFormat);

/*
    Decode a file or buffer into a PPM image.
    Returns the size of the image pixel array.
*/
unsigned long decodePpmFile(const char *filename, unsigned char **image, int *width, int *height);
unsigned long decodePpm(unsigned char *buf, unsigned long bufSize, unsigned char **image, int *width, int *height);

/*
    Encode a buffer of image pixels into a JPEG.
*/
unsigned long encodeJpeg(unsigned char **jpeg, unsigned char *buf, int width, int height, int pixelFormat, int quality, int progressive, int optimize);

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
