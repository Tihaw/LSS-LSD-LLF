#include "OpenCVcfg.h"
extern "C"
{
#include "lsd.h"
}

#include <string>

using std::string;

string filename = "../../Exp/Metal2_20160923.tiff";

cv::Rect showLLF(337, 215, 32, 32);

const int Magnifacation = 9;

double scale_global = 1;

int lsdDemoMain(void);


void main()
{
	//lsdDemoMain();

	Mat src = imread(filename, IMREAD_GRAYSCALE);

	/************************************************************************/
	/* image enhancement //  pre proc*/
	/************************************************************************/
	{
		scale_global = 1.0;
	}

	Mat srccpy;
	src.convertTo(srccpy, CV_64FC1);

	double * image;
	double * out;
	int x, y, i, j, n;
	int X = src.cols;  /* x image size */
	int Y = src.rows;  /* y image size */

	/* create a simple image: left half black, right half gray */
	image = (double *)(srccpy.data);
	if (image == NULL)
	{
		fprintf(stderr, "error: not enough memory\n");
		exit(EXIT_FAILURE);
	}

	/* LSD call */
	out = lsd_scale(&n, image, X, Y, scale_global);

	/* print output */
	printf("%d line segments found:\n", n);
	int count = 0;
	for (i = 0; i < n; i++)
	{
		Point tempX(out[i * 7 + 0], out[i * 7 + 1]);
		Point tempY(out[i * 7 + 2], out[i * 7 + 3]);

		if (showLLF.contains(tempX) &&
			showLLF.contains(tempY))
		{
			count++;
		}
	}
	printf("%d lines in the showLLF found:\n", count);

	//show
	cvtColor(src, src, CV_GRAY2BGR);
	for (i = 0; i < n; i++)
	{
		Point tempX(out[i * 7 + 0], out[i * 7 + 1]);
		Point tempY(out[i * 7 + 2], out[i * 7 + 3]);

		line(src, tempX, tempY, Scalar(0, 255, 0));
	}

	Mat result = src(showLLF);
	resize(src(showLLF), result, 
		Size(showLLF.width*Magnifacation, showLLF.height*Magnifacation));

	imshow("detected lines", result);
	waitKey(0);

	/* free memory */
	free((void *)out);

	return;
}

extern "C"
{
	void showLevelLineField(int width, int height, double *angle)
	{
		Mat levelLineFieldShow(height * Magnifacation, 
			width * Magnifacation, CV_8U, Scalar(255));
		
		/*
		//draw mesh
		for (int i = 0; i < height; ++i)
		{
		for (int j = 0; j < width; ++j)
		{
		line(levelLineFieldShow, Point(j*Magnifacation, i*Magnifacation),
		Point(j*Magnifacation + Magnifacation, i*Magnifacation),
		Scalar(0));
		line(levelLineFieldShow, Point(j*Magnifacation, i*Magnifacation),
		Point(j*Magnifacation, i*Magnifacation + Magnifacation),
		Scalar(0));
		}
		}*/

		for (int i = 0; i < height; ++i)
		{
			for (int j = 0; j < width; ++j)
			{
				//image->data[ x + y * image->xsize ]
				//double tempAngle = *(angle + height*i + j);
				double tempAngle = angle[j + i*width];

				if (tempAngle < -800)//this is the -1024 situation 
					continue;

				Mat roi = levelLineFieldShow(
					Rect(j*Magnifacation, i*Magnifacation, Magnifacation, Magnifacation));

				if ((tempAngle < 0.01 && tempAngle > -0.01)
					|| tempAngle > CV_PI - 0.01 || tempAngle < -CV_PI + 0.01)// for case tan(0), tan(+-pi)
				{
					line(roi, Point(0, Magnifacation / 2),
						Point(Magnifacation, Magnifacation / 2), Scalar(0));
					continue;
				}

				//rest
				{
					double x1 = Magnifacation / 2 - Magnifacation / 2 / tan(tempAngle);
					double y1 = 0;
					double x2 = Magnifacation / 2 + Magnifacation / 2 / tan(tempAngle);
					double y2 = Magnifacation;

					line(roi, Point(x1, y1), Point(x2, y2), Scalar(0));
					continue;
				}
			}
		}

		//!!!!gaussian re-sample changed the size
		Rect tempRect(showLLF);
		tempRect.x *= Magnifacation*scale_global;
		tempRect.y *= Magnifacation*scale_global;
		tempRect.width *= Magnifacation*scale_global;
		tempRect.height *= Magnifacation*scale_global;
		cv::Mat showAdequateSize = levelLineFieldShow(tempRect);

		imshow("level-line field", showAdequateSize);
		waitKey();

		//exit(0);
	}
} // extern C


int lsdDemoMain(void)
{
	double * image;
	double * out;
	int x, y, i, j, n;
	int X = 128;  /* x image size */
	int Y = 128;  /* y image size */

	/* create a simple image: left half black, right half gray */
	image = (double *)malloc(X * Y * sizeof(double));
	if (image == NULL)
	{
		fprintf(stderr, "error: not enough memory\n");
		exit(EXIT_FAILURE);
	}
	for (x = 0; x < X; x++)
		for (y = 0; y < Y; y++)
			image[x + y*X] = x < X / 2 ? 0.0 : 64.0; /* image(x,y) */


	/* LSD call */
	out = lsd(&n, image, X, Y);


	/* print output */
	printf("%d line segments found:\n", n);
	for (i = 0; i < n; i++)
	{
		for (j = 0; j < 7; j++)
			printf("%f ", out[7 * i + j]);
		printf("\n");
	}

	/* free memory */
	free((void *)image);
	free((void *)out);

	return EXIT_SUCCESS;
}
