#!/bin/bash

mkdir -p test-output

for file in test/*; do
    ./jpeg-recompress "$file" "test-output/`basename $file`"
done
