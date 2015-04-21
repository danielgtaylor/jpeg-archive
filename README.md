JPEG Archive [![Build Status](http://img.shields.io/travis/danielgtaylor/jpeg-archive.svg?style=flat)](https://travis-ci.org/danielgtaylor/jpeg-archive) [![Build status](https://ci.appveyor.com/api/projects/status/1p7hrrq380xuqlyh?svg=true)](https://ci.appveyor.com/project/danielgtaylor/jpeg-archive) [![Version](http://img.shields.io/badge/version-2.1.1-blue.svg?style=flat)](https://github.com/danielgtaylor/jpeg-archive/releases) [![License](http://img.shields.io/badge/license-MIT-red.svg?style=flat)](http://dgt.mit-license.org/)
============
Utilities for archiving photos for saving to long term storage or serving over the web. The goals are:

 * Use a common, well supported format (JPEG)
 * Minimize storage space & cost
 * Identify duplicates / similar photos

Approach:

 * Commandline utilities & scripts
 * Simple options & useful help
 * Good quality output via sane defaults

Contributions to this project are very welcome.

Download
--------
You can download the latest source and binary releases from the [JPEG Archive releases page](https://github.com/danielgtaylor/jpeg-archive/releases). Windows binaries for the latest commit are available from the [Windows CI build server](https://ci.appveyor.com/project/danielgtaylor/jpeg-archive/build/artifacts).

If you are looking for an easy way to run these utilities in parallel over many files to utilize all CPU cores, please also download [Ladon](https://github.com/danielgtaylor/ladon). You can then use the `jpeg-archive` command below or use `ladon` directly. Example:

```bash
# Re-compress JPEGs and replace the originals
ladon "Photos/**/*.jpg" -- jpeg-recompress FULLPATH FULLPATH

# Re-compress JPEGs into the new directory 'Comp'
ladon -m Comp/RELDIR "Photos/**/*.jpg" -- jpeg-recompress FULLPATH Comp/RELPATH
```

Utilities
---------
The following utilities are part of this project. All of them accept a `--help` parameter to see the available options.

### jpeg-archive
Compress RAW and JPEG files in a folder utilizing all CPU cores. This is a simple bash script that uses the utilities below. It requires:

* Bash
* [Ladon](https://github.com/danielgtaylor/ladon)
* [dcraw](http://www.cybercom.net/~dcoffin/dcraw/)
* [exiftool](http://www.sno.phy.queensu.ca/~phil/exiftool/)
* jpeg-recompress (part of this project)

```bash
# Compress a folder of images
cd path/to/photos
jpeg-archive

# Custom quality and metric
jpeg-archive --quality medium --method smallfry
```

### jpeg-recompress
Compress JPEGs by re-encoding to the smallest JPEG quality while keeping _perceived_ visual quality the same and by making sure huffman tables are optimized. This is a __lossy__ operation, but the images are visually identical and it usually saves 30-70% of the size for JPEGs coming from a digital camera, particularly DSLRs. By default all EXIF/IPTC/XMP and color profile metadata is copied over, but this can be disabled to save more space if desired.

There is no need for the input file to be a JPEG. In fact, you can use `jpeg-recompress` as a replacement for `cjpeg` by using PPM input and the `--ppm` option.

The better the quality of the input image is, the better the output will be.

Some basic photo-related editing options are available, such as removing fisheye lens distortion.

#### Demo
Below are two 100% crops of [Nikon's D3x Sample Image 2](http://static.nikonusa.com/D3X_gallery/index.html). The left shows the original image from the camera, while the others show the output of `jpeg-recompress` with the `medium` quality setting and various comparison methods. By default SSIM is used, which lowers the file size by **88%**. The recompression algorithm chooses a JPEG quality of 80. By comparison the `veryhigh` quality setting chooses a JPEG quality of 93 and saves 70% of the file size.

![JPEG recompression comparison](https://cloud.githubusercontent.com/assets/106826/3633843/5fde26b6-0eff-11e4-8c98-f18dbbf7b510.png)

Why are they different sizes? The default quality settings are set to average out to similar visual quality over large data sets. They may differ on individual photos (like above) because each metric considers different parts of the image to be more or less important for compression.

#### Image Comparison Metrics
The following metrics are available when using `jpeg-recompress`. SSIM is the default.

Name     | Option        | Description
-------- | ------------- | -----------
MPE      | `-m mpe`      | Mean pixel error (as used by [imgmin](https://github.com/rflynn/imgmin))
SSIM     | `-m ssim`     | [Structural similarity](http://en.wikipedia.org/wiki/Structural_similarity) **DEFAULT**
MS-SSIM* | `-m ms-ssim`  | Multi-scale structural similarity (slow!) ([2008 paper](http://foulard.ece.cornell.edu/publications/dmr_hvei2008_paper.pdf))
SmallFry | `-m smallfry` | Linear-weighted BBCQ-like ([original project](https://github.com/dwbuiten/smallfry), [2011 BBCQ paper](http://spie.org/Publications/Proceedings/Paper/10.1117/12.872231))

**Note**: The SmallFry algorithm may be [patented](http://www.jpegmini.com/main/technology) so use with caution.

#### Subsampling
The JPEG format allows for subsampling of the color channels to save space. For each 2x2 block of pixels per color channel (four pixels total) it can store four pixels (all of them), two pixels or a single pixel. By default, the JPEG encoder subsamples the non-luma channels to two pixels (often referred to as 4:2:0 subsampling). Most digital cameras do the same because of limitations in the human eye. This may lead to unintended behavior for specific use cases (see #12 for an example), so you can use `--subsample disable` to disable this subsampling.

#### Example Commands

```bash
# Default settings
jpeg-recompress image.jpg compressed.jpg

# High quality example settings
jpeg-recompress --quality high --min 60 image.jpg compressed.jpg

# Slow high quality settings (3-4x slower than above, slightly more accurate)
jpeg-recompress --accurate --quality high --min 60 image.jpg compressed.jpg

# Use SmallFry instead of SSIM
jpeg-recompress --method smallfry image.jpg compressed.jpg

# Use 4:4:4 sampling (disables subsampling).
jpeg-recmopress --subsample disable image.jpg compressed.jpg

# Remove fisheye distortion (Tokina 10-17mm on APS-C @ 10mm)
jpeg-recompress --defish 2.6 --zoom 1.2 image.jpg defished.jpg

# Read from stdin and write to stdout with '-' as the filename
jpeg-recompress - - <image.jpg >compressed.jpg

# Convert RAW to JPEG via PPM from stdin
dcraw -w -q 3 -c IMG_1234.CR2 | jpeg-recompress --ppm - compressed.jpg

# Disable progressive mode (not recommended)
jpeg-recompress --no-progressive image.jpg compressed.jpg

# Disable all output except for errors
jpeg-recompress --quiet image.jpg compressed.jpg
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
$ sudo apt-get install build-essential autoconf nasm libtool
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

* [CMake](http://www.cmake.org/files/v3.0/cmake-3.0.0-win32-x86.exe)
* [NASM](http://www.nasm.us/pub/nasm/releasebuilds/2.11.05/win32/)
* [MinGW](http://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win32/Personal%20Builds/mingw-builds/installer/mingw-w64-install.exe/download) (installed to e.g. `C:\mingw`)
* [Github for Windows](https://windows.github.com/)

Run Github for windows. In the settings, set **Git Bash** as the shell. Open Git Shell from the start menu.

```bash
# Update PATH to include MinGW/NASM bin folder, location on your system may vary
$ export PATH=$PATH:/c/mingw/mingw32/bin:/c/Program\ Files \(x68\)/nasm

# Build mozjpeg or download https://www.dropbox.com/s/98jppfgds2xjblu/libjpeg.a
$ git clone https://github.com/mozilla/mozjpeg.git
$ cd mozjpeg
$ cmake -G "MSYS Makefiles" -D CMAKE_C_COMPILER=/c/mingw/bin/gcc.exe -D
CMAKE_MAKE_PROGRAM=/c/mingw/bin/mingw32-make.exe  -DWITH_JPEG8=1 .
$ mingw32-make
$ cd ..

# Build jpeg-archive
$ git clone https://github.com/danielgtaylor/jpeg-archive
$ cd jpeg-archive
$ CC=gcc mingw32-make
```

JPEG-Archive should now be built.

### Compiling (Linux and Mac OS X)
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
JPEG-Archive is copyright &copy; 2015 Daniel G. Taylor
Image Quality Assessment (IQA) is copyright 2011, Tom Distler (http://tdistler.com)
SmallFry is copyright 2014, Derek Buitenhuis (https://github.com/dwbuiten)

All are released under an MIT license.

http://dgt.mit-license.org/
