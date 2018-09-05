#ifndef UTILS_HPP
#define UTILS_HPP

#include <dlib/image_processing.h>

#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string>
#include <time.h>

#include <opencv2/bgsegm.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>

#include <boost/filesystem.hpp>

#define LEVEL 256
#define INTENSITY_MAX 255
#define INTENSITY_MIN 0

// test
using namespace boost::filesystem;
using namespace dlib;
using namespace std;
using namespace cv;

extern int d2;
extern int d3;
namespace Utils {

void Drawing_Detection(Mat &image, Rect rect, Scalar color);
void Drawing_ProgressBar(Mat &image, Point pt, int width, int length,
                         int maxlength);
void Drawing_Landmark(
    Mat &image, const full_object_detection
                    &det); // const std::vector<full_object_detection>& dets);
void textCentered(cv::InputOutputArray img, const std::string &text, int x1,
                  int _width, int y, int fontFace, double fontScale,
                  cv::Scalar color, int thickness = 1,
                  int lineType = cv::LINE_8, bool bottomLeftOrigin = false,
                  bool background = false);
dlib::matrix<dlib::rgb_pixel> Convert_Mat2Dlib(Mat image);
bool isBlurry(cv::Mat &matImg);
int sendDataToImgServer(cv::Mat mat, std::string fileName, std::string faceId,
                        std::string vec, std::string camid);
bool isLicenseAlive();
}
#endif // UTILS_HPP
