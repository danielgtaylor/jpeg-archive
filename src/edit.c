#include <math.h>
#include <stdlib.h>

float clamp(float low, float value, float high) {
    return (value < low) ? low : ((value > high) ? high : value);
}

int interpolate(const unsigned char *image, int width, int components, float x, float y, int offset) {
    int stride = width * components;
    float px = x - floor(x);
    float py = y - floor(y);
    int x1 = floor(x);
    int x2 = ceil(x);
    int y1 = floor(y);
    int y2 = ceil(y);

    float top = (float) image[y1 * stride + x1 * components + offset] * (1.0 - px) +
                (float) image[y1 * stride + x2 * components + offset] * px;
    float bot = (float) image[y2 * stride + x1 * components + offset] * (1.0 - px) +
                (float) image[y2 * stride + x2 * components + offset] * px;
       
    return (top * (1.0 - py)) + (bot * py);
}

void defish(const unsigned char *input, unsigned char *output, int width, int height, int components, float strength, float zoom) {
    const int cx = width / 2;
    const int cy = height / 2;
    const float len = sqrt(width * width + height * height);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float dx = (cx - x) * zoom;
            float dy = (cy - y) * zoom;
            float r = sqrt(dx * dx + dy * dy) / len * strength;
            float theta = 1.0;

            if (r != 0.0) {
                theta = atan(r) / r;
            }

            dx = clamp(0.0, (float) width / 2.0 - theta * dx, width);
            dy = clamp(0.0, (float) height / 2.0 - theta * dy, height);

            for (int z = 0; z < components; z++) {
                output[y * width * 3 + x * 3 + z] = interpolate(input, width, components, dx, dy, z);
            }
        }
    }
}

long grayscale(const unsigned char *input, unsigned char **output, int width, int height) {
    int stride = width * 3;

    *output = malloc(width * height);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Y = 0.299R + 0.587G + 0.114B
            (*output)[y * width + x] = input[y * stride + x * 3] * 0.299 +
                                       input[y * stride + x * 3 + 1] * 0.587 +
                                       input[y * stride + x * 3 + 2] * 0.114 + 0.5;
        }
    }

    return width * height;
}
