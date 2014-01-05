% This file is used to generate the correct output data for the iqa_ssim
% tests.

source('ssim_single.m');

% 4CIF 704x480 scaled down 32x
img_22x15 = [
    27 25 83 56 139 147 119 153 147 132 113 147 160 163 169 151 148 120 113 149 132 145;
    0 0 25 25 73 49 148 145 126 127 113 110 116 102 99 118 161 149 149 129 114 137;
    136 154 0 0 24 29 76 63 143 137 114 117 116 117 147 142 119 98 163 139 147 137;
    114 117 141 147 0 0 23 23 66 51 144 144 102 95 104 130 147 161 129 125 85 131;
    97 111 119 107 132 144 0 0 24 26 36 104 135 147 110 136 101 163 170 172 150 124;
    122 135 118 85 116 120 136 147 0 0 24 21 30 51 148 151 120 33 66 79 164 95;
    50 132 130 76 117 142 120 130 137 143 0 0 25 24 48 50 147 150 145 43 120 147;
    170 136 169 155 105 132 43 118 120 143 141 153 0 0 23 23 44 46 144 158 135 149;
    110 94 122 114 149 136 74 57 92 104 99 149 166 166 0 0 25 27 61 46 143 163;
    137 151 132 64 163 174 145 74 205 91 101 88 94 95 97 95 0 0 21 26 150 39;
    149 75 89 108 111 98 19 21 80 229 26 94 100 102 106 94 110 101 0 0 21 24;
    161 38 99 110 95 112 126 94 89 63 108 228 119 111 102 99 97 105 83 99 0 0;
    24 21 155 81 112 93 71 104 111 95 111 235 64 228 105 110 113 108 98 105 112 112;
    0 0 23 20 61 92 91 69 74 107 97 98 99 117 66 92 104 104 106 106 101 74;
    86 83 0 0 21 88 83 92 95 86 94 93 86 111 98 106 86 118 110 108 113 107
];

img_orig = imread('einstein.bmp');
img_blur = imread('blur.bmp');
img_contrast = imread('contrast.bmp');
img_flipvert = imread('flipvertical.bmp');
img_impulse = imread('impulse.bmp');
img_jpg = imread('jpg.bmp');
img_meanshift = imread('meanshift.bmp');
img_cr_orig = imread('Courtright.bmp');
img_cr_noise = imread('Courtright_Noise.bmp');


disp("\n--- 11x11 Gaussian sampling --\n");
disp("\n22x15 Identical:");
mssim = ssim_single(img_22x15, img_22x15)

disp("\n22x15 Different (mean shift +7):");
img_22x15_mod = img_22x15 + [7];
mssim = ssim_single(img_22x15, img_22x15_mod)

disp("\n22x15 Different (2x2 low-pass):");
lpf = ones(2,2);
lpf = lpf/sum(lpf(:));
img_22x15_mod = imfilter(img_22x15, lpf, 'symmetric', 'same');
img_22x15_mod = round(img_22x15_mod);
mssim = ssim_single(img_22x15, img_22x15_mod)

disp("\nEinstein (identical):");
mssim = ssim_single(img_orig, img_orig)

disp("\nEinstein (blur):");
mssim = ssim_single(img_orig, img_blur)

disp("\nEinstein (contrast):");
mssim = ssim_single(img_orig, img_contrast)

disp("\nEinstein (flip vertical):");
mssim = ssim_single(img_orig, img_flipvert)

disp("\nEinstein (impulse):");
mssim = ssim_single(img_orig, img_impulse)

disp("\nEinstein (jpg):");
mssim = ssim_single(img_orig, img_jpg)

disp("\nEinstein (meanshift):");
mssim = ssim_single(img_orig, img_meanshift)

disp("\nCourtright (identical):");
mssim = ssim_single(img_cr_orig, img_cr_orig)

disp("\nCourtright (noise):");
mssim = ssim_single(img_cr_orig, img_cr_noise)


disp("\n--- 8x8 Linear sampling --\n");
K = [0.01 0.03];
window = ones(8);
L = 255;

disp("\n22x15 Identical:");
mssim = ssim_single(img_22x15, img_22x15, K, window, L)

disp("\n22x15 Different (mean shift +7):");
img_22x15_mod = img_22x15 + [7];
mssim = ssim_single(img_22x15, img_22x15_mod, K, window, L)

disp("\n22x15 Different (2x2 low-pass):");
lpf = ones(2,2);
lpf = lpf/sum(lpf(:));
img_22x15_mod = imfilter(img_22x15, lpf, 'symmetric', 'same');
img_22x15_mod = round(img_22x15_mod);
mssim = ssim_single(img_22x15, img_22x15_mod, K, window, L)

disp("\nEinstein (identical):");
mssim = ssim_single(img_orig, img_orig, K, window, L)

disp("\nEinstein (blur):");
mssim = ssim_single(img_orig, img_blur, K, window, L)

disp("\nEinstein (contrast):");
mssim = ssim_single(img_orig, img_contrast, K, window, L)

disp("\nEinstein (flip vertical):");
mssim = ssim_single(img_orig, img_flipvert, K, window, L)

disp("\nEinstein (impulse):");
mssim = ssim_single(img_orig, img_impulse, K, window, L)

disp("\nEinstein (jpg):");
mssim = ssim_single(img_orig, img_jpg, K, window, L)

disp("\nEinstein (meanshift):");
mssim = ssim_single(img_orig, img_meanshift, K, window, L)


disp("\n--- SIMM with modified parameters --\n");
K = [0.025987 0.0173];
window = fspecial('gaussian', 11, 1.5);
L = 187;
a = 0.39;
b = 0.731;
g = 1.12;

disp("\n22x15 Identical:");
mssim = ssim_single(img_22x15, img_22x15, K, window, L, a, b, g)

disp("\n22x15 Different (mean shift +7):");
img_22x15_mod = img_22x15 + [7];
mssim = ssim_single(img_22x15, img_22x15_mod, K, window, L, a, b, g)

disp("\n22x15 Different (2x2 low-pass):");
lpf = ones(2,2);
lpf = lpf/sum(lpf(:));
img_22x15_mod = imfilter(img_22x15, lpf, 'symmetric', 'same');
img_22x15_mod = round(img_22x15_mod);
mssim = ssim_single(img_22x15, img_22x15_mod, K, window, L, a, b, g)

disp("\nEinstein (identical):");
mssim = ssim_single(img_orig, img_orig, K, window, L, a, b, g)

disp("\nEinstein (blur):");
mssim = ssim_single(img_orig, img_blur, K, window, L, a, b, g)

disp("\nEinstein (contrast):");
mssim = ssim_single(img_orig, img_contrast, K, window, L, a, b, g)

disp("\nEinstein (flip vertical):");
mssim = ssim_single(img_orig, img_flipvert, K, window, L, a, b, g)

disp("\nEinstein (impulse):");
mssim = ssim_single(img_orig, img_impulse, K, window, L, a, b, g)

disp("\nEinstein (jpg):");
mssim = ssim_single(img_orig, img_jpg, K, window, L, a, b, g)

disp("\nEinstein (meanshift):");
mssim = ssim_single(img_orig, img_meanshift, K, window, L, a, b, g)

