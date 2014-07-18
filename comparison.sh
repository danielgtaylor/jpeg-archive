#!/bin/bash

# This script requires ImageMagick and Ghostscript

mkdir -p comparison
cd comparison

if [ ! -f nikon_d3x.jpg ]; then
    wget http://static.nikonusa.com/D3X_gallery/images/pic_004b.jpg -O nikon_d3x.jpg
fi

# Create encodings
../jpeg-recompress -m ssim nikon_d3x.jpg test-ssim.jpg
../jpeg-recompress -m ms-ssim nikon_d3x.jpg test-ms-ssim.jpg
../jpeg-recompress -m smallfry nikon_d3x.jpg test-smallfry.jpg

# Crop images
convert nikon_d3x.jpg -crop 360x400+1604+1934! -gravity northwest -fill white -pointsize 16 -annotate +10+10 "Original from camera \(`du -k nikon_d3x.jpg | python2 -c 'kb = raw_input().split()[0]; print("%.2f MiB" % (float(kb) / 1024))'`\)" crop-orig.png
convert test-ssim.jpg -crop 360x400+1604+1934! -gravity northeast -fill white -pointsize 16 -annotate +10+10 "Compressed SSIM \(`du -k test-ssim.jpg | python2 -c 'kb = raw_input().split()[0]; print("%.2f MiB" % (float(kb) / 1024))'`\)" crop-ssim.png
convert test-ms-ssim.jpg -crop 360x400+1604+1934! -gravity southwest -fill white -pointsize 16 -annotate +10+10 "Compressed MS-SSIM \(`du -k test-ms-ssim.jpg | python2 -c 'kb = raw_input().split()[0]; print("%.2f MiB" % (float(kb) / 1024))'`\)" crop-ms-ssim.png
convert test-smallfry.jpg -crop 360x400+1604+1934! -gravity southeast -fill white -pointsize 16 -annotate +10+10 "Compressed SmallFry \(`du -k test-smallfry.jpg | python2 -c 'kb = raw_input().split()[0]; print("%.2f MiB" % (float(kb) / 1024))'`\)" crop-smallfry.png

# Create montage
montage -geometry +0+0 crop-orig.png crop-ssim.png crop-ms-ssim.png crop-smallfry.png ../comparison.png

cd ..
