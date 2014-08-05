/*
    Hash a single JPEG file. The hash is based on tracking gradients
    between pixels in the image. The larger the hash size, the less
    likely you are to get collisions, but the more time it takes to
    calculate.
*/
#include <stdio.h>
#include <stdlib.h>

#include "src/commander.h"
#include "src/hash.h"
#include "src/util.h"

int size = 16;

static void setSize(command_t *self) {
    size = atoi(self->arg);
}

int main (int argc, char **argv) {
    unsigned char *hash;

    // Parse commandline options
    command_t cmd;
    command_init(&cmd, argv[0], VERSION);
    cmd.usage = "[options] image.jpg";
    command_option(&cmd, "-s", "--size [arg]", "Set image hash size", setSize);
    command_parse(&cmd, argc, argv);

    if (cmd.argc < 1) {
        command_help(&cmd);
        return 255;
    }

    // Generate the image hash
    if (jpegHash(cmd.argv[0], &hash, size)) {
        printf("Error hashing image!\n");
        return 1;
    }

    // Print out the hash a string of 1s and 0s
    for (int x = 0; x < size * size; x++) {
        printf("%c", hash[x] ? '1' : '0');
    }
    printf("\n");

    // Cleanup
    command_free(&cmd);
    free(hash);

    return 0;
}
