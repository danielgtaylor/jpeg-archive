#!/bin/bash

set -e

mkdir -p test-output

if [ ! -d test-files ]; then
    wget https://www.dropbox.com/s/hb3ah7p5hcjvhc1/jpeg-archive-test-files.zip
    unzip jpeg-archive-test-files.zip
fi

for file in test-files/*; do
    ./jpeg-recompress "$file" "test-output/`basename $file`"
done
