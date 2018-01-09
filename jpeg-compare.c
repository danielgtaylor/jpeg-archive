/*
    Compare two JPEG images. Several methods are available. PSNR, SSIM,
    and MS_SSIM require the images to be exactly the same size, while
    FAST works on any sized image.

    FAST compares two images and returns the difference on a
    scale of 0 - 99, where 0 would mean the images are identical. The
    comparison should be immune to exposure, saturation, white balance,
    scaling, minor crops and similar modifications.

    If the difference is 10 or less than the images are very likely
    different versions of the same image (e.g. a thumbnail or black
    and white edit) or just slightly different images. It is possible
    to get false positives, in which case a slower PSNR or SSIM
    comparison will help.
*/

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "src/edit.h"
#include "src/hash.h"
#include "src/iqa/include/iqa.h"
#include "src/smallfry.h"
#include "src/util.h"

static const char *progname = "jpeg-compare";

// Comparison method
enum METHOD {
    UNKNOWN,
    FAST,
    PSNR,
    SSIM,
    MS_SSIM,
    SMALLFRY
};

int method = FAST;

// Hash size when method is FAST
int size = 16;

// Use PPM input?
enum filetype inputFiletype1 = FILETYPE_AUTO;
enum filetype inputFiletype2 = FILETYPE_AUTO;

static enum METHOD parseMethod(const char *s) {
    if (!strcmp("fast", s))
        return FAST;
    if (!strcmp("psnr", s))
        return PSNR;
    if (!strcmp("ssim", s))
        return SSIM;
    if (!strcmp("ms-ssim", s))
        return MS_SSIM;
    if (!strcmp("smallfry", s))
        return SMALLFRY;
    return UNKNOWN;
}

static enum filetype parseInputFiletype(const char *s) {
    if (!strcmp("auto", s))
        return FILETYPE_AUTO;
    if (!strcmp("jpeg", s))
        return FILETYPE_JPEG;
    if (!strcmp("ppm", s))
        return FILETYPE_PPM;
    return FILETYPE_UNKNOWN;
}

int compareFast(const char *filename1, const char *filename2) {
    unsigned char *hash1, *hash2;

    // Generate hashes
    if (jpegHash(filename1, &hash1, size)) {
        printf("Error hashing image 1!\n");
        return 1;
    }

    if (jpegHash(filename2, &hash2, size)) {
        printf("Error hashing image 2!\n");
        return 1;
    }

    // Compare and print out hamming distance
    printf("%u\n", hammingDist(hash1, hash2, size * size) * 100 / (size * size));

    // Cleanup
    free(hash1);
    free(hash2);

    return 0;
}

int compare(const char *filename1, const char *filename2) {
    unsigned char *image1, *image2, *image1Gray = NULL, *image2Gray = NULL;
    int width1, width2, height1, height2;
    int format, components;
    float diff;

    // Set requested pixel format
    switch (method) {
        case PSNR:
            format = JCS_RGB;
            components = 3;
            break;
        case SSIM: case MS_SSIM: default:
            format = JCS_GRAYSCALE;
            components = 1;
            break;
    }

    // Decode files
    if (!decodeFile(filename1, &image1, inputFiletype1, &width1, &height1, format)) {
        fprintf(stderr, "invalid input file: %s\n", filename1);
        return 1;
    }

    if (1 == components) {
        grayscale(image1, &image1Gray, width1, height1);
        free(image1);
        image1 = image1Gray;
    }

    if (!decodeFile(filename2, &image2, inputFiletype2, &width2, &height2, format)) {
        fprintf(stderr, "invalid input file: %s\n", filename2);
        return 1;
    }

    if (1 == components && FILETYPE_PPM == inputFiletype2) {
        grayscale(image2, &image2Gray, width2, height2);
        free(image2);
        image2 = image2Gray;
    }

    // Ensure width/height are equal
    if (width1 != width2 || height1 != height2) {
        printf("Images must be identical sizes for selected method!\n");
        return 1;
    }

    // Calculate and print comparison
    switch (method) {
        case PSNR:
            diff = iqa_psnr(image1, image2, width1, height1, width1 * components);
            printf("PSNR: %f\n", diff);
            break;
        case SMALLFRY:
            diff = smallfry_metric(image1, image2, width1, height1);
            printf("SMALLFRY: %f\n", diff);
            break;
        case MS_SSIM:
            diff = iqa_ms_ssim(image1, image2, width1, height1, width1 * components, 0);
            printf("MS-SSIM: %f\n", diff);
            break;
        case SSIM: default:
            diff = iqa_ssim(image1, image2, width1, height1, width1 * components, 0, 0);
            printf("SSIM: %f\n", diff);
            break;
    }

    // Cleanup
    free(image1);
    free(image2);

    return 0;
}

void version(void) {
    printf("%s\n", VERSION);
}

void usage(void) {
    printf("usage: %s [options] image1.jpg image2.jpg\n\n", progname);
    printf("options:\n\n");
    printf("  -V, --version                output program version\n");
    printf("  -h, --help                   output program help\n");
    printf("  -s, --size [arg]             set fast comparison image hash size\n");
    printf("  -m, --method [arg]           set comparison method to one of 'fast', 'psnr', 'ssim', or 'ms-ssim' [fast]\n");
    printf("  -r, --ppm                    parse first input as PPM instead of JPEG\n");
    printf("  -T, --input-filetype [arg]   set first input file type to one of 'auto', 'jpeg', 'ppm' [auto]\n");
    printf("  -U, --second-filetype [arg]  set second input file type to one of 'auto', 'jpeg', 'ppm' [auto]\n");
}

int main (int argc, char **argv) {
    const char *optstring = "VhS:m:rT:U:";
    static const struct option opts[] = {
        { "version", no_argument, 0, 'V' },
        { "help", no_argument, 0, 'h' },
        { "size", required_argument, 0, 'S' },
        { "method", required_argument, 0, 'm' },
        { "ppm", no_argument, 0, 'r' },
        { "input-filetype", required_argument, 0, 'T' },
        { "second-filetype", required_argument, 0, 'U' },
        { 0, 0, 0, 0 }
    };
    int opt, longind = 0;

    while ((opt = getopt_long(argc, argv, optstring, opts, &longind)) != -1) {
        switch (opt) {
        case 'V':
            version();
            return 0;
        case 'h':
            usage();
            return 0;
        case 's':
            size = atoi(optarg);
            break;
        case 'm':
            method = parseMethod(optarg);
            break;
        case 'r':
            inputFiletype1 = FILETYPE_PPM;
            break;
        case 'T':
            inputFiletype1 = parseInputFiletype(optarg);
            break;
        case 'U':
            inputFiletype2 = parseInputFiletype(optarg);
            break;
        };
    }

    if (argc - optind != 2) {
        usage();
        return 255;
    }

    /* Detect input file types. */
    if (inputFiletype1 == FILETYPE_AUTO)
        inputFiletype1 = detectFiletype(argv[optind]);
    if (inputFiletype2 == FILETYPE_AUTO)
        inputFiletype2 = detectFiletype(argv[optind + 1]);

    // Calculate and print output
    switch (method) {
        case FAST:
            if (inputFiletype1 != FILETYPE_JPEG || inputFiletype2 != FILETYPE_JPEG) {
                printf("fast comparison only works with JPEG files!\n");
                return 255;
            }
            return compareFast(argv[optind], argv[optind + 1]);
        case PSNR:
        case SSIM:
        case MS_SSIM:
        case SMALLFRY:
            return compare(argv[optind], argv[optind + 1]);
        default:
            printf("Unknown comparison method!\n");
            return 255;
    }
}
