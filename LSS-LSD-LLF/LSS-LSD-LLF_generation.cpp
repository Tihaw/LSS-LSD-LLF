#include "OpenCVcfg.h"
extern "C"
{
#include "lsd.h"
}

#include <string>
#include <sstream>

#define LLF_DEBUG

using std::string;

string filename = "../../Exp/Metal2_20160923.tiff";
stringstream info;

Mat src;
Mat llfImg;
Point origin;
const int Magnifacation = 5;
cv::Rect showLLFRoi(337, 215, 32, 32);
cv::Rect selection;
double scale_global = 1;
enum Method{
	scale1,
	scale8,
	scale8_sharpenLap,
	scale8_sharpenUSM,
	scale8_histogramEqualization,
	scale8_localContrast,
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
		return "8_sharpenLap";
		break;
	case scale8_sharpenUSM:
		return "8_sharpenUSM";
		break;
	case scale8_histogramEqualization:
		return "8_histogramEqualization";
		break;
	case scale8_localContrast:
		return "8_localContrast";
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
		showLLFRoi = selection;
		break;
	}
}

void main()
{
	//read image
	src = imread(filename, IMREAD_GRAYSCALE);

	//set mouse interface
	namedWindow("image preview & select target", WINDOW_AUTOSIZE);
	setMouseCallback("image preview & select target", onMouse, 0);
	imshow("image preview & select target", src);
	waitKey();
	//destroyWindow("image preview & select target");

	//prompt
	cout << showLLFRoi.size() << endl;
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
		// sharpen image using Laplacian mask algorithm
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
	case scale8_localContrast:
	{
		scale_global = 0.8;
		double alpha = 1.7;
		int beta = 0;
		Mat new_image = Mat::zeros(src.size(), src.type());;
		//adjust contrast, especially the DM code area
		for (int y = 0; y < src.rows; y++)
		{
			for (int x = 0; x < src.cols; x++)
			{
				new_image.at<uchar>(y, x) =
					saturate_cast<uchar>(alpha*(src.at<uchar>(y, x)) + beta);
			}
		}

		llfFetchShow(new_image, scale8_localContrast); 
	}
	default:
		break;
	}

	return;
}

void llfFetchShow(const Mat &src, Method m)
{
	Mat srcgray;
	src.copyTo(srcgray);

	srcgray.convertTo(srcgray, CV_64FC1);

	double * image;
	double * out;
	int n;
	int X = src.cols;  /* x image size */
	int Y = src.rows;  /* y image size */

	/* create a simple image: left half black, right half gray */
	image = (double *)(srcgray.data);
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
	//count
	int count = 0;
	for (int i = 0; i < n; i++)
	{
		Point2d tempX(out[i * 7 + 0], out[i * 7 + 1]);
		Point2d tempY(out[i * 7 + 2], out[i * 7 + 3]);

		if (showLLFRoi.contains(tempX) &&
			showLLFRoi.contains(tempY))
		{
			count++;
		}
	}
	printf("%d lines in the showLLF found:\n", count);

	//PPI img
	Mat ppI = src(showLLFRoi);
	resize(src(showLLFRoi), ppI,
		Size(static_cast<int>(showLLFRoi.width*Magnifacation*scale_global),
		static_cast<int>(showLLFRoi.height*Magnifacation*scale_global)));
	cvtColor(ppI, ppI, CV_GRAY2BGR);

	//llf img
	llfImg.convertTo(llfImg, CV_8U);
	cvtColor(llfImg, llfImg, CV_GRAY2BGR);

	//show result
	Mat result(llfImg.size(), CV_8UC3);
	for (int i = 0; i < n; i++)
	{
		Point2d tempX(out[i * 7 + 0] * Magnifacation * scale_global, 
					  out[i * 7 + 1] * Magnifacation * scale_global);
		Point2d tempY(out[i * 7 + 2] * Magnifacation * scale_global, 
					  out[i * 7 + 3] * Magnifacation * scale_global);
		line(result, tempX, tempY, Scalar(0, 255, 0), Magnifacation);
	}
	addWeighted(result, 0.5, llfImg, 0.5, 0, result);

	// re select roi for llf and result
	Rect magnifiedRoi;
	magnifiedRoi.x = static_cast<int>(showLLFRoi.x * Magnifacation*scale_global);
	magnifiedRoi.y = static_cast<int>(showLLFRoi.y * Magnifacation*scale_global);
	magnifiedRoi.width = static_cast<int>(showLLFRoi.width * Magnifacation*scale_global);
	magnifiedRoi.height = static_cast<int>(showLLFRoi.height * Magnifacation*scale_global);
	cv::Mat llfShow = llfImg(magnifiedRoi);
	cv::Mat resultShow = result(magnifiedRoi);
	
	// mix three img into one 4 show
	Mat mixedResult = Mat::zeros(resultShow.rows * 3, resultShow.cols, CV_8UC3);
	ppI.copyTo(mixedResult(Range(0, ppI.rows), Range(0, ppI.cols)));
	llfShow.copyTo(mixedResult(Range(llfShow.rows, llfShow.rows * 2), Range(0, llfShow.cols)));
	resultShow.copyTo(mixedResult(Range(resultShow.rows * 2, resultShow.rows * 3), Range(0, resultShow.cols)));

	//show with info
	info.str("");
	info << "m: " << GetMethodName(m);
	putText(mixedResult, info.str(),
		Point(0, 10), FONT_HERSHEY_PLAIN, 1.0, Scalar(0, 0, 255), 1);

	info.str("");
	info << "r: " << count << "/" << n;
	putText(mixedResult, info.str(),
		Point(0, 20), FONT_HERSHEY_PLAIN, 1.0, Scalar(0, 0, 255), 1);

#ifdef LLF_DEBUG
	imshow(GetMethodName(m).append("_LSD_result"), mixedResult);
	printf("press 'n' to proc next img \n\n\n");

	while (char c = waitKey())
	{
	if (c == 'n')
	{
	break;
	}
	}
#endif

#ifndef LLF_DEBUG
	imwrite(GetMethodName(m).append(".tiff"), mixedResult);
#endif


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
		
/*
		//!!!!gaussian re-sample changed the size
		Rect magnifiedRoi;
		magnifiedRoi.x = static_cast<int>(showLLFRoi.x * Magnifacation*scale_global);
		magnifiedRoi.y = static_cast<int>(showLLFRoi.y * Magnifacation*scale_global);
		magnifiedRoi.width = static_cast<int>(showLLFRoi.width * Magnifacation*scale_global);
		magnifiedRoi.height = static_cast<int>(showLLFRoi.height * Magnifacation*scale_global);

		cv::Mat showAdequateSize = levelLineFieldShow(magnifiedRoi);*/

		levelLineFieldShow.copyTo(llfImg);
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