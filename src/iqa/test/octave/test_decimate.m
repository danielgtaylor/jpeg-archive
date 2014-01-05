% This file is used to generate the correct output data for the iqa_decimate
% tests.

img_4x4 = [255 128 64 0; 128 64 0 255; 64 0 255 128; 0 255 128 64];
img_5x5 = [0 1 2 3 5; 73 79 83 89 97; 127 131 137 139 149; 179 181 191 193 197; 233 239 241 251 255];

lpf_avg_2x2 = ones(2);
lpf_gaussian_3x3 = [0.07511 0.12384 0.07511; 0.12384 0.20418 0.12384; 0.07511 0.12384 0.07511]; % normalized

% normalize
lpf_avg_2x2 = lpf_avg_2x2/sum(lpf_avg_2x2(:));

disp("\n--- DECIMATE ---\n");

lpf_avg_2x2
lpf_gaussian_3x3

img_4x4

disp("\n4x4 image with linear 2x2 kernel and 2x factor:");
img = imfilter(img_4x4, lpf_avg_2x2, 'symmetric', 'same');
img(1:2:end,1:2:end)

disp("\n4x4 image with gaussian 3x3 kernel and 2x factor:");
img = imfilter(img_4x4, lpf_gaussian_3x3, 'symmetric', 'same');
img(1:2:end,1:2:end)

img_5x5

disp("\n5x5 image with linear 2x2 kernel and 2x factor:");
img = imfilter(img_5x5, lpf_avg_2x2, 'symmetric', 'same');
img(1:2:end,1:2:end)

disp("\n5x5 image with gaussian 3x3 kernel and 2x factor:");
img = imfilter(img_5x5, lpf_gaussian_3x3, 'symmetric', 'same');
img(1:2:end,1:2:end)

disp("\n5x5 image with gaussian 3x3 kernel and 3x factor:");
img = imfilter(img_5x5, lpf_gaussian_3x3, 'symmetric', 'same');
img(1:3:end,1:3:end)

