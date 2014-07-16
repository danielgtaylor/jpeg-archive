#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INPUT_BUFFER_SIZE 102400

long readFile(char *name, void **buffer) {
    FILE *file;
    size_t fileLen = 0;
    size_t bytesRead = 0;

    unsigned char chunk[INPUT_BUFFER_SIZE];

    // Open file
    if (strcmp("-", name) == 0) {
        file = stdin;
    } else {
        file = fopen(name, "rb");

        if (!file)
        {
            fprintf(stderr, "Unable to open file %s\n", name);
            return 0;
        }
    }

    *buffer = malloc(sizeof chunk);
    while ((bytesRead = fread(chunk, 1, sizeof chunk, file)) > 0) {
        unsigned char *reallocated = realloc(*buffer, fileLen + bytesRead);
        if (reallocated) {
            *buffer = reallocated;
            memmove((unsigned char *)(*buffer) + fileLen, chunk, bytesRead);
            fileLen += bytesRead;
        } else {
            fprintf(stderr, "Only able to read %zu bytes!\n", fileLen);
            free(*buffer);
            fclose(file);
            return 0;
        }
    }

    fclose(file);
    return fileLen;
}

unsigned long decodeJpegFile(const char *filename, unsigned char **image, int *width, int *height, int pixelFormat) {
    unsigned char *buf = NULL;
    long bufSize = 0;

    bufSize = readFile((char *) filename, (void **) &buf);

    if (!bufSize) { return 0; }

    return decodeJpeg(buf, bufSize, image, width, height, pixelFormat);
}

unsigned long decodeJpeg(unsigned char *buf, unsigned long bufSize, unsigned char **image, int *width, int *height, int pixelFormat) {
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    int row_stride;
    JSAMPARRAY buffer;

    cinfo.err = jpeg_std_error(&jerr);

    jpeg_create_decompress(&cinfo);

    // Set the source
    jpeg_mem_src(&cinfo, buf, bufSize);

    // Read header and set custom parameters
    jpeg_read_header(&cinfo, TRUE);

    cinfo.out_color_space = pixelFormat;

    // Start decompression
    jpeg_start_decompress(&cinfo);

    *width = cinfo.output_width;
    *height = cinfo.output_height;

    // Allocate temporary row
    row_stride = (*width) * cinfo.output_components;
    buffer = (*cinfo.mem->alloc_sarray)
        ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

    // Allocate image pixel buffer
    *image = malloc(row_stride * (*height));

    // Read image row by row
    int row = 0;
    while (cinfo.output_scanline < cinfo.output_height) {
        (void) jpeg_read_scanlines(&cinfo, buffer, 1);
        memcpy((void *)((*image) + row_stride * row), buffer[0], row_stride);
        row++;
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    return row_stride * (*height);
}

unsigned long encodeJpeg(unsigned char **jpeg, unsigned char *buf, int width, int height, int pixelFormat, int quality, int optimize) {
    long unsigned int jpegSize = 0;
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    JSAMPROW row_pointer[1];
    int row_stride = width * 3;

    cinfo.err = jpeg_std_error(&jerr);

    jpeg_create_compress(&cinfo);

    // Set destination
    jpeg_mem_dest(&cinfo, jpeg, &jpegSize);

    // Set options
    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = pixelFormat == JCS_RGB ? 3 : 1;
    cinfo.in_color_space = pixelFormat;

    if (optimize) {
        cinfo.use_moz_defaults = TRUE;
    }

    jpeg_set_defaults(&cinfo);

    jpeg_set_quality(&cinfo, quality, TRUE);

    // Start the compression
    jpeg_start_compress(&cinfo, TRUE);

    // Process scanlines one by one
    while (cinfo.next_scanline < cinfo.image_height) {
        row_pointer[0] = &buf[cinfo.next_scanline * row_stride];
        (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);

    return jpegSize;
}

unsigned long decodePpm(unsigned char *buf, unsigned long bufSize, unsigned char **image, int *width, int *height) {
    unsigned long pos = 0, imageDataSize;
    int depth;

    if (bufSize < 2 || buf[0] != 'P' || buf[1] != '6') {
        fprintf(stderr, "Not a valid PPM format image!\n");
        return 0;
    }

    // Read to first newline
    while (buf[pos++] != '\n') {}

    // Discard for any comment lines
    while (buf[pos] == '#') {
        while (buf[pos] != '\n') {
            pos++;
        }
        pos++;
    }

    // Read width/height
    sscanf((const char *) buf + pos, "%d %d", width, height);

    // Go to next line
    while (buf[pos++] != '\n') {}

    // Read bit depth
    sscanf((const char*) buf + pos, "%d", &depth);

    if (depth != 255) {
        fprintf(stderr, "Unsupported bit depth %d!\n", depth);
        return 0;
    }

    // Go to next line
    while (buf[pos++] != '\n') {}

    // Width * height * red/green/blue
    imageDataSize = (*width) * (*height) * 3;
    if (pos + imageDataSize != bufSize) {
        fprintf(stderr, "Incorrect image size! %lu vs. %lu\n", bufSize, pos + imageDataSize);
        return 0;
    }

    // Allocate image pixel buffer
    *image = malloc(imageDataSize);

    // Copy pixel data
    memcpy((void *) *image, (void *) buf + pos, imageDataSize);

    return (*width) * (*height);
}

int getMetadata(const unsigned char *buf, unsigned int bufSize, unsigned char **meta, unsigned int *metaSize, const char *comment) {
    unsigned int pos = 0;
    unsigned int totalSize = 0;
    unsigned int offsets[20];
    unsigned int sizes[20];
    unsigned int count = 0;

    // Read through all the file markers
    while (pos < bufSize && count < 20) {
        unsigned int marker = (buf[pos] << 8) + buf[pos + 1];

        //printf("Marker %x at %u\n", marker, pos);

        if (marker == 0xffda /* SOS */) {
            // This is the end, so stop!
            break;
        } else if (marker == 0xffdd /* DRI */) {
            pos += 2 + 4;
        } else if (marker >= 0xffd0 && marker <= 0xffd9 /* RST0+x */) {
            pos += 2;
        } else {
            // Marker has a custom size, read it in
            int size = (buf[pos + 2] << 8) + buf[pos + 3];
            //printf("Size is %i (%x)\n", size, size);

            // Save APP0+x and COM markers
            if ((marker >= 0xffe1 && marker <= 0xffef) || marker == 0xfffe) {
                if (marker == 0xfffe && comment != NULL && !strncmp(comment, (char *) buf + pos + 4, strlen(comment))) {
                    return 1;
                }

                totalSize += size + 2;
                offsets[count] = pos;
                sizes[count] = size + 2;
                count++;
            }

            pos += 2 + size;
        }
    }

    // Allocate the metadata buffer
    *meta = malloc(totalSize);
    *metaSize = totalSize;

    // Copy over all the metadata into the new buffer
    pos = 0;
    for (int x = 0; x < count; x++) {
        memcpy(((*meta) + pos), (buf + offsets[x]), sizes[x]);
        pos += sizes[x];
    }

    return 0;
}
