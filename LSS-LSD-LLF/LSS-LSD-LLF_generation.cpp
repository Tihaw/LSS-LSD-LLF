#include "OpenCVcfg.h"
extern "C"
{
#include "lsd.h"
}

#include <string>

using std::string;

string filename = "../../Exp/Metal2_20160923.tiff";

Mat src;
Point origin;
const int Magnifacation = 9;
cv::Rect showLLF(337, 215, 32, 32);
cv::Rect selection;
double scale_global = 1;
enum Method{
	scale1,
	scale8,
	scale8_sharpenLap,
	scale8_sharpenUSM,
	scale8_histogramEqualization,
	scale8_globalContrast,
};
Method m = scale1;

int lsdDemoMain(void);
void llfFetchShow(const Mat &src, Method m);


string GetMethodName(Method m){
	switch (m)
	{
	case scale1:
		return "scale1";
		break;
	case scale8:
		return "scale8";
		break;
	case scale8_sharpenLap:
		return "scale8_sharpenLap";
		break;
	case scale8_sharpenUSM:
		return "scale8_sharpenUSM";
		break;
	case scale8_histogramEqualization:
		return "scale8_histogramEqualization";
		break;
	case scale8_globalContrast:
		return "scale8_globalContrast";
		break;
	default:
		return "";
		break;
	}
}

static void onMouse(int event, int x, int y, int, void*)
{

	selection.x = MIN(x, origin.x);
	selection.y = MIN(y, origin.y);
	selection.width = std::abs(x - origin.x);
	selection.height = std::abs(y - origin.y);

	//cout << selection.x << " " << selection.y << endl;

	selection &= Rect(0, 0, src.cols, src.rows);

	switch (event)
	{
	case CV_EVENT_LBUTTONDOWN:
		origin = Point(x, y);
		selection = Rect(x, y, 0, 0);
		break;
	case CV_EVENT_LBUTTONUP:
		showLLF = selection;
		break;
	}
}

void main()
{
	//read image
	src = imread(filename, IMREAD_GRAYSCALE);

	//set mouse interface
	namedWindow("image preview", WINDOW_AUTOSIZE);
	setMouseCallback("image preview", onMouse, 0);
	imshow("image preview", src);
	waitKey();

	//prompt
	cout << showLLF.size() << endl;
	cout << "****	begin different pre proc method!" << endl;

	/************************************************************************/
	/* image enhancement //  pre proc*/
	/************************************************************************/
	switch (m)
	{
	case scale1:
		scale_global = 1.0;
		llfFetchShow(src, scale1);
	case scale8:
		scale_global = 0.8;
		llfFetchShow(src, scale8);
	case scale8_sharpenLap:
	{
		scale_global = 0.8;
		Mat sharpenedLap;
		Mat kernel = (Mat_<float>(3, 3) << 0, -1, 0, -1, 5, -1, 0, -1, 0);
		filter2D(src, sharpenedLap, src.depth(), kernel);
		llfFetchShow(sharpenedLap, scale8_sharpenLap);
	}
	case scale8_sharpenUSM:
	{
		scale_global = 0.8;
		// sharpen image using "unsharp mask" algorithm
		Mat blurred;
		double sigma = 1, threshold = 5, amount = 1;
		GaussianBlur(src, blurred, Size(), sigma, sigma);
		Mat lowContrastMask = abs(src - blurred) < threshold;
		Mat sharpened = src*(1 + amount) + blurred*(-amount);
		src.copyTo(sharpened, lowContrastMask);
		llfFetchShow(sharpened, scale8_sharpenUSM);
	}
	case scale8_histogramEqualization:
	{
		scale_global = 0.8; 
		Mat histEq;
		/// Apply Histogram Equalization
		equalizeHist(src, histEq);
		llfFetchShow(histEq, scale8_histogramEqualization);
	}
	default:
		break;
	}

	return;
}

void llfFetchShow(const Mat &src, Method m)
{
	Mat srccpy;
	src.copyTo(srccpy);
	src.convertTo(srccpy, CV_64FC1);

	double * image;
	double * out;
	int n;
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

	printf("Pre proc method : %s \n", GetMethodName(m).c_str());
	printf("%d line segments found:\n", n);
	int count = 0;
	for (int i = 0; i < n; i++)
	{
		Point2d tempX(out[i * 7 + 0], out[i * 7 + 1]);
		Point2d tempY(out[i * 7 + 2], out[i * 7 + 3]);

		if (showLLF.contains(tempX) &&
			showLLF.contains(tempY))
		{
			count++;
		}
	}
	printf("%d lines in the showLLF found:\n\n\n", count);

	//show
	src.convertTo(srccpy, CV_8U);
	cvtColor(srccpy, srccpy, CV_GRAY2BGR);
	for (int i = 0; i < n; i++)
	{
		Point2d tempX(out[i * 7 + 0], out[i * 7 + 1]);
		Point2d tempY(out[i * 7 + 2], out[i * 7 + 3]);

		line(srccpy, tempX, tempY, Scalar(0, 255, 0));
	}

	Mat result = srccpy(showLLF);
	resize(srccpy(showLLF), result,
		Size(showLLF.width*Magnifacation, showLLF.height*Magnifacation));

	imshow(GetMethodName(m).append("lines detected"), result);
	waitKey(0);

	/* free memory */
	free((void *)out);
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

					line(roi, Point2d(x1, y1), Point2d(x2, y2), Scalar(0));
					continue;
				}
			}
		}

		//!!!!gaussian re-sample changed the size
		Rect tempRect(showLLF);
		tempRect.x = static_cast<int>(tempRect.x * Magnifacation*scale_global);
		tempRect.y = static_cast<int>(tempRect.y * Magnifacation*scale_global);
		tempRect.width = static_cast<int>(tempRect.width * Magnifacation*scale_global);
		tempRect.height = static_cast<int>(tempRect.height * Magnifacation*scale_global);

		cv::Mat showAdequateSize = levelLineFieldShow(tempRect);

		imshow("level-line field", showAdequateSize);
		waitKey(10);

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