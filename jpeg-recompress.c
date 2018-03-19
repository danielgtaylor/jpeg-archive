/*
    Recompress a JPEG file while attempting to keep visual quality the same
    by using structural similarity (SSIM) as a metric. Does a binary search
    between JPEG quality 40 and 95 to find the best match. Also makes sure
    that huffman tables are optimized if they weren't already.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "src/commander.h"
#include "src/recompress.h"

recompress_options_t options = {
    .method = METHOD_SSIM,
    .attempts = 6,
    .target = 0,
    .preset = QUALITY_MEDIUM,
    .jpegMin = 40,
    .jpegMax = 95,
    .strip = false,
    .noProgressive = false,
    .defishStrength = 0.0,
    .defishZoom = 1.0,
    .inputFiletype = FILETYPE_AUTO,
    .copyFiles = true,
    .accurate = false,
    .subsample = SUBSAMPLE_DEFAULT,
    .quiet = false
};

static void setAttempts(command_t *self) {
    options.attempts = atoi(self->arg);
}

static void setTarget(command_t *self) {
    options.target = atof(self->arg);
}

static void setQuality(command_t *self) {
    if (!strcmp("low", self->arg)) {
        options.preset = QUALITY_LOW;
    } else if (!strcmp("medium", self->arg)) {
        options.preset = QUALITY_MEDIUM;
    } else if (!strcmp("high", self->arg)) {
        options.preset = QUALITY_HIGH;
    } else if (!strcmp("veryhigh", self->arg)) {
        options.preset = QUALITY_VERYHIGH;
    } else {
        fprintf(stderr, "Unknown quality preset '%s'!\n", self->arg);
    }
}

static void setMethod(command_t *self) {
    if (!strcmp("ssim", self->arg)) {
        options.method = METHOD_SSIM;
    } else if (!strcmp("ms-ssim", self->arg)) {
        options.method = METHOD_MS_SSIM;
    } else if (!strcmp("smallfry", self->arg)) {
        options.method = METHOD_SMALLFRY;
    } else if (!strcmp("mpe", self->arg)) {
        options.method = METHOD_MPE;
    } else {
        options.method = METHOD_UNKNOWN;
    }
}

static void setNoProgressive(command_t *self) {
    options.noProgressive = true;
}

static void setMinimum(command_t *self) {
    options.jpegMin = atoi(self->arg);
}

static void setMaximum(command_t *self) {
    options.jpegMax = atoi(self->arg);
}

static void setStrip(command_t *self) {
    options.strip = true;
}

static void setDefish(command_t *self) {
    options.defishStrength = atof(self->arg);
}

static void setZoom(command_t *self) {
    options.defishZoom = atof(self->arg);
}

static void setInputFiletype(command_t *self) {
    if (!strcmp("auto", self->arg))
        options.inputFiletype = FILETYPE_AUTO;
    else if (!strcmp("jpeg", self->arg))
        options.inputFiletype = FILETYPE_JPEG;
    else if (!strcmp("ppm", self->arg))
        options.inputFiletype = FILETYPE_PPM;
    else
        options.inputFiletype = FILETYPE_UNKNOWN;
}

static void setPpm(command_t *self) {
    options.inputFiletype = FILETYPE_PPM;
}

static void setCopyFiles(command_t *self) {
    options.copyFiles = false;
}

static void setAccurate(command_t *self) {
    options.accurate = true;
}

static void setSubsampling(command_t *self) {
    if (!strcmp("default", self->arg)) {
        options.subsample = SUBSAMPLE_DEFAULT;
    } else if (!strcmp("disable", self->arg)) {
        options.subsample = SUBSAMPLE_444;
    } else {
        fprintf(stderr, "Unknown sampling method '%s', using default!\n", self->arg);
    }
}

static void setQuiet(command_t *self) {
    options.quiet = true;
}

int main (int argc, char **argv) {
    // Parse commandline options
    command_t cmd;
    command_init(&cmd, argv[0], VERSION);
    cmd.usage = "[options] input.jpg compressed-output.jpg";
    command_option(&cmd, "-t", "--target [arg]", "Set target quality [0.9999]", setTarget);
    command_option(&cmd, "-q", "--quality [arg]", "Set a quality preset: low, medium, high, veryhigh [medium]", setQuality);
    command_option(&cmd, "-n", "--min [arg]", "Minimum JPEG quality [40]", setMinimum);
    command_option(&cmd, "-x", "--max [arg]", "Maximum JPEG quality [95]", setMaximum);
    command_option(&cmd, "-l", "--loops [arg]", "Set the number of runs to attempt [6]", setAttempts);
    command_option(&cmd, "-a", "--accurate", "Favor accuracy over speed", setAccurate);
    command_option(&cmd, "-m", "--method [arg]", "Set comparison method to one of 'mpe', 'ssim', 'ms-ssim', 'smallfry' [ssim]", setMethod);
    command_option(&cmd, "-s", "--strip", "Strip metadata", setStrip);
    command_option(&cmd, "-d", "--defish [arg]", "Set defish strength [0.0]", setDefish);
    command_option(&cmd, "-z", "--zoom [arg]", "Set defish zoom [1.0]", setZoom);
    command_option(&cmd, "-r", "--ppm", "Parse input as PPM", setPpm);
    command_option(&cmd, "-c", "--no-copy", "Disable copying files that will not be compressed", setCopyFiles);
    command_option(&cmd, "-p", "--no-progressive", "Disable progressive encoding", setNoProgressive);
    command_option(&cmd, "-S", "--subsample [arg]", "Set subsampling method. Valid values: 'default', 'disable'. [default]", setSubsampling);
    command_option(&cmd, "-T", "--input-filetype [arg]", "Set input file type to one of 'auto', 'jpeg', 'ppm' [auto]", setInputFiletype);
    command_option(&cmd, "-Q", "--quiet", "Only print out errors.", setQuiet);
    command_parse(&cmd, argc, argv);

    if (cmd.argc < 2) {
        command_help(&cmd);
        return 255;
    }

    if (options.method == METHOD_UNKNOWN) {
        fprintf(stderr, "Invalid method!");
        command_help(&cmd);
        return 255;
    }

    recompress_error_t **error = malloc(sizeof(recompress_error_t *));
    bool res = recompress(cmd.argv[0], cmd.argv[1], &options, error);

    /* Cleanup. */
    command_free(&cmd);

    if (!res) {
        int code = (*error)->statusCode;
        fprintf(stderr, "%s\n", (*error)->message);
        free(*error);
        free(error);
        return code;
    }

    free(error);
    return 0;
}
