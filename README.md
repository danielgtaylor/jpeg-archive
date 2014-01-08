JPEG Archive
============
Utilities for archiving JPEGs for long term storage. The goals are:

 * Minimize storage space
 * Identify duplicates / similar photos

Contributions to this project are very welcome.

Utilities
---------
The following utilities are part of this project. All of them accept a `--help` parameter to see the available options.

### jpeg-recompress
Compress JPEGs by re-encoding to the smallest JPEG quality while keeping _perceived_ visual quality the same and by making sure huffman tables are optimized. This is a __lossy__ operation, but the images are visually identical and it usually saves 30-70% of the size for JPEGs coming from a digital camera, particularly DSLRs. By default all EXIF/IPTC/XMP and color profile metadata is copied over, but this can be disabled to save more space if desired.

The better the quality of the input image is, the better the output will be.

Some basic photo-related editing options are available, such as removing fisheye lens distortion.

```bash
# Default settings
jpeg-recompress image.jpg compressed.jpg

# High quality example settings
jpeg-recompress --quality high --min 60 image.jpg compressed.jpg

# Remove fisheye distortion (Tokina 10-17mm on APS-C @ 10mm)
jpeg-recompress --defish 2.6 --zoom 1.2 image.jpg defished.jpg
```

### jpeg-compare
Compare two JPEG photos to judge how similar they are. The `fast` comparison method returns an integer from 0 to 99, where 0 is identical. PSNR, SSIM, and MS-SSIM return floats but require images to be the same dimensions.

```bash
# Do a fast compare of two images
jpeg-compare image1.jpg image2.jpg

# Calculate PSNR
jpeg-compare --method psnr image1.jpg image2.jpg

# Calculate SSIM
jpeg-compare --method ssim image1.jpg image2.jpg
```

### jpeg-hash
Create a hash of an image that can be used to compare it to other images quickly.

```bash
jpeg-hash image.jpg
```

Building
--------
### Dependencies
 * libjpeg (or libjpeg-turbo is recommended)

#### Ubuntu
Ubuntu users can install via `apt-get`:

```bash
sudo apt-get install build-essential libjpeg-turbo8 libjpeg-turbo8-dev
```

#### Mac OS X
Mac users can install it via [Homebrew](http://brew.sh/):

```bash
brew install libjpeg-turbo
```

### Compiling
The `Makefile` should work as-is on Ubuntu and Mac OS X. Other platforms may need to set the location of `libjpeg.a`.

```bash
make
```

### Installation
Install the binaries into `/usr/local/bin`:

```bash
sudo make install
```

Links / Alternatives
--------------------
* https://github.com/rflynn/imgmin
* https://news.ycombinator.com/item?id=803839

License
-------
Copyright &copy; 2014 Daniel G. Taylor

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Image Quality Assessment (IQA) is copyright 2011, Tom Distler (http://tdistler.com)
