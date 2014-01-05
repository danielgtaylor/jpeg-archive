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

#include "hptime.h"

#ifdef WIN32
    #include <windows.h>
#else
    #include <time.h>
    #define HPT_CLOCK_TYPE   CLOCK_MONOTONIC
    #define HPT_NANO_SEC     1000000000
#endif

unsigned long long hpt_get_frequency()
{
    unsigned long long freq;

#ifdef WIN32
    QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
#else
    struct timespec cfreq;
    clock_getres(HPT_CLOCK_TYPE, &cfreq);
    /* 1/tick-duration converts time-between-ticks to ticks-per-second. */
    freq = (unsigned long long)( 1.0 / ((double)cfreq.tv_sec + ((double)cfreq.tv_nsec / (double)HPT_NANO_SEC)) );
#endif

    return freq;
}

unsigned long long hpt_get_time()
{
    unsigned long long now;

#ifdef WIN32
    QueryPerformanceCounter((LARGE_INTEGER*)&now);
#else
    struct timespec cnow;
    clock_gettime(HPT_CLOCK_TYPE, &cnow);
    /* Convert to nanosecond ticks. */
    now = (unsigned long long)cnow.tv_sec;
    now *= HPT_NANO_SEC;
    now += cnow.tv_nsec;
#endif

    return now;
}

double hpt_elapsed_time(const unsigned long long start, const unsigned long long end, const unsigned long long freq)
{
    return (double)(end - start) / (double)freq;
}
