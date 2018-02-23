/*
    Hash a single JPEG file. The hash is based on tracking gradients
    between pixels in the image. The larger the hash size, the less
    likely you are to get collisions, but the more time it takes to
    calculate.
*/
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "src/hash.h"
#include "src/util.h"

static const char *progname = "jpeg-hash";

int size = 16;

void version(void) {
    printf("%s\n", VERSION);
}

void usage(void) {
    printf("usage: %s [options] image.jpg\n\n", progname);
    printf("options:\n\n");
    printf("  -V, --version                output program version\n");
    printf("  -h, --help                   output program help\n");
    printf("  -s, --size [arg]             set fast comparison image hash size\n");
}

int main (int argc, char **argv) {
    unsigned char *hash;

    const char *optstring = "Vhs:";
    static const struct option opts[] = {
        { "version", no_argument, 0, 'V' },
        { "help", no_argument, 0, 'h' },
        { "size", required_argument, 0, 's' },
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
        };
    }

    if (argc - optind != 1) {
        usage();
        return 255;
    }

    // Generate the image hash
    if (jpegHash(argv[optind], &hash, size)) {
        printf("Error hashing image!\n");
        return 1;
    }

    // Print out the hash a string of 1s and 0s
    for (int x = 0; x < size * size; x++) {
        printf("%c", hash[x] ? '1' : '0');
    }
    printf("\n");

    // Cleanup
    free(hash);

    return 0;
}
