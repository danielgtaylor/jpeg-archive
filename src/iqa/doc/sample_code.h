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

/**
 * @page sample_code Sample Code
 * 
 * All the IQA algorithm functions follow a similar prototype:
 * @code iqa_<algorithm>( <reference image>, <modified image>, <width>, <height>, <stride>, <other info>);
 * @endcode
 *
 * @li iqa_mse() - Mean Squared Error
 * @li iqa_psnr() - Peak Signal-to-Noise Ration
 * @li iqa_ssim() - Structural SIMilarity
 * @li iqa_ms_ssim() - Multi-Scale Structure SIMilarity
 *
 * The only header file your code should need to include is "iqa.h". This file defines the main API for IQA.
 *
 * IQA is compiled as a static library so it is linked directly into your application (there is no *.so or *.dll).
 *
 * <br>
 * @section example_1 Example 1
 * Here's a simple example showing how to calculate PSNR:
 *
 * @code
 * #include "iqa.h"
 *
 * // Load reference and modified images.
 *
 * float psnr = iqa_psnr(img_ref, img_mod, width, height, stride);
 * @endcode
 *
 * That's it (seriously).
 * 
 * <br>
 * @section example_2 Example 2
 * Some algorithms support additional configuration parameters. Here's an example that calculates the SSIM index using different weighting factors (a=0.5, b=1.34, c=0.5):
  *
 * @code
 * #include "iqa.h"
 *
 * float ssim;
 * struct iqa_ssim_args args;
 * args.alpha = 0.5f;
 * args.beta  = 1.34f;
 * args.gamma = 0.5f;
 * args.L  = 255;   // default
 * args.K1 = 0.01;  // default
 * args.K2 = 0.03;  // default
 * args.f  = 0;     // default
 *
 * // Load reference and modified images.
 *
 * ssim = iqa_ssim(img_ref, img_mod, width, height, stride, 1, &args);
 * if (ssim == INFINITY)
 *     // Error
 * @endcode
 */
