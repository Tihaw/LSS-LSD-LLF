#pragma once

#ifndef	 _OPENCV_cfg_
#define _OPENCV_cfg_

#include <opencv.hpp>
#include <opencv2/ml/ml.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv_modules.hpp>
#include <opencv2/flann/flann.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include "io.h"
#include <iostream>
#include <fstream>  
#include <string>  
#include <vector>  
#include <Windows.h>
#include <time.h>

#ifdef _DEBUG
#define lnkLIB(name) name "d"
#else
#define lnkLIB(name) name
#endif
#define CV_VERSION_ID CVAUX_STR(CV_MAJOR_VERSION) CVAUX_STR(CV_MINOR_VERSION) CVAUX_STR(CV_SUBMINOR_VERSION)
#define cvLIB(name) lnkLIB("opencv_" name CV_VERSION_ID)

/**************opencv libs****************/
#pragma comment( lib, cvLIB("core"))
#pragma comment( lib, cvLIB("imgproc"))
#pragma comment( lib, cvLIB("highgui"))
#pragma comment(lib, cvLIB("contrib"))

#pragma comment( lib, cvLIB("calib3d"))
#pragma comment( lib, cvLIB("features2d"))
#pragma comment(lib, cvLIB("flann"))
#pragma comment( lib, cvLIB("gpu"))

#pragma comment( lib, cvLIB("legacy"))
#pragma comment( lib, cvLIB("objdetect"))
#pragma comment(lib, cvLIB("ts"))
#pragma comment( lib, cvLIB("video"))

#pragma comment( lib, cvLIB("nonfree"))
#pragma comment( lib, cvLIB("ocl"))
#pragma comment(lib, cvLIB("photo"))
#pragma comment( lib, cvLIB("stitching"))

#pragma comment( lib, cvLIB("superres"))
#pragma comment( lib, cvLIB("videostab"))
#pragma comment(lib, cvLIB("ml"))

using namespace cv;  
using namespace std;  

#endif