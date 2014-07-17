JPEG Archive [![Build Status](https://travis-ci.org/danielgtaylor/jpeg-archive.png?branch=master)](https://travis-ci.org/danielgtaylor/jpeg-archive)
============
Utilities for archiving JPEGs for long term storage. The goals are:

 * Minimize storage space
 * Identify duplicates / similar photos

Contributions to this project are very welcome.

Download
--------
You can download the latest source and binary releases from the [JPEG Archive releases page](https://github.com/danielgtaylor/jpeg-archive/releases).

If you are looking for an easy way to run these utilities in parallel over many files to utilize all CPU cores, please also download [Ladon](https://github.com/danielgtaylor/ladon). Example:

```bash
# Re-compress JPEGs and replace the originals
ladon "Photos/**/*.jpg" -- jpeg-recompress FULLPATH FULLPATH

# Re-compress JPEGs into the new directory 'Comp'
ladon -m Comp/RELDIR "Photos/**/*.jpg" -- jpeg-recompress FULLPATH Comp/RELPATH
```

Utilities
---------
The following utilities are part of this project. All of them accept a `--help` parameter to see the available options.

### jpeg-recompress
Compress JPEGs by re-encoding to the smallest JPEG quality while keeping _perceived_ visual quality the same and by making sure huffman tables are optimized. This is a __lossy__ operation, but the images are visually identical and it usually saves 30-70% of the size for JPEGs coming from a digital camera, particularly DSLRs. By default all EXIF/IPTC/XMP and color profile metadata is copied over, but this can be disabled to save more space if desired.

There is no need for the input file to be a JPEG. In fact, you can use `jpeg-recompress` as a replacement for `cjpeg` by using PPM input and the `--ppm` option.

The better the quality of the input image is, the better the output will be.

Some basic photo-related editing options are available, such as removing fisheye lens distortion.

#### Demo
Below are two 100% crops of [Nikon's D3x Sample Image 2](http://static.nikonusa.com/D3X_gallery/index.html). The left shows the original image from the camera, while the right shows the output of `jpeg-recompress` with the `medium` quality setting, which lowers the file size by **85%**. The recompression algorithm chooses a JPEG quality of 80 for a structural similarity (SSIM) of 0.999915. By comparison the `veryhigh` quality setting chooses a JPEG quality of 93 and saves 63% of the file size.

![JPEG recompression comparison](https://raw.github.com/danielgtaylor/jpeg-archive/master/comparison.png)

#### Image Comparison Metrics
The following metrics are available when using `jpeg-recompress`. SSIM is the default.

Name     | Option        | Description
-------- | ------------- | -----------
SSIM     | `-m ssim`     | [Structural similarity](http://en.wikipedia.org/wiki/Structural_similarity)
MS-SSIM* | `-m ms-ssim`  | Multi-scale structural similarity ([2008 paper](http://foulard.ece.cornell.edu/publications/dmr_hvei2008_paper.pdf))
SmallFry | `-m smallfry` | Linear-weighted BBCQ-like ([original project](https://github.com/dwbuiten/smallfry), [2011 BBCQ paper](http://spie.org/Publications/Proceedings/Paper/10.1117/12.872231))

**Note**: The SmallFry algorithm may be [patented](http://www.jpegmini.com/main/technology) so use with caution.

```bash
# Default settings
jpeg-recompress image.jpg compressed.jpg

# High quality example settings
jpeg-recompress --quality high --min 60 image.jpg compressed.jpg

# Use SmallFry instead of SSIM
jpeg-recompress --method smallfry image.jpg compressed.jpg

# Remove fisheye distortion (Tokina 10-17mm on APS-C @ 10mm)
jpeg-recompress --defish 2.6 --zoom 1.2 image.jpg defished.jpg

# Read from stdin and write to stdout with '-' as the filename
jpeg-recompress - - <image.jpg >compressed.jpg

# Convert RAW to JPEG via PPM from stdin
dcraw -w -q 3 -c IMG_1234.CR2 | jpeg-recompress --ppm - compressed.jpg
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
 * [mozjpeg](https://github.com/mozilla/mozjpeg)

#### Ubuntu
Ubuntu users can install via `apt-get`:

```bash
$ sudo apt-get install build-essential autoconf nasm
$ git clone https://github.com/mozilla/mozjpeg.git
$ cd mozjpeg
$ autoreconf -fiv
$ ./configure --with-jpeg8
$ make
$ sudo make install 
```

#### Mac OS X
Mac users can install it via [Homebrew](http://brew.sh/):

```bash
$ brew install mozjpeg
```

#### Windows
The `Makefile` should work with MinGW/Cygwin/etc and standard GCC. Patches welcome.

To get everything you need to build, install these:

* [NASM](http://www.nasm.us/pub/nasm/releasebuilds/2.11.05/win32/)
* [MinGW](http://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win32/Personal%20Builds/mingw-builds/installer/mingw-w64-install.exe/download)
* [Github for Windows](https://windows.github.com/)

Run Github for windows. In the settings, set **Git Bash** as the shell. Open Git Shell from the start menu.

```bash
# Update PATH to include MinGW bin folder, location on your system may vary
$ export PATH=$PATH:/c/Program\ Files\ (x86)\MinGW......../bin

$ git clone https://github.com/mozilla/mozjpeg.git
$ cd mozjpeg
$ autoreconf -fiv
$ ./configure --with-jpeg8
$ make
$ cd ..

$ git clone https://github.com/danielgtaylor/jpeg-archive
$ cd jpeg-archive

# Build the executables
$ mingw32-make

# If the `uname` utility is missing, create a blank file named `uname.bat`
# and then try again and things should work.
```

### Compiling
The `Makefile` should work as-is on Ubuntu and Mac OS X. Other platforms may need to set the location of `libjpeg.a` or make other tweaks.

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
SmallFry is copyright 2014, Derek Buitenhuis (https://github.com/dwbuiten)
