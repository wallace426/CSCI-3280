#include "bmp.h"		//	Simple .bmp library
#include<iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

#define Baseline 30.0
#define Focal_Length 100
#define Image_Width 35.0
#define Image_Height 35.0
#define Resolution_Row 512
#define Resolution_Col 512
#define View_Grid_Row 9
#define View_Grid_Col 9

struct Point3d
{
	double x;
	double y;
	double z;
	Point3d(double x_, double y_, double z_) :x(x_), y(y_), z(z_) {}
};

struct Point2d
{
	double x;
	double y;
	Point2d(double x_, double y_) :x(x_), y(y_) {}
};

void calcInterceptPoint(Point3d& m, Point3d& Vl, Point3d& n, Point3d& Vp, Point3d& res);
void getViewNeighbors(Point3d& p, int& us, int& ut, double& alpha, double& beta);
void bilinearInterp(Point3d& res, Point3d& p0, Point3d& p1, Point3d& p2, Point3d& p3, double alpha, double beta);

void calcPixelValue(Bitmap& bitmap, Point3d& vec, Point3d& res);

int main(int argc, char** argv)
{
	if (argc != 6)
	{
		// insufficient args
		cout << "Arguments prompt: viewSynthesis.exe <LF_dir> <X Y Z> <focal_length>" << endl;
		return 0;
	}
	string LFDir = argv[1];
	double Vx = stod(argv[2]), Vy = stod(argv[3]), Vz = stod(argv[4]);
	double targetFocalLen = stod(argv[5]);

	vector<Bitmap> viewImageList;
	//! loading light field views
	for (int i = 0; i < View_Grid_Col * View_Grid_Row; i++)
	{
		char name[128];
		sprintf(name, "/cam%03d.bmp", i + 1);
		string filePath = LFDir + name;
		Bitmap view_i(filePath.c_str());
		viewImageList.push_back(view_i);
	}

	Bitmap targetView(Resolution_Col, Resolution_Row);
	cout << "Synthesizing image from viewpoint: (" << Vx << "," << Vy << "," << Vz << ") with focal length: " << targetFocalLen << endl;

	double pixel_width = Image_Width / Resolution_Col;
	double pixel_height = Image_Height / Resolution_Row;


	//! resample pixels of the target view one by one
	for (int r = 0; r < Resolution_Row; r++)
	{
		for (int c = 0; c < Resolution_Col; c++)
		{

			Point3d rayRGB(0, 0, 0);
			//! resample the pixel value of this ray: TODO

			// convert (col, row) to image coord system

			double x0 = (Image_Width / 2.0) - (c + 0.5) * pixel_width;
			double y0 = (r + 0.5) * pixel_height - (Image_Height / 2.0);


			// target view point
			Point3d P(Vx, Vy, Vz);
			// vector of current ray
			Point3d crt_ray(x0, y0, -targetFocalLen);

			// calculate intercept point with camera plane
			Point3d n(0, 0, 0);
			Point3d Vp(0, 0, -1);
			Point3d interp_camplane(0, 0, 0);
			calcInterceptPoint(P, crt_ray, n, Vp, interp_camplane);

			// handle ray out of range
			if ((interp_camplane.x > 120) || (interp_camplane.x < -120) ||
				(interp_camplane.y > 120) || (interp_camplane.y < -120))
			{
				rayRGB.x = 0;
				rayRGB.y = 0;
				rayRGB.z = 0;
			}
			else
			{
				// find interp neighbors of current ray
				int xs;
				int xt;
				double alpha, beta;
				getViewNeighbors(interp_camplane, xs, xt, alpha, beta);

				// strange big Z version
				unsigned char R;
				unsigned char G;
				unsigned char B;
				Point3d p0(0, 0, 0);
				Point3d p1(0, 0, 0);
				Point3d p2(0, 0, 0);
				Point3d p3(0, 0, 0);


				calcPixelValue(viewImageList[xt * 9 + xs], crt_ray, p0);
				calcPixelValue(viewImageList[xt * 9 + xs - 1], crt_ray, p1);
				calcPixelValue(viewImageList[(xt + 1) * 9 + xs], crt_ray, p2);
				calcPixelValue(viewImageList[(xt + 1) * 9 + xs - 1], crt_ray, p3);

				// bilinear interp
				bilinearInterp(rayRGB, p0, p1, p2, p3, alpha, beta);

				//rayRGB.x = p0.x;
				//rayRGB.y = p0.y;
				//rayRGB.z = p0.z;
			}

			//! record the resampled pixel value
			targetView.setColor(c, r, (unsigned char)(rayRGB.x), (unsigned char)(rayRGB.y), (unsigned char)(rayRGB.z));
		}
	}
	string savePath = "newView.bmp";
	targetView.save(savePath.c_str());
	cout << "Result saved!" << endl;
	return 0;
}

void calcInterceptPoint(Point3d& m, Point3d& Vl, Point3d& n, Point3d& Vp, Point3d& res)
{
	double t = 0;
	t = ((n.x - m.x) * Vp.x + (n.y - m.y) * Vp.y + (n.z - m.z) * Vp.z) / (Vp.x * Vl.x + Vp.y * Vl.y + Vp.z * Vl.z);

	res.x = m.x + Vl.x * t;
	res.y = m.y + Vl.y * t;
	res.z = m.z + Vl.z * t;
}

void getViewNeighbors(Point3d& p, int& xs, int& xt, double& alpha, double& beta)
{
	xs = (p.x + 120) / Baseline;
	xt = (p.y + 120) / Baseline;

	while (xs * Baseline < p.x + 120) {
		xs++;
	}

	while (xt * Baseline < p.y + 120) {
		xt++;
	}


	alpha = (xs * Baseline - (p.x + 120)) / Baseline;
	beta = (xt * Baseline - (p.y + 120)) / Baseline;

	xs = (xs == 0) ? 1 : xs;
	xt = (xt == 0) ? 1 : xt;
	xt = 8 - xt;
}

void bilinearInterp(Point3d& res, Point3d& p0, Point3d& p1, Point3d& p2, Point3d& p3, double alpha, double beta)
{
	Point3d tmp1_color(0, 0, 0);
	tmp1_color.x = (1 - alpha) * p0.x + alpha * p1.x;
	tmp1_color.y = (1 - alpha) * p0.y + alpha * p1.y;
	tmp1_color.z = (1 - alpha) * p0.z + alpha * p1.z;

	Point3d tmp2_color(0, 0, 0);
	tmp2_color.x = (1 - alpha) * p2.x + alpha * p3.x;
	tmp2_color.y = (1 - alpha) * p2.y + alpha * p3.y;
	tmp2_color.z = (1 - alpha) * p2.z + alpha * p3.z;


	res.x = (1 - beta) * tmp1_color.x + beta * tmp2_color.x;
	res.y = (1 - beta) * tmp1_color.y + beta * tmp2_color.y;
	res.z = (1 - beta) * tmp1_color.z + beta * tmp2_color.z;
}

void calcPixelValue(Bitmap& bitmap, Point3d& vec, Point3d& res)
{
	double pixel_width = Image_Width / Resolution_Col;
	double pixel_height = Image_Height / Resolution_Row;

	unsigned char R;
	unsigned char G;
	unsigned char B;

	// calculate interp point of bitmap and ray vector
	// because target ray is parallel of known ray
	// image center is (0, 0) at current view
	Point3d interp(0, 0, 0);
	Point3d P(0, 0, 0);
	Point3d n(0, 0, -Focal_Length);
	Point3d Vp(0, 0, -1);
	Point3d interp_focalplane(0, 0, 0);
	calcInterceptPoint(P, vec, n, Vp, interp);

	if ((interp.x > Image_Width / 2.0) || (interp.x < -Image_Width / 2.0) ||
		(interp.y > Image_Height / 2.0) || (interp.y < -Image_Height / 2.0))
	{
		res.x = 0;
		res.y = 0;
		res.z = 0;
	}
	else
	{
		// convert global coord to img coord
		int c = (Image_Width / 2.0 - interp.x) / pixel_width;
		int r = (interp.y + Image_Height / 2.0) / pixel_height;

		while (r * pixel_height < interp.y + Image_Height / 2.0)
		{
			r++;
		}

		while (Image_Width - c * pixel_width < interp.x + Image_Width / 2.0)
		{
			c--;
		}

		c = (c < 0) ? 0 : c;
		r = (r < 0) ? 1 : r;
		c = (c >= 511) ? 510 : c;
		r = (r >= 511) ? 511 : r;

		double alpha = (Image_Width / 2.0 - c * pixel_width - interp.x) / pixel_width;
		double beta = ((r * pixel_height) - interp.y - Image_Width / 2.0) / pixel_height;

		bitmap.getColor(c, r, R, G, B);
		Point3d c1(R, G, B);
		bitmap.getColor(c + 1, r, R, G, B);
		Point3d c2(R, G, B);
		bitmap.getColor(c, r - 1, R, G, B);
		Point3d c3(R, G, B);
		bitmap.getColor(c + 1, r - 1, R, G, B);
		Point3d c4(R, G, B);

		bilinearInterp(res, c1, c2, c3, c4, alpha, beta);
	}

}
