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

#include "convolve.h"
#include "test_convolve.h"
#include <stdio.h>
#include "math_utils.h"
#include <string.h>

static float kernel_1x1[1] = { 1.0f };
#define k2x2 1.0f/4.0f
static float kernel_2x2[4] = { 
    k2x2, k2x2,
    k2x2, k2x2
};
#define k3x3 1.0f/9.0f
static float kernel_3x3[9] = { 
    k3x3, k3x3, k3x3,
    k3x3, k3x3, k3x3,
    k3x3, k3x3, k3x3
};

static float img_1x1[1] = {
    128.0f
};
static float img_2x2[4] = {
    255.0f, 0.0f,
    0.0f, 255.0f
};
static float img_4x3[12] = {
    255.0f, 128.0f, 64.0f, 0.0f,
    128.0f, 64.0f, 0.0f, 255.0f,
    64.0f, 0.0f, 255.0f, 128.0f
};
static float img_4x4[16] = {
    255.0f, 128.0f, 64.0f, 0.0f,
    128.0f, 64.0f, 0.0f, 255.0f,
    64.0f, 0.0f, 255.0f, 128.0f,
    0.0f, 255.0f, 128.0f, 64.0f
};

static int _test_convolve_1x1_kernel();
static int _test_convolve_2x2_kernel();
static int _test_convolve_3x3_kernel();
static int _test_img_filter_1x1_kernel();
static int _test_img_filter_2x2_kernel();
static int _test_img_filter_3x3_kernel();

/*----------------------------------------------------------------------------
 * TEST ENTRY POINT
 *---------------------------------------------------------------------------*/
int test_convolve()
{
    int failure = 0;

    printf("\nConvolve:\n");
    failure += _test_convolve_1x1_kernel();
    failure += _test_convolve_2x2_kernel();
    failure += _test_convolve_3x3_kernel();
    printf("\nImage Filter:\n");
    failure += _test_img_filter_1x1_kernel();
    failure += _test_img_filter_2x2_kernel();
    failure += _test_img_filter_3x3_kernel();

    return failure;
}

/*----------------------------------------------------------------------------
 * _test_convolve_1x1_kernel
 *---------------------------------------------------------------------------*/
int _test_convolve_1x1_kernel()
{
    int rw, rh, passed, failures=0;
    struct _kernel k;
    float img_tmp_1x1[1];
    float img_tmp_2x2[4];

    float result_2x2[4] = {
        255.0f, 0.0f,
        0.0f, 255.0f
    };

    k.w = k.h = 1;
    k.kernel = kernel_1x1;
    k.normalized = 1;

    printf("\t1x1 image, 1x1 kernel:\n");
    printf("\t  w/ result no rw/rh: ");
    memset(img_tmp_1x1,0,sizeof(img_tmp_1x1));
    _iqa_convolve(img_1x1, 1, 1, &k, img_tmp_1x1, 0, 0);
    passed = 0;
    if (img_tmp_1x1[0]==128.0f)
        passed = 1;
    printf("[-,-]\t%s\n", passed?"PASS":"FAILED");
    failures += passed?0:1;

    printf("\t2x2 image, 1x1 kernel:\n");

    /* With result buffer, no rw or rh */
    printf("\t  w/ result no rw/rh: ");
    memset(img_tmp_2x2,0,sizeof(img_tmp_2x2));
    _iqa_convolve(img_2x2, 2, 2, &k, img_tmp_2x2, 0, 0);
    passed = 0;
    if (_matrix_cmp(img_tmp_2x2, result_2x2, 2, 2, 3) == 0)
        passed = 1;
    printf("[-,-]\t%s\n", passed?"PASS":"FAILED");
    failures += passed?0:1;

    /* With result buffer, WITH rw or rh */
    printf("\t  w/ result w/ rw/rh: ");
    memset(img_tmp_2x2,0,sizeof(img_tmp_2x2));
    _iqa_convolve(img_2x2, 2, 2, &k, img_tmp_2x2, &rw, &rh);
    passed = 0;
    if (_matrix_cmp(img_tmp_2x2, result_2x2, 2, 2, 3) == 0 &&
        rw == 2 &&
        rh == 2)
        passed = 1;
    printf("[%i,%i]\t%s\n", rw, rh, passed?"PASS":"FAILED");
    failures += passed?0:1;

    /* In-place, no rw or rh */
    printf("\t  in-place  no rw/rh: ");
    memcpy(img_tmp_2x2, img_2x2, sizeof(img_2x2));
    _iqa_convolve(img_tmp_2x2, 2, 2, &k, 0, 0, 0);
    passed = 0;
    if (_matrix_cmp(img_tmp_2x2, result_2x2, 2, 2, 3) == 0)
        passed = 1;
    printf("[-,-]\t%s\n", passed?"PASS":"FAILED");
    failures += passed?0:1;

    /* I-place, WITH rw or rh */
    printf("\t  in-place  w/ rw/rh: ");
    memcpy(img_tmp_2x2, img_2x2, sizeof(img_2x2));
    _iqa_convolve(img_tmp_2x2, 2, 2, &k, 0, &rw, &rh);
    passed = 0;
    if (_matrix_cmp(img_tmp_2x2, result_2x2, 2, 2, 3) == 0 &&
        rw == 2 &&
        rh == 2)
        passed = 1;
    printf("[%i,%i] \t%s\n", rw, rh, passed?"PASS":"FAILED");
    failures += passed?0:1;

    return failures;
}

/*----------------------------------------------------------------------------
 * _test_convolve_2x2_kernel
 *---------------------------------------------------------------------------*/
int _test_convolve_2x2_kernel()
{
    int passed, failures=0;
    struct _kernel k;
    float img_tmp_4x3[12];

    float result_3x2[6] = {
        143.75f, 64.0f, 79.75f,
        64.0f, 79.75f, 159.5
    };

    printf("\t4x3 image, 2x2 kernel:\n");
    k.w = k.h = 2;
    k.kernel = kernel_2x2;
    k.normalized = 1;

    /* With result buffer, no rw or rh */
    printf("\t  w/ result no rw/rh: ");
    memset(img_tmp_4x3,0,sizeof(img_tmp_4x3));
    _iqa_convolve(img_4x3, 4, 3, &k, img_tmp_4x3, 0, 0);
    passed = 0;
    if (_matrix_cmp(img_tmp_4x3, result_3x2, 3, 2, 3) == 0)
        passed = 1;
    printf("[-,-]\t%s\n", passed?"PASS":"FAILED");
    failures += passed?0:1;

    /* In-place, no rw or rh */
    printf("\t  in-place  no rw/rh: ");
    memcpy(img_tmp_4x3, img_4x3, sizeof(img_4x3));
    _iqa_convolve(img_tmp_4x3, 4, 3, &k, 0, 0, 0);
    passed = 0;
    if (_matrix_cmp(img_tmp_4x3, result_3x2, 3, 2, 3) == 0)
        passed = 1;
    printf("[-,-]\t%s\n", passed?"PASS":"FAILED");
    failures += passed?0:1;

    return failures;
}

/*----------------------------------------------------------------------------
 * _test_convolve_3x3_kernel
 *---------------------------------------------------------------------------*/
int _test_convolve_3x3_kernel()
{
    int passed, failures=0;
    struct _kernel k;
    float img_tmp_4x4[16];

    float result_2x2[4] = {
        106.444f, 99.333f,
        99.333f, 127.667f
    };

    printf("\t4x4 image, 3x3 kernel:\n");
    k.w = k.h = 3;
    k.kernel = kernel_3x3;
    k.normalized = 1;

    /* With result buffer, no rw or rh */
    printf("\t  w/ result no rw/rh: ");
    memset(img_tmp_4x4,0,sizeof(img_tmp_4x4));
    _iqa_convolve(img_4x4, 4, 4, &k, img_tmp_4x4, 0, 0);
    passed = 0;
    if (_matrix_cmp(img_tmp_4x4, result_2x2, 2, 2, 3) == 0)
        passed = 1;
    printf("[-,-]\t%s\n", passed?"PASS":"FAILED");
    failures += passed?0:1;

    /* In-place, no rw or rh */
    printf("\t  in-place  no rw/rh: ");
    memcpy(img_tmp_4x4, img_4x4, sizeof(img_4x4));
    _iqa_convolve(img_tmp_4x4, 4, 4, &k, 0, 0, 0);
    passed = 0;
    if (_matrix_cmp(img_tmp_4x4, result_2x2, 2, 2, 3) == 0)
        passed = 1;
    printf("[-,-]\t%s\n", passed?"PASS":"FAILED");
    failures += passed?0:1;

    return failures;
}

/*----------------------------------------------------------------------------
 * _test_img_filter_1x1_kernel
 *---------------------------------------------------------------------------*/
int _test_img_filter_1x1_kernel()
{
    int passed, failures=0;
    struct _kernel k;
    float img_tmp_1x1[1];
    float img_tmp_2x2[4];

    float result_2x2[4] = {
        255.0f, 0.0f,
        0.0f, 255.0f
    };

    k.w = k.h = 1;
    k.kernel = kernel_1x1;
    k.normalized = 1;
    k.bnd_opt = KBND_SYMMETRIC;

    /* 1x1 image, 1x1 kernel */
    memset(img_tmp_1x1,0,sizeof(img_tmp_1x1));
    _iqa_img_filter(img_1x1, 1, 1, &k, img_tmp_1x1);
    passed = 0;
    if (img_tmp_1x1[0]==128.0f)
        passed = 1;
    printf("\t1x1 image, 1x1 kernel:\t%s\n", passed?"PASS":"FAILED");
    failures += passed?0:1;

    printf("\t2x2 image, 1x1 kernel:\n");

    /* With result buffer */
    printf("\t  w/ result: ");
    memset(img_tmp_2x2,0,sizeof(img_tmp_2x2));
    _iqa_img_filter(img_2x2, 2, 2, &k, img_tmp_2x2);
    passed = 0;
    if (_matrix_cmp(img_tmp_2x2, result_2x2, 2, 2, 3) == 0)
        passed = 1;
    printf("\t\t%s\n", passed?"PASS":"FAILED");
    failures += passed?0:1;

    /* In-place */
    printf("\t  in-place:  ");
    memcpy(img_tmp_2x2, img_2x2, sizeof(img_2x2));
    _iqa_img_filter(img_tmp_2x2, 2, 2, &k, 0);
    passed = 0;
    if (_matrix_cmp(img_tmp_2x2, result_2x2, 2, 2, 3) == 0)
        passed = 1;
    printf("\t\t%s\n", passed?"PASS":"FAILED");
    failures += passed?0:1;

    return failures;
}

/*----------------------------------------------------------------------------
 * _test_img_filter_2x2_kernel
 *---------------------------------------------------------------------------*/
int _test_img_filter_2x2_kernel()
{
    int passed, failures=0;
    struct _kernel k;
    float img_tmp_4x3[12];

    float result_4x3[12] = {
        255.000f, 191.500f, 96.000f, 32.000f,
        191.500f, 143.750f, 64.000f, 79.750f,
        96.000f, 64.000f, 79.750f, 159.500f,
    };

    printf("\t4x3 image, 2x2 kernel:\n");
    k.w = k.h = 2;
    k.kernel = kernel_2x2;
    k.normalized = 1;
    k.bnd_opt = KBND_SYMMETRIC;

    /* With result buffer */
    printf("\t  w/ result: ");
    memset(img_tmp_4x3,0,sizeof(img_tmp_4x3));
    _iqa_img_filter(img_4x3, 4, 3, &k, img_tmp_4x3);
    passed = 0;
    if (_matrix_cmp(img_tmp_4x3, result_4x3, 3, 2, 3) == 0)
        passed = 1;
    printf("\t\t%s\n", passed?"PASS":"FAILED");
    failures += passed?0:1;

    /* In-place */
    printf("\t  in-place:  ");
    memcpy(img_tmp_4x3, img_4x3, sizeof(img_4x3));
    _iqa_img_filter(img_tmp_4x3, 4, 3, &k, 0);
    passed = 0;
    if (_matrix_cmp(img_tmp_4x3, result_4x3, 3, 2, 3) == 0)
        passed = 1;
    printf("\t\t%s\n", passed?"PASS":"FAILED");
    failures += passed?0:1;

    return failures;
}

/*----------------------------------------------------------------------------
 * _test_img_filter_3x3_kernel
 *---------------------------------------------------------------------------*/
int _test_img_filter_3x3_kernel()
{
    int passed, failures=0;
    struct _kernel k;
    float img_tmp_4x4[16];

    float result_4x4[16] = {
        177.333f, 120.667f, 78.111f, 70.889f,
        120.667f, 106.444f, 99.333f, 120.556f,
        78.111f, 99.333f, 127.667f, 141.889f,
        70.889f, 120.556f, 141.889f, 113.667f,
    };

    printf("\t4x4 image, 3x3 kernel:\n");
    k.w = k.h = 3;
    k.kernel = kernel_3x3;
    k.normalized = 1;
    k.bnd_opt = KBND_SYMMETRIC;

    /* With result buffer */
    printf("\t  w/ result: ");
    memset(img_tmp_4x4,0,sizeof(img_tmp_4x4));
    _iqa_img_filter(img_4x4, 4, 4, &k, img_tmp_4x4);
    passed = 0;
    if (_matrix_cmp(img_tmp_4x4, result_4x4, 4, 4, 3) == 0)
        passed = 1;
    printf("\t\t%s\n", passed?"PASS":"FAILED");
    failures += passed?0:1;

    /* In-place*/
    printf("\t  in-place:  ");
    memcpy(img_tmp_4x4, img_4x4, sizeof(img_4x4));
    _iqa_img_filter(img_tmp_4x4, 4, 4, &k, 0);
    passed = 0;
    if (_matrix_cmp(img_tmp_4x4, result_4x4, 4, 4, 3) == 0)
        passed = 1;
    printf("\t\t%s\n", passed?"PASS":"FAILED");
    failures += passed?0:1;

    return failures;
}
