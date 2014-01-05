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

#include "iqa.h"
#include "test_mse.h"
#include "hptime.h"
#include <stdio.h>
#include <string.h>

static unsigned char img_1x1[1] = { 128 };
static unsigned char img_2x2[4] = { 0, 128, 192, 255 };

int test_mse()
{
    int passed, failures=0;
    float result;
    unsigned long long start, end;
    unsigned char img2[4];

    printf("\nMSE:\n");

    printf("\t1x1 Identical: ");
    memcpy(img2, img_1x1, sizeof(img_1x1));
    start = hpt_get_time();
    result = iqa_mse(img_1x1, img2, 1, 1, 1);
    end = hpt_get_time();
    passed = result==0.0f ? 1 : 0;
    printf("%.5f  (%.3lf ms)\t%s\n", 
        result, 
        hpt_elapsed_time(start,end,hpt_get_frequency()) * 1000.0,
        passed?"PASS":"FAILED");
    failures += passed?0:1;

    printf("\t1x1 Different: ");
    img2[0] += 8;
    start = hpt_get_time();
    result = iqa_mse(img_1x1, img2, 1, 1, 1);
    end = hpt_get_time();
    passed = result==64.0f ? 1 : 0;
    printf("%.5f (%.3lf ms)\t%s\n", 
        result, 
        hpt_elapsed_time(start,end,hpt_get_frequency()) * 1000.0,
        passed?"PASS":"FAILED");
    failures += passed?0:1;

    printf("\t2x2 Identical: ");
    memcpy(img2, img_2x2, sizeof(img_2x2));
    start = hpt_get_time();
    result = iqa_mse(img_2x2, img2, 2, 2, 2);
    end = hpt_get_time();
    passed = result==0.0f ? 1 : 0;
    printf("%.5f  (%.3lf ms)\t%s\n", 
        result, 
        hpt_elapsed_time(start,end,hpt_get_frequency()) * 1000.0,
        passed?"PASS":"FAILED");
    failures += passed?0:1;

    printf("\t2x2 Different: ");
    img2[2] -= 13;
    start = hpt_get_time();
    result = iqa_mse(img_2x2, img2, 2, 2, 2);
    end = hpt_get_time();
    passed = result==42.250f ? 1 : 0;
    printf("%.5f (%.3lf ms)\t%s\n", 
        result, 
        hpt_elapsed_time(start,end,hpt_get_frequency()) * 1000.0,
        passed?"PASS":"FAILED");
    failures += passed?0:1;

    return failures;
}
