% This file is used to generate the correct output data for the iqa_convolve and
% iqa_img_filter tests.

img_1x1 = [128];
img_2x2 = [255 0; 0 255];
img_4x3 = [255 128 64 0; 128 64 0 255; 64 0 255 128];
img_4x4 = [255 128 64 0; 128 64 0 255; 64 0 255 128; 0 255 128 64];

kernel_1x1 = ones(1);
kernel_2x2 = ones(2);
kernel_3x3 = ones(3);

% normalize
kernel_2x2 = kernel_2x2/sum(kernel_2x2(:));
kernel_3x3 = kernel_3x3/sum(kernel_3x3(:));

disp("\n--- CONVOLVE ---");
disp("\n1x1 image with 1x1 kernel:");
filter2(kernel_1x1, img_1x1, 'valid')

disp("\n2x2 image with 1x1 kernel:");
filter2(kernel_1x1, img_2x2, 'valid')

disp("\n2x2 image with 3x3 kernel:");
filter2(kernel_3x3, img_2x2, 'valid')

disp("\n4x3 image with 2x2 kernel:");
filter2(kernel_2x2, img_4x3, 'valid')

disp("\n4x4 image with 3x3 kernel:");
filter2(kernel_3x3, img_4x4, 'valid')


disp("\n--- IMG_FILTER ---");
disp("\n1x1 image with 1x1 kernel:");
imfilter(img_1x1, kernel_1x1, 'symmetric', 'same')

disp("\n2x2 image with 1x1 kernel:");
imfilter(img_2x2, kernel_1x1, 'symmetric', 'same')

disp("\n4x3 image with 2x2 kernel:");
imfilter(img_4x3, kernel_2x2, 'symmetric', 'same')

disp("\n4x4 image with 3x3 kernel:");
imfilter(img_4x4, kernel_3x3, 'symmetric', 'same')
