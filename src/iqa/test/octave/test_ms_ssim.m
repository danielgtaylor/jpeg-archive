% This file is used to generate the correct output data for the 
% iqa_ms_ssim tests.

source('ms_ssim.m');


img_orig = imread('einstein.bmp');
img_blur = imread('blur.bmp');
img_contrast = imread('contrast.bmp');
img_flipvert = imread('flipvertical.bmp');
img_impulse = imread('impulse.bmp');
img_jpg = imread('jpg.bmp');
img_meanshift = imread('meanshift.bmp');
img_cr_orig = imread('Courtright.bmp');
img_cr_noise = imread('Courtright_Noise.bmp');


disp("\n\n--- MS-SSIM Rouse/Hemami --\n");

disp("\nEinstein (identical):");
mssim = ms_ssim(img_orig, img_orig, 0)

disp("\nEinstein (blur):");
mssim = ms_ssim(img_orig, img_blur, 0)

disp("\nEinstein (contrast):");
mssim = ms_ssim(img_orig, img_contrast, 0)

disp("\nEinstein (flip vertical):");
mssim = ms_ssim(img_orig, img_flipvert, 0)

disp("\nEinstein (impulse):");
mssim = ms_ssim(img_orig, img_impulse, 0)

disp("\nEinstein (jpg):");
mssim = ms_ssim(img_orig, img_jpg, 0)

disp("\nEinstein (meanshift):");
mssim = ms_ssim(img_orig, img_meanshift, 0)

disp("\nCourtright (identical):");
mssim = ms_ssim(img_cr_orig, img_cr_orig, 0)

disp("\nCourtright (noise):");
mssim = ms_ssim(img_cr_orig, img_cr_noise, 0)


disp("\n\n--- MS-SSIM Wang --\n");

disp("\nEinstein (identical):");
mssim = ms_ssim(img_orig, img_orig, 1)

disp("\nEinstein (blur):");
mssim = ms_ssim(img_orig, img_blur, 1)

disp("\nEinstein (contrast):");
mssim = ms_ssim(img_orig, img_contrast, 1)

disp("\nEinstein (flip vertical):");
mssim = ms_ssim(img_orig, img_flipvert, 1)

disp("\nEinstein (impulse):");
mssim = ms_ssim(img_orig, img_impulse, 1)

disp("\nEinstein (jpg):");
mssim = ms_ssim(img_orig, img_jpg, 1)

disp("\nEinstein (meanshift):");
mssim = ms_ssim(img_orig, img_meanshift, 1)


disp("\n\n--- MS-SSIM Linear Window (R/H) --\n");

disp("\nEinstein (identical):");
mssim = ms_ssim(img_orig, img_orig, 0, 0)

disp("\nEinstein (blur):");
mssim = ms_ssim(img_orig, img_blur, 0, 0)

disp("\nEinstein (contrast):");
mssim = ms_ssim(img_orig, img_contrast, 0, 0)

disp("\nEinstein (flip vertical):");
mssim = ms_ssim(img_orig, img_flipvert, 0, 0)

disp("\nEinstein (impulse):");
mssim = ms_ssim(img_orig, img_impulse, 0, 0)

disp("\nEinstein (jpg):");
mssim = ms_ssim(img_orig, img_jpg, 0, 0)

disp("\nEinstein (meanshift):");
mssim = ms_ssim(img_orig, img_meanshift, 0, 0)


disp("\n\n--- MS-SSIM Scale=4 --\n");
scale = 4;
a = [0 0 0 0.25];
b = [0.25 0.25 0.25 0.25];
g = [0.25 0.25 0.25 0.25];

disp("\nEinstein (identical):");
mssim = ms_ssim(img_orig, img_orig, 0, 1, scale, a, b, g)

disp("\nEinstein (blur):");
mssim = ms_ssim(img_orig, img_blur, 0, 1, scale, a, b, g)

disp("\nEinstein (contrast):");
mssim = ms_ssim(img_orig, img_contrast, 0, 1, scale, a, b, g)

disp("\nEinstein (flip vertical):");
mssim = ms_ssim(img_orig, img_flipvert, 0, 1, scale, a, b, g)

disp("\nEinstein (impulse):");
mssim = ms_ssim(img_orig, img_impulse, 0, 1, scale, a, b, g)

disp("\nEinstein (jpg):");
mssim = ms_ssim(img_orig, img_jpg, 0, 1, scale, a, b, g)

disp("\nEinstein (meanshift):");
mssim = ms_ssim(img_orig, img_meanshift, 0, 1, scale, a, b, g)