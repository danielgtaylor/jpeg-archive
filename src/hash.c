#include <stdlib.h>

#include "hash.h"
#include "util.h"

void scale(unsigned char *image, int width, int height, unsigned char **newImage, int newWidth, int newHeight) {
    *newImage = malloc((unsigned long) newWidth * newHeight);

    for (int y = 0; y < newHeight; y++) {
        for (int x = 0; x < newWidth; x++) {
            int oldY = (float) y / newHeight * height + 0.5;
            int oldX = (float) x / newWidth * width + 0.5;

            (*newImage)[y * newWidth + x] = image[oldY * width + oldX];

        }
    }
}

void genHash(unsigned char *image, int width, int height, unsigned char **hash) {
    *hash = malloc((unsigned long) width * height);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int pos = y * width + x;

            (*hash)[pos] = image[pos] < image[pos + 1];
        }
    }
}

int jpegHash(const char *filename, unsigned char **hash, int size) {
    unsigned char *buf;
    long bufSize = 0;
    unsigned char *image;
    unsigned long imageSize = 0;
    unsigned char *scaled;
    int width, height;

    bufSize = readFile((char *) filename, (void **) &buf);

    if (!bufSize) { return 1; }

    imageSize = decodeJpeg(buf, bufSize, &image, &width, &height, JCS_GRAYSCALE);

    if (!imageSize) { return 1; }
    free(buf);

    scale(image, width, height, &scaled, size, size);

    free(image);

    genHash(scaled, size, size, hash);

    free(scaled);

    return 0;
}

unsigned int hammingDist(const unsigned char *hash1, const unsigned char *hash2, int hashLength) {
    unsigned int dist = 0;

    for (unsigned int x = 0; x < hashLength; x++) {
        if (hash1[x] != hash2[x]) {
            dist++;
        }
    }

    return dist;
}
