#!/bin/bash

mkdir -p test-output

for file in test-files/*; do
    ./jpeg-recompress "$file" "test-output/`basename $file`"
done
