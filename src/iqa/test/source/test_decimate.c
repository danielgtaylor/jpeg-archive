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

#include "decimate.h"
#include "test_decimate.h"
#include "math_utils.h"
#include <stdio.h>
#include <string.h>

static float lpf_avg_2x2[] = {
    0.25f, 0.25f,
    0.25f, 0.25f
};
static float lpf_gaussian_3x3[] = {
    0.075110f, 0.123840f, 0.075110f,
    0.123840f, 0.204180f, 0.123840f,
    0.075110f, 0.123840f, 0.075110f
};

static float img_4x4[] = {
    255.0f, 128.0f, 64.0f, 0.0f,
    128.0f, 64.0f, 0.0f, 255.0f,
    64.0f, 0.0f, 255.0f, 128.0f,
    0.0f, 255.0f, 128.0f, 64.0f
};
static float img_5x5[] = {
    0.0f, 1.0f, 2.0f, 3.0f, 5.0f,
    73.0f, 79.0f, 83.0f, 89.0f, 97.0f,
    127.0f, 131.0f, 137.0f, 139.0f, 149.0f,
    179.0f, 181.0f, 191.0f, 193.0f, 197.0f,
    233.0f, 239.0f, 241.0f, 251.0f, 255.0f
};


static int _test_decimate_2x_4x4();
static int _test_decimate_2x_5x5();
static int _test_decimate_3x_5x5();


/*----------------------------------------------------------------------------
 * TEST ENTRY POINT
 *---------------------------------------------------------------------------*/
int test_decimate()
{
    int failure = 0;

    printf("\nDecimate:\n");
    failure += _test_decimate_2x_4x4();
    failure += _test_decimate_2x_5x5();
    failure += _test_decimate_3x_5x5();

    return failure;
}

/*----------------------------------------------------------------------------
 * _test_decimate_2x_4x4
 *---------------------------------------------------------------------------*/
int _test_decimate_2x_4x4()
{
    int rw, rh, passed, failures=0;
    struct _kernel k_linear, k_gaussian;
    float img_tmp_4x4[16];

    float result_linear[] = {
        255.0f, 96.0f,
        96.0f, 79.750f
    };

    float result_gaussian[] = {
        190.116f, 70.419f,
        70.419f, 131.689f
    };

    k_linear.w = k_linear.h = 2;
    k_linear.kernel = lpf_avg_2x2;
    k_linear.normalized = 1;
    k_linear.bnd_opt = KBND_SYMMETRIC;

    k_gaussian.w = k_gaussian.h = 3;
    k_gaussian.kernel = lpf_gaussian_3x3;
    k_gaussian.normalized = 1;
    k_gaussian.bnd_opt = KBND_SYMMETRIC;

    printf("\t4x4 image, 2x2 linear filter, 2x factor:\n");

    /* With result buffer */
    printf("\t  w/ result no rw/rh: ");
    memset(img_tmp_4x4,0,sizeof(img_tmp_4x4));
    _iqa_decimate(img_4x4, 4, 4, 2, &k_linear, img_tmp_4x4, 0, 0);
    passed = 0;
    if (_matrix_cmp(img_tmp_4x4, result_linear, 2, 2, 3) == 0)
        passed = 1;
    printf("\t%s\n", passed?"PASS":"FAILED");
    failures += passed?0:1;

    printf("\t  w/ result w/ rw/rh: ");
    memset(img_tmp_4x4,0,sizeof(img_tmp_4x4));
    _iqa_decimate(img_4x4, 4, 4, 2, &k_linear, img_tmp_4x4, &rw, &rh);
    passed = 0;
    if (_matrix_cmp(img_tmp_4x4, result_linear, 2, 2, 3) == 0 &&
        rw == 2 &&
        rh == 2)
        passed = 1;
    printf("\t%s\n", passed?"PASS":"FAILED");
    failures += passed?0:1;

    /* In-place*/
    printf("\t  in-place  no rw/rh: ");
    memcpy(img_tmp_4x4, img_4x4, sizeof(img_4x4));
    _iqa_decimate(img_tmp_4x4, 4, 4, 2, &k_linear, 0, 0, 0);
    passed = 0;
    if (_matrix_cmp(img_tmp_4x4, result_linear, 2, 2, 3) == 0)
        passed = 1;
    printf("\t%s\n", passed?"PASS":"FAILED");
    failures += passed?0:1;


    printf("\t4x4 image, 3x3 Gaussian filter, 2x factor:\n");

    /* With result buffer */
    printf("\t  w/ result no rw/rh: ");
    memset(img_tmp_4x4,0,sizeof(img_tmp_4x4));
    _iqa_decimate(img_4x4, 4, 4, 2, &k_gaussian, img_tmp_4x4, 0, 0);
    passed = 0;
    if (_matrix_cmp(img_tmp_4x4, result_gaussian, 2, 2, 3) == 0)
        passed = 1;
    printf("\t%s\n", passed?"PASS":"FAILED");
    failures += passed?0:1;

    return failures;
}

/*----------------------------------------------------------------------------
 * _test_decimate_2x_5x5
 *---------------------------------------------------------------------------*/
int _test_decimate_2x_5x5()
{
    int rw, rh, passed, failures=0;
    struct _kernel k_linear, k_gaussian;
    float img_tmp_5x5[25];

    float result_linear[] = {
        0.0f, 1.5f, 4.0f,
        100.0f, 107.5f, 118.5f,
        206.0f, 213.0f, 224.0f
    };

    float result_gaussian[] = {
        20.656f, 24.349f, 29.215f,
        127.546f, 136.051f, 145.761f,
        219.540f, 228.283f, 238.003f
    };

    k_linear.w = k_linear.h = 2;
    k_linear.kernel = lpf_avg_2x2;
    k_linear.normalized = 1;
    k_linear.bnd_opt = KBND_SYMMETRIC;

    k_gaussian.w = k_gaussian.h = 3;
    k_gaussian.kernel = lpf_gaussian_3x3;
    k_gaussian.normalized = 1;
    k_gaussian.bnd_opt = KBND_SYMMETRIC;

    printf("\t5x5 image, 2x2 linear filter, 2x factor:\n");

    /* With result buffer */
    printf("\t  w/ result no rw/rh: ");
    memset(img_tmp_5x5,0,sizeof(img_tmp_5x5));
    _iqa_decimate(img_5x5, 5, 5, 2, &k_linear, img_tmp_5x5, 0, 0);
    passed = 0;
    if (_matrix_cmp(img_tmp_5x5, result_linear, 3, 3, 3) == 0)
        passed = 1;
    printf("\t%s\n", passed?"PASS":"FAILED");
    failures += passed?0:1;

    printf("\t  w/ result w/ rw/rh: ");
    memset(img_tmp_5x5,0,sizeof(img_tmp_5x5));
    _iqa_decimate(img_5x5, 5, 5, 2, &k_linear, img_tmp_5x5, &rw, &rh);
    passed = 0;
    if (_matrix_cmp(img_tmp_5x5, result_linear, 3, 3, 3) == 0 &&
        rw == 3 &&
        rh == 3)
        passed = 1;
    printf("\t%s\n", passed?"PASS":"FAILED");
    failures += passed?0:1;

    /* In-place*/
    printf("\t  in-place  no rw/rh: ");
    memcpy(img_tmp_5x5, img_5x5, sizeof(img_5x5));
    _iqa_decimate(img_tmp_5x5, 5, 5, 2, &k_linear, 0, 0, 0);
    passed = 0;
    if (_matrix_cmp(img_tmp_5x5, result_linear, 3, 3, 3) == 0)
        passed = 1;
    printf("\t%s\n", passed?"PASS":"FAILED");
    failures += passed?0:1;


    printf("\t5x5 image, 3x3 Gaussian filter, 2x factor:\n");

    /* With result buffer */
    printf("\t  w/ result no rw/rh: ");
    memset(img_tmp_5x5,0,sizeof(img_tmp_5x5));
    _iqa_decimate(img_5x5, 5, 5, 2, &k_gaussian, img_tmp_5x5, 0, 0);
    passed = 0;
    if (_matrix_cmp(img_tmp_5x5, result_gaussian, 3, 3, 3) == 0)
        passed = 1;
    printf("\t%s\n", passed?"PASS":"FAILED");
    failures += passed?0:1;

    return failures;
}

/*----------------------------------------------------------------------------
 * _test_decimate_3x_5x5
 *---------------------------------------------------------------------------*/
int _test_decimate_3x_5x5()
{
    int rw, rh, passed, failures=0;
    struct _kernel k_gaussian;
    float img_tmp_5x5[25];

    float result_gaussian[] = {
        20.656f, 26.918f,
        180.543f, 194.490f
    };

    k_gaussian.w = k_gaussian.h = 3;
    k_gaussian.kernel = lpf_gaussian_3x3;
    k_gaussian.normalized = 1;
    k_gaussian.bnd_opt = KBND_SYMMETRIC;

    printf("\t5x5 image, 3x3 Gaussian filter, 3x factor:\n");

    /* With result buffer */
    printf("\t  w/ result no rw/rh: ");
    memset(img_tmp_5x5,0,sizeof(img_tmp_5x5));
    _iqa_decimate(img_5x5, 5, 5, 3, &k_gaussian, img_tmp_5x5, 0, 0);
    passed = 0;
    if (_matrix_cmp(img_tmp_5x5, result_gaussian, 2, 2, 3) == 0)
        passed = 1;
    printf("\t%s\n", passed?"PASS":"FAILED");
    failures += passed?0:1;

    printf("\t  w/ result w/ rw/rh: ");
    memset(img_tmp_5x5,0,sizeof(img_tmp_5x5));
    _iqa_decimate(img_5x5, 5, 5, 3, &k_gaussian, img_tmp_5x5, &rw, &rh);
    passed = 0;
    if (_matrix_cmp(img_tmp_5x5, result_gaussian, 2, 2, 3) == 0 &&
        rw == 2 &&
        rh == 2)
        passed = 1;
    printf("\t%s\n", passed?"PASS":"FAILED");
    failures += passed?0:1;

    return failures;
}
