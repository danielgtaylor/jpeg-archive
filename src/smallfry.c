/*
 * Copyright (c) 2014, Derek Buitenhuis
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#include "smallfry.h"

#define MAX(a, b) (a > b ? a : b)
#define MIN(a, b) (a < b ? a : b)

static double psnr_factor(uint8_t *orig, uint8_t *cmp, int orig_stride,
                          int cmp_stride, int width, int height, uint8_t max)
{
    uint8_t *old, *new;
    double ret;
    int sum;
    int i, j;

    sum = 0;
    old = orig;
    new = cmp;

    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++)
            sum += (old[j] - new[j]) * (old[j] - new[j]);

        old += orig_stride;
        new += cmp_stride;
    }

    ret  = (double) sum / (double) (width * height);
    ret  = 10.0 * log10(65025.0 / ret);

    if (max > 128)
        ret /= 50.0;
    else
        ret /= (0.0016 * (double) (max * max)) - (0.38 * (double) max + 72.5);

    return MAX(MIN(ret, 1.0), 0.0);
}

#define DVAL(i) (abs(old[i] - new[i]))
#define HDVAL(i,j) (abs(old[i + j * orig_stride] - new[i + j * cmp_stride]))
static double aae_factor(uint8_t *orig, uint8_t *cmp, int orig_stride,
                         int cmp_stride, int width, int height, uint8_t max)
{
    uint8_t *old, *new;
    double ret;
    double sum;
    double cfmax, cf;
    int i, j;
    int cnt;

    sum = 0.0;
    cnt = 0;
    old = orig;
    new = cmp;

    for (i = 0; i < height; i++) {
        for (j = 7; j < width - 1; j += 8) {
            double calc;

            cnt++;

            calc  = abs(DVAL(j) - DVAL(j + 1));
            calc /= (abs(DVAL(j - 1) - DVAL(j)) + abs(DVAL(j + 1) - DVAL(j + 2)) + 0.0001) / 2.0;

            if (calc > 5.0)
                sum += 1.0;
            else if (calc > 2.0)
                sum += (calc - 2.0) / (5.0 - 2.0);
        }

        old += orig_stride;
        new += cmp_stride;
    }

    old = orig + 7 * orig_stride;
    new = cmp  + 7 * cmp_stride;

    for (i = 7; i < height - 1; i += 8) {
        for (j = 0; j < width; j++) {
            double calc;

            cnt++;

            calc  = abs(DVAL(j) - HDVAL(j, 1));
            calc /= (abs(HDVAL(j, -1) - DVAL(j)) + abs(HDVAL(j, 1) - HDVAL(j, 2)) + 0.0001) / 2.0;

            if (calc > 5.0)
                sum += 1.0;
            else if (calc > 2.0)
                sum += (calc - 2.0) / (5.0 - 2.0);
        }

        old += 8 * orig_stride;
        new += 8 * cmp_stride;
    }
    
    ret = 1 - (sum / (double) cnt);

    if (max > 128)
        cfmax = 0.65;
    else
        cfmax = 0.65 + 0.35 * ((128.0 - (double) max) / 128.0);

    cf = MAX(cfmax, MIN(1, 0.25 + (1000.0 * (double) cnt) / sum));

    return ret * cf;
}

static uint8_t maxluma(uint8_t *buf, int stride, int width, int height)
{
    uint8_t *in = buf;
    uint8_t max = 0;
    int i, j;

    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++)
            max = MAX(in[j], max);

        in += stride;
    }

    return max;
}

double smallfry_metric(uint8_t *inbuf, uint8_t *outbuf, int width, int height)
{
    double p, a, b;
    uint8_t max;

    max = maxluma(inbuf, width, width, height);

    p = psnr_factor(inbuf, outbuf, width, width, width, height, max);
    a = aae_factor(inbuf, outbuf, width, width, width, height, max);

    b = p * 37.1891885161239 + a * 78.5328607296973;

    return b;
}
