#include "bmp.h"		//	Simple .bmp library
#include<iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>


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

bool withinRange(int val, int minVal, int maxVal) {
	return val >= minVal && val <= maxVal;
}

int myFloor(double x) {
	int i = (int)x;  // integer part of x
	if (x >= 0 || x == i) {
		return i;
	}
	else {
		return i - 1;
	}
}



int main(int argc, char** argv)
{
	if (argc < 5 || argc > 6)
	{
		cout << "Arguments prompt: viewSynthesis.exe <LF_dir> <X Y Z> OR: viewSynthesis.exe <LF_dir> <X Y Z> <focal_length>" << endl;
		return 0;
	}
	string LFDir = argv[1];
	double Vx = stod(argv[2]), Vy = stod(argv[3]), Vz = stod(argv[4]);
	double targetFocalLen = 100; // default focal length for "basic requirement" part
	if (argc == 6)
	{
		targetFocalLen = stod(argv[5]);
	}


	vector<Bitmap> viewImageList;
	//! loading light field views
	for (int i = 0; i < View_Grid_Col * View_Grid_Row; i++)
	{
		char name[128];
		sprintf(name, "/cam%03d.bmp", i);
		string filePath = LFDir + name;
		Bitmap view_i(filePath.c_str());
		viewImageList.push_back(view_i);
	}

	Bitmap targetView(Resolution_Col, Resolution_Row);
	cout << "Synthesizing image from viewpoint: (" << Vx << "," << Vy << "," << Vz << ") with focal length: " << targetFocalLen << endl;
	//! resample pixels of the target view one by one

	Color  ray1,ray2,ray3,ray4;
	double x, y, alpha, beta;
	int u1, u2, v1, v2;
	int colu1v1, colu1v2, colu2v1, colu2v2, rowu1v1, rowu1v2, rowu2v1, rowu2v2;
	unsigned char bindR1, bindG1, bindB1;
	unsigned char bindR2, bindG2, bindB2;
	double disx, disy;
	for (int r = 0; r < Resolution_Row; r++)
	{
		for (int c = 0; c < Resolution_Col; c++)
		{
			Point3d rayRGB(0, 0, 0);
			// Image resolution:512x512
			// Image size : 35x35
			//subtracting half no. in the image
			double half = (512.0 - 1.0) / 2.0;
			//ratio
			double ratio= 35.0 /512.0;
			//displacement of a ray from a new viewpoint to pixel
			double disx = (((c - half) * ratio) / targetFocalLen);
			double disy = (((r - half) * ratio) / targetFocalLen);

			//printf("%f %f\n", disx,disy);
			// coordinates of projection on viewplane 3D Vx Vy Vz to a pixel (c, r)
			x = Vx + Vz*disx;
			y = Vy + Vz*disy; 
			//printf("%f %f\n", x,y);


			//in range
			if ((x >= -120.0) && (x <= 120.0) && (y >= -120.0) && (y <= 120.0)) {
				//Resample the value of a ray of the target view
				double planex = (120.0 + x);
				double planey = (120.0 + y);
				//u1 = (int)floor(planex / Baseline); //ui
				u1 = (int)floor(planex / Baseline); //ui
				u2 = (int)ceil(planex / Baseline); // ui+1
				v1 = (int)floor((120.0 - y) / Baseline); // view point vi
				v2 = (int)ceil((120.0 - y) / Baseline); // view point vi+1

				alpha = (x - (-120.0 + u1 * Baseline)) / Baseline; // alpha
				beta = ((- y+120.0 - v1 * Baseline) ) / Baseline; // beta

				double u1x = (-120.0 + u1 * Baseline) + Focal_Length * disx;
				double u2x = (-120.0 + u2 * Baseline) + Focal_Length * disx;
				double v1y = (120.0 - v1 * Baseline) + Focal_Length * disy;
				double v2y = (120.0 - v2 * Baseline) + Focal_Length * disy;
				// Interpolate the value of a ray from a known viewpoint
				Point2d u1v1(u1x, v1y); 
				Point2d u1v2(u1x, v2y); 
				Point2d u2v1(u2x, v1y);
				Point2d u2v2(u2x, v2y);
				//pixel column
				colu1v1 = round((u1v1.x + (120.0 - u1 * Baseline))/ ratio + (512.0 - 1.0)/ 2.0);
				colu1v2 = round((u1v2.x + (120.0 - u1 * Baseline))/ ratio + (512.0 - 1.0)/ 2.0); 
				colu2v1 = round((u2v1.x + (120.0 - u2 * Baseline))/ ratio + (512.0 - 1.0)/ 2.0); 
				colu2v2 = round((u2v2.x + (120.0 - u2 * Baseline))/ ratio + (512.0 - 1.0)/ 2.0); 
				//pixel row				
				rowu1v1 = round((u1v1.y - (120.0 - v1 * Baseline))/ ratio + (512.0 - 1.0)/ 2.0); 
				rowu1v2 = round((u1v2.y - (120.0 - v2 * Baseline))/ ratio + (512.0 - 1.0)/ 2.0); 
				rowu2v1 = round((u2v1.y - (120.0 - v1 * Baseline))/ ratio + (512.0 - 1.0)/ 2.0); 
				rowu2v2 = round((u2v2.y - (120.0 - v2 * Baseline))/ ratio + (512.0 - 1.0)/ 2.0); 
				//in range pixel
				if (withinRange(colu1v1, 0, Resolution_Col - 1) && withinRange(rowu1v1, 0, Resolution_Row - 1)
					&& withinRange(colu1v2, 0, Resolution_Col - 1) && withinRange(rowu1v2, 0, Resolution_Row - 1)
					&& withinRange(colu2v1, 0, Resolution_Col - 1) && withinRange(rowu2v1, 0, Resolution_Row - 1)
					&& withinRange(colu2v2, 0, Resolution_Col - 1) && withinRange(rowu2v2, 0, Resolution_Row - 1)){
				
					viewImageList[(u1 + v1 * 9)].getColor(colu1v1, rowu1v1, ray1.R, ray1.G, ray1.B); //u1 v1
					viewImageList[(u2 + v1 * 9)].getColor(colu1v2, rowu1v2, ray2.R, ray2.G, ray2.B); //u2 v1
					viewImageList[(u1 + v2 * 9)].getColor(colu2v1, rowu2v1, ray3.R, ray3.G, ray3.B); //u1 v2
					viewImageList[(u2 + v2 * 9)].getColor(colu2v2, rowu2v2, ray4.R, ray4.G, ray4.B); //u2 v2
					//Bilinear Interpolation
					//pvi
					bindR1 = ray1.R * (1 - alpha) + ray2.R * alpha;
					bindG1 = ray1.G * (1 - alpha) + ray2.G * alpha;
					bindB1 = ray1.B * (1 - alpha) + ray2.B * alpha;
					//pvi+1
					bindR2 = ray3.R * (1 - alpha) + ray4.R * alpha;
					bindG2 = ray3.G * (1 - alpha) + ray4.G * alpha;
					bindB2 = ray3.B * (1 - alpha) + ray4.B * alpha;
					//ptarget
					rayRGB.x = bindR1 * (1 - beta) + bindR2 * (beta);
					rayRGB.y = bindG1 * (1 - beta) + bindG2 * (beta);
					rayRGB.z = bindB1 * (1 - beta) + bindB2 * (beta);

					
				}

				//! record the resampled pixel value

			}
			targetView.setColor(c, r, (unsigned char)rayRGB.x, (unsigned char)rayRGB.y, (unsigned char)rayRGB.z);
		}
	}


	string savePath = "newView.bmp";
	targetView.save(savePath.c_str());
	cout << "Result saved!" << endl;
	return 0;
}