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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "src/commander.h"
#include "src/edit.h"
#include "src/hash.h"
#include "src/iqa/include/iqa.h"
#include "src/util.h"

// Comparison method
enum METHOD {
    UNKNOWN,
    FAST,
    PSNR,
    SSIM,
    MS_SSIM
};

int method = FAST;

// Hash size when method is FAST
int size = 16;

// Use PPM input?
enum filetype inputFiletype1 = FILETYPE_AUTO;
enum filetype inputFiletype2 = FILETYPE_AUTO;

static void setSize(command_t *self) {
    size = atoi(self->arg);
}

static void setMethod(command_t *self) {
    if (!strcmp("fast", self->arg)) {
        method = FAST;
    } else if (!strcmp("psnr", self->arg)) {
        method = PSNR;
    } else if (!strcmp("ssim", self->arg)) {
        method = SSIM;
    } else if (!strcmp("ms-ssim", self->arg)) {
        method = MS_SSIM;
    } else {
        method = UNKNOWN;
    }
}

static void setInputFiletype1(command_t *self) {
    if (!strcmp("auto", self->arg))
        inputFiletype1 = FILETYPE_AUTO;
    else if (!strcmp("jpeg", self->arg))
        inputFiletype1 = FILETYPE_JPEG;
    else if (!strcmp("ppm", self->arg))
        inputFiletype1 = FILETYPE_PPM;
    else
        inputFiletype1 = FILETYPE_UNKNOWN;
}

static void setInputFiletype2(command_t *self) {
    if (!strcmp("auto", self->arg))
        inputFiletype2 = FILETYPE_AUTO;
    else if (!strcmp("jpeg", self->arg))
        inputFiletype2 = FILETYPE_JPEG;
    else if (!strcmp("ppm", self->arg))
        inputFiletype2 = FILETYPE_PPM;
    else
        inputFiletype2 = FILETYPE_UNKNOWN;
}

static void setPpm(command_t *self) {
    inputFiletype1 = FILETYPE_PPM;
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
    unsigned char *image1, *image2, *image1Gray = NULL;
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

    /* Detect input file types. */
    if (inputFiletype1 == FILETYPE_AUTO)
        inputFiletype1 = detectFiletype(filename1);
    if (inputFiletype2 == FILETYPE_AUTO)
        inputFiletype2 = detectFiletype(filename2);

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

int main (int argc, char **argv) {
    int error;

    // Parse commandline options
    command_t cmd;
    command_init(&cmd, argv[0], VERSION);
    cmd.usage = "[options] image1.jpg image2.jpg";
    command_option(&cmd, "-s", "--size [arg]", "Set fast comparison image hash size", setSize);
    command_option(&cmd, "-m", "--method [arg]", "Set comparison method to one of 'fast', 'psnr', 'ssim', or 'ms-ssim' [fast]", setMethod);
    command_option(&cmd, "-r", "--ppm", "Parse first input as PPM", setPpm);
    command_option(&cmd, "-T", "--input-filetype [arg]", "Set first input file type to one of 'auto', 'jpeg', 'ppm' [auto]", setInputFiletype1);
    command_option(&cmd, "-U", "--second-filetype [arg]", "Set second input file type to one of 'auto', 'jpeg', 'ppm' [auto]", setInputFiletype2);
    command_parse(&cmd, argc, argv);

    if (cmd.argc < 2) {
        command_help(&cmd);
        return 255;
    }

    // Calculate and print output
    switch (method) {
        case FAST:
            error = compareFast(cmd.argv[0], cmd.argv[1]);
            break;
        case PSNR: case SSIM: case MS_SSIM:
            error = compare(cmd.argv[0], cmd.argv[1]);
            break;
        default:
            printf("Unknown comparison method!\n");
            error = 255;
    }

    command_free(&cmd);

    return error;
}
