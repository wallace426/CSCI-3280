#include "bmp.h"		//	Simple .bmp library
#include<iostream>
#include <fstream>
#include <string>
#include <math.h>
#include <vector>

using namespace std;
#define PI 3.14159265358979
double c(int x) {
	if (x == 0) {
		return 1.0 / sqrt(2.0);
	}
	else {
		return 1.0;
	}
}
int QuantizationMatrix[64] = {3, 5,	7, 9, 11, 13, 15, 17, 5, 7,	9, 11, 13, 15, 17, 19, 7, 9, 11, 13, 15, 17, 19, 21, 9, 11,	13,	15,	17,	19,	21,	23, 11,	13,	15,	17,	19,	21,	23,	25, 13,	15,	17,	19,	21,	23,	25,	27, 15,	17,	19,	21,	23,	25,	27,	29, 17,	19,	21,	23,	25,	27,	29,	31};


int main(int argc, char** argv)
{
	if (argc != 3)
	{
		cout << "Arguments prompt: dct.exe <img_path> <apply_quantization>" << endl;
		return 0;
	}
	string imgPath = argv[1];
	bool need_quantization = stoi(argv[2]);

	//! read input image
	Bitmap s_img(imgPath.c_str());
	int rows = s_img.getHeight(), cols = s_img.getWidth();
	cout << "Apply DCT on image ("<<rows<<", "<<cols<< ")." << endl;
	
	//! preprocess by shifting pixel values by 128
	//TODO
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			unsigned char pixel;
			s_img.getPixel(j, i, pixel);
			s_img.setPixel(j, i, pixel - 128);
		}
	}
	
	//! 2D DCT for every 8x8 block (assume that the input image resolution is fixed to 256)	
	double coeffArray[256][256]={0};
	int blockRow = rows / 8, blockCol = cols / 8;
	for (int i = 0; i < blockRow; i++)
	{
		for (int j = 0; j < blockCol; j++)
		{
			int xpos = j*8, ypos = i*8;
			//! apply DCT on block_ij (basic requirement)
			//TODO
			double tmpR[8][8] = { 0 };
			double tmpF[8][8] = { 0 };
			for (int v = 0; v < 8; v++) {
				for (int u = 0; u < 8; u++) {
					for (int x = 0; x < 8; x++) {
						unsigned char s;
						s_img.getPixel(x + xpos, v + ypos, s);
						tmpR[u][v] += (double)s * cos(u * PI * (2 * x + 1) / 16.0);
					}
					tmpR[u][v] *= c(u) / 2.0;
				}
			}

			for (int u = 0; u < 8; u++) {
				for (int v = 0; v < 8; v++) {
					double f = 0.0;
					for (int y = 0; y < 8; y++) {
						f += tmpR[u][y] * cos(v * PI * (2 * y + 1) / 16.0);
					}
					tmpF[u][v] = f * c(v) / 2.0;
				}

			}

			//! quantize the frequency coefficient of this block (enhanced part)
			//TODO
			if (need_quantization) {
				for (int u = 0; u < 8; u++) {
					for (int v = 0; v < 8; v++) {
						tmpF[u][v] = round(tmpF[u][v] / QuantizationMatrix[u + v * 8]);
					}
				}
			}
			
			for (int u = 0; u < 8; u++) {
				for (int v = 0; v < 8; v++) {
					coeffArray[u + xpos][v + ypos] = tmpF[u][v];
				}
			}
		}
	}
	
	//! output the computed coefficient array
	FILE *fp = fopen("coeffs.txt", "w");
	for (int r = 0; r < rows; r++)
	{
		for (int c = 0; c < cols; c++)
		{
			fprintf(fp, "%3.3lf ", coeffArray[c][r]);
		}
		fprintf(fp, "\n");
	}
	cout << "Result saved!" << endl;
	return 0;
}