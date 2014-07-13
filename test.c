#include "src/edit.h"
#include "src/hash.h"
#include "src/util.h"

#include "src/test/describe.h"

describe ("Unit Tests", {
    it ("Should clamp values", {
        assert_equal_float(0.0, clamp(0.0, -10.0, 100.0));
        assert_equal_float(32.5, clamp(0.0, 32.5, 100.0));
        assert_equal_float(100.0, clamp(0.0, 150.3, 100.0));
    });

    it ("Should interpolate pixels", {
        unsigned char *image;
        int value;

        image = malloc(4);

        image[0] = 0;
        image[1] = 255;
        image[2] = 127;
        image[3] = 66;

        value = interpolate(image, 2, 1, 0.3, 0.6, 0);

        assert_equal(95, value);

        free(image);
    });

    it ("Should scale an image", {
        unsigned char *image;
        unsigned char *scaled;

        /*
        [  0  1  2  3
           4  5  6  7
           8  9 10 11
          12 13 14 15 ]
        */

        image = malloc(16);

        for (int x = 0; x < 16; x++) {
            image[x] = (unsigned char) x;
        }

        /*
        [  0  2
           8 10 ]
        */
        scale(image, 4, 4, &scaled, 2, 2);

        assert_equal(image[2], scaled[1]);
        assert_equal(image[8], scaled[2]);

        free(scaled);
        free(image);
    });

    it ("Should generate an image hash", {
        unsigned char *image;
        unsigned char *hash;

        image = malloc(16);

        /*
        [  0 15  2 13
           4 11  6  9
           8  7 10  5
          12  3 14  1 ]
        */

        for (int x = 0; x < 16; x++) {
            image[x] = (unsigned char) ((x % 2) ? 16 - x : x);
        }

        // Hash should be 101010100101010
        genHash(image, 4, 4, &hash);

        assert_equal(1, hash[0]);
        assert_equal(0, hash[1]);
        assert_equal(0, hash[5]);
        assert_equal(1, hash[9]);

        free(hash);
        free(image);
    });

    it ("Should calculate hamming distance", {
        int dist = hammingDist((unsigned char *) "101010", (unsigned char *) "111011", 6);
        assert_equal(2, dist);
    });

    it ("Should decode a PPM", {
        char *image = "P6\n2 2\n255\n\x1\x2\x3\x4\x5\x6\x7\x8\x9\xa\xb\xc";
        unsigned char *imageData;
        int width;
        int height;

        decodePpm((unsigned char *) image, 23, (unsigned char **) &imageData, &width, &height);

        assert_equal(2, width);
        assert_equal(2, height);
        assert_equal('\x1', imageData[0]);
        assert_equal('\xc', imageData[11]);

        free(imageData);
    })
});
