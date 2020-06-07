/*

CSCI 3280, Introduction to Multimedia Systems
Spring 2020

Assignment 01 Skeleton

halftone.cpp

*/

#include "bmp.h"		//	Simple .bmp library
#include <iostream>
#include <string>
#include <sstream>

#define MAX_SHADES 3

#define SAFE_FREE(p)  { if(p){ free(p);  (p)=NULL;} }


int main(int argc, char** argv)
{
	//
	//	Your code goes here ....
	//
	//	1. Open BMP file 
	//  you can use the following parameters for testing,
	//  remember to modify it based on the user input.
	if (argc != 4) {
		std::cerr << "The program should have three inputs";
		return 1;
	}
	int imagesize = atoi(argv[2]);
	int patchsize = atoi(argv[3]);

	Bitmap image_data(argv[1]);
	image_data.resize(imagesize);

	//	2. Load image patches
	//
	Bitmap patch_0("patch/0.bmp");
	patch_0.resize(patchsize);
	Bitmap patch_1("patch/1.bmp");
	patch_1.resize(patchsize);
	Bitmap patch_2("patch/2.bmp");
	patch_2.resize(patchsize);


	//
	//	3. Obtain Luminance
	//

	int w = image_data.getWidth();
	int h = image_data.getHeight();
	double* grayscale = new double[w * h];

	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			unsigned char r, g, b;
			image_data.getColor(x, y, r, g, b);
			grayscale[x + y * w] = 0.299 * r + 0.587 * g + 0.114 * b;
		}
	}

	//
	//	4. Quantization
	//

	int patch_h = (h + patchsize - 1) / patchsize;
	int patch_w = (w + patchsize - 1) / patchsize;
	int* quantization = new int[patch_h * patch_w];
	for (int i = 0; i < patch_h; i++) {
		for (int j = 0; j < patch_w; j++) {
			int sy = i * patchsize, sx = j * patchsize;
			double stat = 0.0;
			int num_pixel = 0;
			for (int y = sy; y < sy + patchsize && y < h; y++) {
				for (int x = sx; x < sx + patchsize && x < w; x++) {
					stat += grayscale[x + y * w];
					num_pixel++;
				}
			}
			stat /= num_pixel;


			if (stat < 85.0) {
				quantization[j + i * patch_w] = 0;
			}
			else if (stat < 170.0) {
				quantization[j + i * patch_w] = 1;
			}
			else {
				quantization[j + i * patch_w] = 2;
			}

		}
	}

	//
	//  5. Generate bmp image and parse patches according to quantized image
	//

	Bitmap result(w, h);
	for (int i = 0; i < patch_h; i++) {
		for (int j = 0; j < patch_w; j++) {
			int sy = i * patchsize, sx = j * patchsize;

			for (int y = sy; y < sy + patchsize && y < h; y++) {
				for (int x = sx; x < sx + patchsize && x < w; x++) {
					int dx = x - sx, dy = y - sy;
					unsigned char r, g, b;
					if (quantization[j + i * patch_w] == 0) {
						patch_0.getColor(dx, dy, r, g, b);
					}
					else if (quantization[j + i * patch_w] == 1) {
						patch_1.getColor(dx, dy, r, g, b);
					}
					else if (quantization[j + i * patch_w] == 2) {
						patch_2.getColor(dx, dy, r, g, b);
					}
					result.setColor(x, y, r, g, b);

				}
			}

		}
	}

	result.save("result.bmp");

	//  free memory
	delete quantization;
	delete grayscale;

	return 0;
}