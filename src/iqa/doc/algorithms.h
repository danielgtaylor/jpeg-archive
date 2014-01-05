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
 * @page algorithms Algorithms
 * All of the algorithms described here are called full-reference algorithms. This means they required the original undistorted image to compare the distorted image against.
 * 
 * <br>
 * <hr>
 * @section mse MSE
 * Mean Squared Error is the average squared difference between a reference image and a distorted image. It is computed pixel-by-pixel by adding up the squared differences of all the pixels and dividing by the total pixel count.
 *
 * For images A = {a1 .. aM} and B = {b1 .. bM}, where M is the number of pixels:
 * @image html mse_eq.jpg
 * 
 * The squaring of the differences dampens small differences between the 2 pixels but penalizes large ones. 
 *
 * More info: http://en.wikipedia.org/wiki/Mean_squared_error
 *
 * <br>
 * <hr>
 * @section psnr PSNR
 * Peak Signal-to-Noise Ratio is the ratio between the reference signal and the distortion signal in an image, given in decibels. The higher the PSNR, the closer the distorted image is to the original. In general, a higher PSNR value should correlate to a higher quality image, but tests have shown that this isn't always the case. However, PSNR is a popular quality metric because it's easy and fast to calculate while still giving okay results.
 *
 * For images A = {a1 .. aM}, B = {b1 .. bM}, and MAX equal to the maximum possible pixel value (2^8 - 1 = 255 for 8-bit images):
 *
 * @image html psnr_eq.jpg
 *
 * More info: http://en.wikipedia.org/wiki/PSNR
 *
 * <br>
 * <hr>
 * @section ssim SSIM
 * Structural SIMilarity is based on the idea that the human visual system is highly adapted to process structural information, and the algorithm attepts to measure the change in this information between and reference and distorted image. Based on numberous tests, SSIM does a much better job at quantifying subjective image quality than MSE or PSNR.
 *
 * At a high level, SSIM attempts to measure the change in luminance, contrast, and structure in an image. Luminance is modeled as average pixel intensity, constrast by the variance between the reference and distorted image, and structure by the cross-correlation between the 2 images. The resulting values are combined (using exponents referred to as alpha, beta, and gamma) and averaged to generate a final SSIM index value.
 *
 * The original paper defined 2 methods for calculating each local SSIM value: an 8x8 linear or 11x11 circular Gaussian sliding window. IQA defaults to using the Gaussian window that the paper suggests for best results. However, the window type, stabilization constants, and exponents can all be set adjusted by the application.
 *
 * Original paper: https://ece.uwaterloo.ca/~z70wang/publications/ssim.html
 *
 * Here's an interesting article by the authors discussing the limitations of MSE and PSNR as compared to SSIM: https://ece.uwaterloo.ca/~z70wang/publications/SPM09.pdf
 *
 * <br>
 * <hr>
 * @section ms_ssim MS-SSIM
 * Multi-Scale Structural SIMilarity is layered on SSIM. The algorithm calculates multiple SSIM values at multiple image scales (resolutions). By running the algorithm at different scales, the quality of the image is evaluated for different viewing distances. MS-SSIM also puts less emphasis on the luminance component compared to the contrast and structure components. In total, MS-SSIM has been shown to increase the correlation between the MS-SSIM index and subjective quality tests. However, the trade-off is that MS-SSIM takes a few times longer to run than the straight SSIM algorithm.
 *
 * @note MS-SSIM was proposed by Wang, et al, and later extended by Rouse/Hemami. IQA referes to MS-SSIM as 'wang'.
 *
 * Original paper: https://ece.uwaterloo.ca/~z70wang/publications/msssim.pdf
 *
 * <br>
 * <hr>
 * @section ms_ssim_star MS-SSIM*
 * MS-SSIM* is an extension to the original MS-SSIM algorithm proposed by Wang, et al. MS-SSIM* does away with the stabilization constants and defines concrete values for the edge cases. The authors provide data that shows a modest increase in correlation between the MS-SSIM* index and subjective tests, as compared to MS-SSIM. MS-SSIM* is the default algorithm used by the iqa_ms_ssim() method.
 *
 * @note MS-SSIM* is sometimes referred to as Rouse/Hemami in IQA to differentiate it from MS-SSIM by Wang.
 *
 * Original paper: http://foulard.ece.cornell.edu/publications/dmr_hvei2008_paper.pdf
 *
 */
