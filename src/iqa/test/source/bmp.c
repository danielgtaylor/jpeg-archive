/*
 * Copyright (c) 2011, Tom Distler (http://tdistler.com)
 * All rights reserved.
 *
 * The BSD License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, 
 *   this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * - Neither the name of the tdistler.com nor the names of its contributors may
 *   be used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmp.h"

int load_bmp(const char *fname, struct bmp *b)
{
    long len=0;
    unsigned char *hdr=0;
    FILE *file;
    int row;
    unsigned char *flip, *orig, *btm;

    file = fopen(fname, "rb");
    if (!file)
        return 1;

    fseek(file, 0, SEEK_END);
    len = ftell(file);
    fseek(file, 0, SEEK_SET);

    b->raw = (unsigned char*)malloc(len);
    if (!b->raw) {
        fclose(file);
        return 2;
    }

    if (len != fread(b->raw, 1, len, file)) {
        free(b->raw);
        fclose(file);
        return 3;
    }
    fclose(file);

    /* Parse the headers */
    hdr = b->raw + sizeof(struct bmp_magic);
    orig = b->raw + ((struct bmp_header*)hdr)->bmp_offset;
    hdr += sizeof(struct bmp_header);
    b->w = ((struct bmp_info_hdr*)hdr)->width;
    b->h = ((struct bmp_info_hdr*)hdr)->height;
    b->bit_depth = ((struct bmp_info_hdr*)hdr)->bitspp;
    b->stride = (b->w * (b->bit_depth/8) + 3) & ~3;

    /* Bitmaps are stored bottom-up... so flip. */
    flip = b->img = (unsigned char*)malloc(b->h * b->stride);
    if (!b->img) {
        free_bmp(b);
        return 4;
    }
    btm = orig + ((b->h - 1) * b->stride); /* Point to bottom row */
    for (row=0; row < b->h; ++row) {
        memcpy(flip, btm, b->stride);
        flip += b->stride;
        btm -= b->stride;
    }

    return 0;
}

void free_bmp(struct bmp *b)
{
    if (b) {
        if (b->raw)
            free(b->raw);
        if (b->img)
            free(b->img);
    }
}
