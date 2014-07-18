/*
    Image editing functions
*/
#ifndef EDIT_H
#define EDIT_H

/*
    Clamp a value between low and high.
*/
float clamp(float low, float value, float high);

/*
    Bilinear interpolation
*/
int interpolate(const unsigned char *image, int width, int components, float x, float y, int offset);

/*
    Get mean error per pixel rate.
*/
float meanPixelError(const unsigned char *original, const unsigned char *compressed, int width, int height, int components);

/*
    Remove fisheye distortion from an image. The amount of distortion is
    controlled by strength, while zoom controls where the image gets
    cropped. For example, the Tokina 10-17mm ATX fisheye on a Canon APS-C
    body set to 10mm looks good with strength=2.6 and zoom=1.2.
*/
void defish(const unsigned char *input, unsigned char *output, int width, int height, int components, float strength, float zoom);

/*
    Convert an RGB image to grayscale. Assumes 8-bit color components, 
    3 color components and a row stride of width * 3.
*/
long grayscale(const unsigned char *input, unsigned char **output, int width, int height);

#endif
