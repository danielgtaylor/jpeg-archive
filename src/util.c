#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    #include <io.h>
    #include <fcntl.h>
#endif

#define INPUT_BUFFER_SIZE 102400

const char *VERSION = "2.1.1";

long readFile(const char *name, void **buffer) {
    FILE *file;
    size_t fileLen = 0;
    size_t bytesRead = 0;

    unsigned char chunk[INPUT_BUFFER_SIZE];

    // Open file
    if (strcmp("-", name) == 0) {
        file = stdin;

        #ifdef _WIN32
            // We need to use binary mode on Windows otherwise we get
            // corrupted files. See this issue:
            // https://github.com/danielgtaylor/jpeg-archive/issues/14
            setmode(fileno(stdin), O_BINARY);
        #endif
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

int checkJpegMagic(const unsigned char *buf, unsigned long size) {
    return (size >= 2 && buf[0] == 0xff && buf[1] == 0xd8);
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

unsigned long encodeJpeg(unsigned char **jpeg, unsigned char *buf, int width, int height, int pixelFormat, int quality, int progressive, int optimize, int subsample) {
    long unsigned int jpegSize = 0;
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    JSAMPROW row_pointer[1];
    int row_stride = width * (pixelFormat == JCS_RGB ? 3 : 1);

    cinfo.err = jpeg_std_error(&jerr);

    jpeg_create_compress(&cinfo);

    // Set destination
    jpeg_mem_dest(&cinfo, jpeg, &jpegSize);

    // Set options
    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = pixelFormat == JCS_RGB ? 3 : 1;
    cinfo.in_color_space = pixelFormat;

    if (!optimize) {
        // Not optimizing for space, so use a much faster compression
        // profile. This is about twice as fast and can be used when
        // testing visual quality *before* doing the final encoding.
        // Note: This *must* be set before calling `jpeg_set_defaults`
        // as it modifies how that call works.
        if (jpeg_c_int_param_supported(&cinfo, JINT_COMPRESS_PROFILE)) {
            jpeg_c_set_int_param(&cinfo, JINT_COMPRESS_PROFILE, JCP_FASTEST);
        }
    }

    jpeg_set_defaults(&cinfo);

    if (!optimize) {
        // Disable trellis quantization if we aren't optimizing. This saves
        // a little processing.
        if (jpeg_c_bool_param_supported(&cinfo, JBOOLEAN_TRELLIS_QUANT)) {
            jpeg_c_set_bool_param(&cinfo, JBOOLEAN_TRELLIS_QUANT, FALSE);
        }
        if (jpeg_c_bool_param_supported(&cinfo, JBOOLEAN_TRELLIS_QUANT_DC)) {
            jpeg_c_set_bool_param(&cinfo, JBOOLEAN_TRELLIS_QUANT_DC, FALSE);
        }
    }

    if (optimize && !progressive) {
        // Moz defaults, disable progressive
        cinfo.scan_info = NULL;
        cinfo.num_scans = 0;
        if (jpeg_c_bool_param_supported(&cinfo, JBOOLEAN_OPTIMIZE_SCANS)) {
            jpeg_c_set_bool_param(&cinfo, JBOOLEAN_OPTIMIZE_SCANS, FALSE);
        }
    }

    if (!optimize && progressive) {
        // No moz defaults, set scan progression
        jpeg_simple_progression(&cinfo);
    }

    if (subsample == SUBSAMPLE_444) {
        cinfo.comp_info[0].h_samp_factor = 1;
        cinfo.comp_info[0].v_samp_factor = 1;
        cinfo.comp_info[1].h_samp_factor = 1;
        cinfo.comp_info[1].v_samp_factor = 1;
        cinfo.comp_info[2].h_samp_factor = 1;
        cinfo.comp_info[2].v_samp_factor = 1;
    }

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

int checkPpmMagic(const unsigned char *buf, unsigned long size) {
    return (size >= 2 && buf[0] == 'P' && buf[1] == '6');
}

unsigned long decodePpm(unsigned char *buf, unsigned long bufSize, unsigned char **image, int *width, int *height) {
    unsigned long pos = 0, imageDataSize;
    int depth;

    if (!checkPpmMagic(buf, bufSize)) {
        fprintf(stderr, "Not a valid PPM format image!\n");
        return 0;
    }

    // Read to first newline
    while (buf[pos++] != '\n' && pos < bufSize);

    // Discard for any comment and empty lines
    while ((buf[pos] == '#' || buf[pos] == '\n') && pos < bufSize) {
        while (buf[pos] != '\n') {
            pos++;
        }
        pos++;
    }

    if (pos >= bufSize) {
        fprintf(stderr, "Not a valid PPM format image!\n");
        return 0;
    }

    // Read width/height
    sscanf((const char *) buf + pos, "%d %d", width, height);

    // Go to next line
    while (buf[pos++] != '\n' && pos < bufSize);

    if (pos >= bufSize) {
        fprintf(stderr, "Not a valid PPM format image!\n");
        return 0;
    }

    // Read bit depth
    sscanf((const char*) buf + pos, "%d", &depth);

    if (depth != 255) {
        fprintf(stderr, "Unsupported bit depth %d!\n", depth);
        return 0;
    }

    // Go to next line
    while (buf[pos++] != '\n' && pos < bufSize);

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

enum filetype detectFiletype(const char *filename) {
    unsigned char *buf = NULL;
    long bufSize = 0;
    bufSize = readFile(filename, (void **)&buf);
    enum filetype ret = detectFiletypeFromBuffer(buf, bufSize);
    free(buf);
    return ret;
}

enum filetype detectFiletypeFromBuffer(unsigned char *buf, long bufSize) {
    if (checkJpegMagic(buf, bufSize))
        return FILETYPE_JPEG;

    if (checkPpmMagic(buf, bufSize))
        return FILETYPE_PPM;

    return FILETYPE_UNKNOWN;
}

unsigned long decodeFile(const char *filename, unsigned char **image, enum filetype type, int *width, int *height, int pixelFormat) {
    unsigned char *buf = NULL;
    long bufSize = 0;
    bufSize = readFile(filename, (void **)&buf);
    unsigned long ret = decodeFileFromBuffer(buf, bufSize, image, type, width, height, pixelFormat);
    free(buf);
    return ret;
}

unsigned long decodeFileFromBuffer(unsigned char *buf, long bufSize, unsigned char **image, enum filetype type, int *width, int *height, int pixelFormat) {
    switch(type) {
        case FILETYPE_PPM:
            return decodePpm(buf, bufSize, image, width, height);
        case FILETYPE_JPEG:
            return decodeJpeg(buf, bufSize, image, width, height, pixelFormat);
        default:
            return 0;
    }
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
