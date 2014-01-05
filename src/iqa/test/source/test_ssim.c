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

#include "test_ssim.h"
#include "iqa.h"
#include "convolve.h"
#include "bmp.h"
#include "hptime.h"
#include "math_utils.h"
#include <stdio.h>
#include <string.h>

static const int img_width = 22;
static const int img_height = 15;
static const int img_stride = 23;
static const unsigned char img_22x15[] = {
    27, 25, 83, 56, 139, 147, 119, 153, 147, 132, 113, 147, 160, 163, 169, 151, 148, 120, 113, 149, 132, 145, 0,
    0, 0, 25, 25, 73, 49, 148, 145, 126, 127, 113, 110, 116, 102, 99, 118, 161, 149, 149, 129, 114, 137, 0,
    136, 154, 0, 0, 24, 29, 76, 63, 143, 137, 114, 117, 116, 117, 147, 142, 119, 98, 163, 139, 147, 137, 0,
    114, 117, 141, 147, 0, 0, 23, 23, 66, 51, 144, 144, 102, 95, 104, 130, 147, 161, 129, 125, 85, 131, 0,
    97, 111, 119, 107, 132, 144, 0, 0, 24, 26, 36, 104, 135, 147, 110, 136, 101, 163, 170, 172, 150, 124, 0,
    122, 135, 118, 85, 116, 120, 136, 147, 0, 0, 24, 21, 30, 51, 148, 151, 120, 33, 66, 79, 164, 95, 0,
    50, 132, 130, 76, 117, 142, 120, 130, 137, 143, 0, 0, 25, 24, 48, 50, 147, 150, 145, 43, 120, 147, 0,
    170, 136, 169, 155, 105, 132, 43, 118, 120, 143, 141, 153, 0, 0, 23, 23, 44, 46, 144, 158, 135, 149, 0,
    110, 94, 122, 114, 149, 136, 74, 57, 92, 104, 99, 149, 166, 166, 0, 0, 25, 27, 61, 46, 143, 163, 0,
    137, 151, 132, 64, 163, 174, 145, 74, 205, 91, 101, 88, 94, 95, 97, 95, 0, 0, 21, 26, 150, 39, 0,
    149, 75, 89, 108, 111, 98, 19, 21, 80, 229, 26, 94, 100, 102, 106, 94, 110, 101, 0, 0, 21, 24, 0,
    161, 38, 99, 110, 95, 112, 126, 94, 89, 63, 108, 228, 119, 111, 102, 99, 97, 105, 83, 99, 0, 0, 0,
    24, 21, 155, 81, 112, 93, 71, 104, 111, 95, 111, 235, 64, 228, 105, 110, 113, 108, 98, 105, 112, 112, 0,
    0, 0, 23, 20, 61, 92, 91, 69, 74, 107, 97, 98, 99, 117, 66, 92, 104, 104, 106, 106, 101, 74, 0,
    86, 83, 0, 0, 21, 88, 83, 92, 95, 86, 94, 93, 86, 111, 98, 106, 86, 118, 110, 108, 113, 107, 0,
};

static float lpf_linear_2x2[] = {
    0.25f, 0.25f,
    0.25f, 0.25f,
};

static const struct iqa_ssim_args ssim_args = {
    0.39f,      /* alpha */
    0.731f,     /* beta */
    1.12f,      /* gamma */
    187,        /* L */
    0.025987f,  /* K1 */
    0.0173f,    /* K2 */
    1           /* factor */
};


/* Defines the answer format */
struct answer {
    float value;        /**< Expected result */
    int   precision;    /**< Digits of precision */
};

static const struct answer ans_key_22x15_gauss[] = {
    {1.00000f, 5},  /* Identical */
    {0.99668f, 5},  /* Mean Shift +7 */
    {0.68944f, 5},  /* 2x2 low-pass filter */
};

static const struct answer ans_key_22x15_linear[] = {
    {1.00000f, 5},  /* Identical */
    {0.99737f, 5},  /* Mean Shift +7 */
    {0.71679f, 5},  /* 2x2 low-pass filter */
};

static const struct answer ans_key_22x15_args[] = {
    {1.00000f, 5},    /* Identical */
    {0.99870f, 5},    /* Mean Shift +7 */
    {0.67986f, 5},    /* 2x2 low-pass filter */
};

static const struct answer ans_key_einstein_gauss[] = {
    {1.00000f, 5},  /* Identical */
    {0.69408f, 5},  /* Blur */
    {0.91327f, 5},  /* Contrast */
    {0.28771f, 5},  /* Flip Vertical */
    {0.83957f, 5},  /* Impulse */
    {0.66246f, 5},  /* JPEG */
    {0.98836f, 5},  /* Mean Shift */
};

static const struct answer ans_key_einstein_linear[] = {
    {1.00000f, 5},  /* Identical */
    {0.72422f, 5},  /* Blur */
    {0.91273f, 5},  /* Contrast */
    {0.25001f, 5},  /* Flip Vertical */
    {0.82213f, 5},  /* Impulse */
    {0.68550f, 5},  /* JPEG */
    {0.98845f, 5},  /* Mean Shift */
};

/* NOTE: Values verified. Different from Octave due to float precision. */
static const struct answer ans_key_einstein_args[] = {
    {1.00000f, 5},    /* Identical */
    {0.56509f, 5},    /* Blur */
    {0.94412f, 5},    /* Contrast */
    {0.13106f, 5},    /* Flip Vertical */
    {0.81263f, 5},    /* Impulse */
    {0.50202f, 4},    /* JPEG (rounding error on 64-bit)*/
    {0.99542f, 5},    /* Mean Shift */
};

static const struct answer ans_key_courtright[] = {
    {1.00000f, 5},    /* Identical */
    {0.74989f, 2},    /* Noise (rounding error) */
};


#define BMP_ORIGINAL    "einstein.bmp"
#define BMP_BLUR        "blur.bmp"
#define BMP_CONTRAST    "contrast.bmp"
#define BMP_FLIPVERT    "flipvertical.bmp"
#define BMP_IMPULSE     "impulse.bmp"
#define BMP_JPG         "jpg.bmp"
#define BMP_MEANSHIFT   "meanshift.bmp"
#define BMP_CR_ORIGINAL "Courtright.bmp"
#define BMP_CR_NOISE    "Courtright_Noise.bmp"

static int _test_ssim_22x15(int gaussian, const struct answer *answers, const struct iqa_ssim_args *args);
static int _test_ssim_einstein_bmp(int gaussian, const struct answer *answers, const struct iqa_ssim_args *args);
static int _test_ssim_courtright_bmp(int gaussian, const struct answer *answers, const struct iqa_ssim_args *args);


/*----------------------------------------------------------------------------
 * TEST ENTRY POINT
 *---------------------------------------------------------------------------*/
int test_ssim()
{
    int failure = 0;

    printf("\nSSIM:\n");
    failure += _test_ssim_22x15(1, ans_key_22x15_gauss, 0);
    failure += _test_ssim_22x15(0, ans_key_22x15_linear, 0);
    failure += _test_ssim_22x15(1, ans_key_22x15_args, &ssim_args);
    failure += _test_ssim_einstein_bmp(1, ans_key_einstein_gauss, 0);
    failure += _test_ssim_einstein_bmp(0, ans_key_einstein_linear, 0);
    failure += _test_ssim_einstein_bmp(1, ans_key_einstein_args, &ssim_args);
    failure += _test_ssim_courtright_bmp(1, ans_key_courtright, 0);

    return failure;
}

/*----------------------------------------------------------------------------
 * _test_ssim_22x15
 *---------------------------------------------------------------------------*/
int _test_ssim_22x15(int gaussian, const struct answer *answers, const struct iqa_ssim_args *args)
{
    int x,y;
    int passed, failures=0;
    float result;
    unsigned long long start, end;
    unsigned char img_tmp[sizeof(img_22x15)];
    float img_tmp_f[sizeof(img_22x15)];
    struct _kernel lpf;

    printf("\t22x15 Image (%s%s):\n", gaussian?"Gaussian":"Linear",args?" - Custom Args":"");

    printf("\t  Identical: ");
    memcpy(img_tmp, img_22x15, sizeof(img_22x15));
    start = hpt_get_time();
    result = iqa_ssim(img_22x15, img_tmp, img_width, img_height, img_stride, gaussian, args);
    end = hpt_get_time();
    passed = _cmp_float(result, answers[0].value, answers[0].precision) ? 0 : 1;
    printf("\t\t%.5f  (%.3lf ms)\t%s\n", 
        result, 
        hpt_elapsed_time(start,end,hpt_get_frequency()) * 1000.0,
        passed?"PASS":"FAILED");
    failures += passed?0:1;

    printf("\t  Mean-shifted +7: ");
    memcpy(img_tmp, img_22x15, sizeof(img_22x15));
    for (y=0; y < img_height; ++y) {
        for (x=0; x < img_width; ++x) {
            img_tmp[y*img_stride+x] += 7;
        }
    }
    start = hpt_get_time();
    result = iqa_ssim(img_22x15, img_tmp, img_width, img_height, img_stride, gaussian, args);
    end = hpt_get_time();
    passed = _cmp_float(result, answers[1].value, answers[1].precision) ? 0 : 1;
    printf("\t%.5f  (%.3lf ms)\t%s\n", 
        result, 
        hpt_elapsed_time(start,end,hpt_get_frequency()) * 1000.0,
        passed?"PASS":"FAILED");
    failures += passed?0:1;

    printf("\t  2x2 linear low-pass: ");
    lpf.kernel = lpf_linear_2x2;
    lpf.w = lpf.h = 2;
    lpf.normalized = 1;
    lpf.bnd_opt = KBND_SYMMETRIC;
    for (y=0; y < img_height; ++y) {
        for (x=0; x < img_width; ++x) {
            img_tmp_f[y*img_width+x] = (float)img_22x15[y*img_stride+x];
        }
    }
    if (_iqa_img_filter(img_tmp_f, img_width, img_height, &lpf, 0)) {
        printf("FAILED\n");
        failures++;
    }
    else {
        for (y=0; y < img_height; ++y) {
            for (x=0; x < img_width; ++x) {
                img_tmp[y*img_stride+x] = (unsigned char)_round(img_tmp_f[y*img_width+x]);
            }
        }
        start = hpt_get_time();
        result = iqa_ssim(img_22x15, img_tmp, img_width, img_height, img_stride, gaussian, args);
        end = hpt_get_time();
        passed = _cmp_float(result, answers[2].value, answers[2].precision) ? 0 : 1;
        printf("\t%.5f  (%.3lf ms)\t%s\n", 
            result, 
            hpt_elapsed_time(start,end,hpt_get_frequency()) * 1000.0,
            passed?"PASS":"FAILED");
        failures += passed?0:1;
    }

    return failures;
}

/*----------------------------------------------------------------------------
 * _test_ssim_einstein_bmp
 *---------------------------------------------------------------------------*/
int _test_ssim_einstein_bmp(int gaussian, const struct answer *answers, const struct iqa_ssim_args *args)
{
    struct bmp orig, cmp;
    int passed, failures=0;
    float result;
    unsigned long long start, end;

    printf("\tEinstein (%s%s):\n", gaussian?"Gaussian":"Linear",args?" - Custom Args":"");

    if (load_bmp(BMP_ORIGINAL, &orig)) {
        printf("FAILED to load \'%s\'\n", BMP_ORIGINAL);
        return 1;
    }

    printf("\t  Identical: ");
    start = hpt_get_time();
    result = iqa_ssim(orig.img, orig.img, orig.w, orig.h, orig.stride, gaussian, args);
    end = hpt_get_time();
    passed = _cmp_float(result, answers[0].value, answers[0].precision) ? 0 : 1;
    printf("\t\t%.5f  (%.3lf ms)\t%s\n", 
        result, 
        hpt_elapsed_time(start,end,hpt_get_frequency()) * 1000.0,
        passed?"PASS":"FAILED");
    failures += passed?0:1;

    printf("\t  Blur: ");
    if (load_bmp(BMP_BLUR, &cmp)) {
        printf("FAILED to load \'%s\'\n", BMP_BLUR);
        failures++;
    }
    else {
        start = hpt_get_time();
        result = iqa_ssim(orig.img, cmp.img, orig.w, orig.h, orig.stride, gaussian, args);
        end = hpt_get_time();
        passed = _cmp_float(result, answers[1].value, answers[1].precision) ? 0 : 1;
        printf("\t\t%.5f  (%.3lf ms)\t%s\n", 
            result, 
            hpt_elapsed_time(start,end,hpt_get_frequency()) * 1000.0,
            passed?"PASS":"FAILED");
        failures += passed?0:1;
        free_bmp(&cmp);
    }

    printf("\t  Contrast: ");
    if (load_bmp(BMP_CONTRAST, &cmp)) {
        printf("FAILED to load \'%s\'\n", BMP_CONTRAST);
        failures++;
    }
    else {
        start = hpt_get_time();
        result = iqa_ssim(orig.img, cmp.img, orig.w, orig.h, orig.stride, gaussian, args);
        end = hpt_get_time();
        passed = _cmp_float(result, answers[2].value, answers[2].precision) ? 0 : 1;
        printf("\t\t%.5f  (%.3lf ms)\t%s\n", 
            result, 
            hpt_elapsed_time(start,end,hpt_get_frequency()) * 1000.0,
            passed?"PASS":"FAILED");
        failures += passed?0:1;
        free_bmp(&cmp);
    }

    printf("\t  Flip Vertical: ");
    if (load_bmp(BMP_FLIPVERT, &cmp)) {
        printf("FAILED to load \'%s\'\n", BMP_FLIPVERT);
        failures++;
    }
    else {
        start = hpt_get_time();
        result = iqa_ssim(orig.img, cmp.img, orig.w, orig.h, orig.stride, gaussian, args);
        end = hpt_get_time();
        passed = _cmp_float(result, answers[3].value, answers[3].precision) ? 0 : 1;
        printf("\t%.5f  (%.3lf ms)\t%s\n", 
            result, 
            hpt_elapsed_time(start,end,hpt_get_frequency()) * 1000.0,
            passed?"PASS":"FAILED");
        failures += passed?0:1;
        free_bmp(&cmp);
    }

    printf("\t  Impulse: ");
    if (load_bmp(BMP_IMPULSE, &cmp)) {
        printf("FAILED to load \'%s\'\n", BMP_IMPULSE);
        failures++;
    }
    else {
        start = hpt_get_time();
        result = iqa_ssim(orig.img, cmp.img, orig.w, orig.h, orig.stride, gaussian, args);
        end = hpt_get_time();
        passed = _cmp_float(result, answers[4].value, answers[4].precision) ? 0 : 1;
        printf("\t\t%.5f  (%.3lf ms)\t%s\n", 
            result, 
            hpt_elapsed_time(start,end,hpt_get_frequency()) * 1000.0,
            passed?"PASS":"FAILED");
        failures += passed?0:1;
        free_bmp(&cmp);
    }

    printf("\t  Jpeg: ");
    if (load_bmp(BMP_JPG, &cmp)) {
        printf("FAILED to load \'%s\'\n", BMP_JPG);
        failures++;
    }
    else {
        start = hpt_get_time();
        result = iqa_ssim(orig.img, cmp.img, orig.w, orig.h, orig.stride, gaussian, args);
        end = hpt_get_time();
        passed = _cmp_float(result, answers[5].value, answers[5].precision) ? 0 : 1;
        printf("\t\t%.5f  (%.3lf ms)\t%s\n", 
            result, 
            hpt_elapsed_time(start,end,hpt_get_frequency()) * 1000.0,
            passed?"PASS":"FAILED");
        failures += passed?0:1;
        free_bmp(&cmp);
    }

    printf("\t  Meanshift: ");
    if (load_bmp(BMP_MEANSHIFT, &cmp)) {
        printf("FAILED to load \'%s\'\n", BMP_MEANSHIFT);
        failures++;
    }
    else {
        start = hpt_get_time();
        result = iqa_ssim(orig.img, cmp.img, orig.w, orig.h, orig.stride, gaussian, args);
        end = hpt_get_time();
        passed = _cmp_float(result, answers[6].value, answers[6].precision) ? 0 : 1;
        printf("\t\t%.5f  (%.3lf ms)\t%s\n", 
            result, 
            hpt_elapsed_time(start,end,hpt_get_frequency()) * 1000.0,
            passed?"PASS":"FAILED");
        failures += passed?0:1;
        free_bmp(&cmp);
    }

    free_bmp(&orig);
    return failures;
}

/*----------------------------------------------------------------------------
 * _test_ssim_courtright_bmp
 *---------------------------------------------------------------------------*/
int _test_ssim_courtright_bmp(int gaussian, const struct answer *answers, const struct iqa_ssim_args *args)
{
    struct bmp orig, cmp;
    int passed, failures=0;
    float result;
    unsigned long long start, end;

    printf("\tCourtright (%s%s):\n", gaussian?"Gaussian":"Linear",args?" - Custom Args":"");

    if (load_bmp(BMP_CR_ORIGINAL, &orig)) {
        printf("FAILED to load \'%s\'\n", BMP_CR_ORIGINAL);
        return 1;
    }

    printf("\t  Identical: ");
    start = hpt_get_time();
    result = iqa_ssim(orig.img, orig.img, orig.w, orig.h, orig.stride, gaussian, args);
    end = hpt_get_time();
    passed = _cmp_float(result, answers[0].value, answers[0].precision) ? 0 : 1;
    printf("\t\t%.5f  (%.3lf ms)\t%s\n", 
        result, 
        hpt_elapsed_time(start,end,hpt_get_frequency()) * 1000.0,
        passed?"PASS":"FAILED");
    failures += passed?0:1;

    printf("\t  Noise: ");
    if (load_bmp(BMP_CR_NOISE, &cmp)) {
        printf("FAILED to load \'%s\'\n", BMP_CR_NOISE);
        failures++;
    }
    else {
        start = hpt_get_time();
        result = iqa_ssim(orig.img, cmp.img, orig.w, orig.h, orig.stride, gaussian, args);
        end = hpt_get_time();
        passed = _cmp_float(result, answers[1].value, answers[1].precision) ? 0 : 1;
        printf("\t\t%.5f  (%.3lf ms)\t%s\n", 
            result, 
            hpt_elapsed_time(start,end,hpt_get_frequency()) * 1000.0,
            passed?"PASS":"FAILED");
        failures += passed?0:1;
        free_bmp(&cmp);
    }

    free_bmp(&orig);
    return failures;
}

