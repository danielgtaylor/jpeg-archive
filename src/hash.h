/*
    Generate an image hash given a filename. This is a convenience
    function which reads the file, decodes it to grayscale,
    scales the image, and generates the hash.
*/
int jpegHash(const char *filename, unsigned char **hash, int size);

/*
    Downscale an image with nearest-neighbor interpolation.
    http://jsperf.com/pixel-interpolation/2
*/
void scale(unsigned char *image, int width, int height, unsigned char **newImage, int newWidth, int newHeight);

/*
    Generate an image hash based on gradients.
    http://www.hackerfactor.com/blog/index.php?/archives/529-Kind-of-Like-That.html
*/
void genHash(unsigned char *image, int width, int height, unsigned char **hash);

/*
    Calculate the hamming distance between two hashes.
    http://en.wikipedia.org/wiki/Hamming_distance
*/
unsigned int hammingDist(const unsigned char *hash1, const unsigned char *hash2, int hashLength);
