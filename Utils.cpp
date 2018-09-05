
#include <dlib/clustering.h>
#include <dlib/image_loader/load_image.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_saver/save_jpeg.h>
#include <dlib/opencv.h>

#include <fstream>
#include <iostream>
#include <streambuf>
#include <string>

#include <stdio.h>
#include <time.h>

#include <opencv2/bgsegm.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

#include "jsoncpp/json/json.h"
#include <curl/curl.h>

#include "Define.hpp"

#include "StorageUtils.hpp"
#include "Utils.hpp"
using namespace boost::filesystem;
using namespace dlib;
using namespace std;
using namespace cv;
CURL *curl;
////////////////////////////////////IMPLEMENTATION//////////////////////////////////////////////
int d2;
int d3;

bool Utils::isBlurry(cv::Mat &matImg) {
  cv::Mat lapacianImg, imgGray;
  cv::cvtColor(matImg, imgGray, CV_BGR2GRAY);

  cv::Laplacian(imgGray, lapacianImg, CV_64F);
  cv::Scalar mean, stddev; // 0:1st channel, 1:2nd channel and 2:3rd channel
  cv::meanStdDev(lapacianImg, mean, stddev, cv::Mat());
  double variance = stddev.val[0] * stddev.val[0];

  double threshold = 100;
  std::cout << "blurry :" << variance << endl;
  if (variance <= threshold) {
    return true;
  } else {
    return false;
  }
}
#if 0
void Utils::Drawing_Landmark(
    Mat &image,
    const full_object_detection
        &det) // const std::vector<full_object_detection>& dets)
{
  int thickness = 1;
  const Scalar color = Scalar(0, 255, 0);
  // for (unsigned long i = 0; i < dets.size(); ++i)
  //{
  DLIB_CASSERT(det.num_parts() == 68,
               "\t Invalid inputs were given to this function. "
                   << "\n\t det.num_parts():  " << det.num_parts());

  const full_object_detection &d = det;
  // Around Chin. Ear to Ear
  for (unsigned long i = 1; i <= 16; ++i)
    cv::line(image, Point(d.part(i)(0), d.part(i)(1)),
         Point(d.part(i - 1)(0), d.part(i - 1)(1)), color, thickness);

  //        cv::putText(image, "1", Point(d.part(1)(0),
  //        d.part(1)(1)),cv::FONT_HERSHEY_TRIPLEX, 1.3,
  //        cv::Scalar(255,255,255), 2, 8);
  //        cv::putText(image, "16", Point(d.part(16)(0),
  //        d.part(16)(1)),cv::FONT_HERSHEY_TRIPLEX, 1.3,
  //        cv::Scalar(255,255,255), 2, 8);

  // Line on top of nose
  for (unsigned long i = 28; i <= 30; ++i)
    cv::line(image, Point(d.part(i)(0), d.part(i)(1)),
         Point(d.part(i - 1)(0), d.part(i - 1)(1)), color, thickness);

  // left eyebrow
  for (unsigned long i = 18; i <= 21; ++i)
    line(image, Point(d.part(i)(0), d.part(i)(1)),
         Point(d.part(i - 1)(0), d.part(i - 1)(1)), color, thickness);
  // Right eyebrow
  for (unsigned long i = 23; i <= 26; ++i)
    line(image, Point(d.part(i)(0), d.part(i)(1)),
         Point(d.part(i - 1)(0), d.part(i - 1)(1)), color, thickness);
  // Bottom part of the nose
  for (unsigned long i = 31; i <= 35; ++i)
    line(image, Point(d.part(i)(0), d.part(i)(1)),
         Point(d.part(i - 1)(0), d.part(i - 1)(1)), color, thickness);
  // Line from the nose to the bottom part above
  line(image, Point(d.part(30)(0), d.part(30)(1)),
       Point(d.part(35)(0), d.part(35)(1)), color, thickness);

  // Left eye
  for (unsigned long i = 37; i <= 41; ++i)
    line(image, Point(d.part(i)(0), d.part(i)(1)),
         Point(d.part(i - 1)(0), d.part(i - 1)(1)), color, thickness);
  line(image, Point(d.part(36)(0), d.part(36)(1)),
       Point(d.part(41)(0), d.part(41)(1)), color, thickness);

  // Right eye
  for (unsigned long i = 43; i <= 47; ++i)
    line(image, Point(d.part(i)(0), d.part(i)(1)),
         Point(d.part(i - 1)(0), d.part(i - 1)(1)), color, thickness);
  line(image, Point(d.part(42)(0), d.part(42)(1)),
       Point(d.part(47)(0), d.part(47)(1)), color, thickness);

  // Lips outer part
  for (unsigned long i = 49; i <= 59; ++i)
    line(image, Point(d.part(i)(0), d.part(i)(1)),
         Point(d.part(i - 1)(0), d.part(i - 1)(1)), color, thickness);
  line(image, Point(d.part(48)(0), d.part(48)(1)),
       Point(d.part(59)(0), d.part(59)(1)), color, thickness);

  // Lips inside part
  for (unsigned long i = 61; i <= 67; ++i)
    line(image, Point(d.part(i)(0), d.part(i)(1)),
         Point(d.part(i - 1)(0), d.part(i - 1)(1)), color, thickness);
  line(image, Point(d.part(60)(0), d.part(60)(1)),
       Point(d.part(67)(0), d.part(67)(1)), color, thickness);

  // }
}
}
#endif
void Utils::Drawing_Detection(Mat &image, Rect rect, Scalar color) {
  int x1 = rect.x;
  int y1 = rect.y;
  int x2 = rect.x + rect.width;
  int y2 = rect.y + rect.height;
  int cornerlength = 20;
  // Scalar color = Scalar(0,255,0);
  int thickness = 2;
  cv::line(image, Point(x1, y1), Point(x1 + cornerlength, y1), color,
           thickness);
  cv::line(image, Point(x1, y1), Point(x1, y1 + cornerlength), color,
           thickness);

  cv::line(image, Point(x2, y2), Point(x2, y2 - cornerlength), color,
           thickness);
  cv::line(image, Point(x2, y2), Point(x2 - cornerlength, y2), color,
           thickness);

  cv::line(image, Point(x1, y2), Point(x1 + cornerlength, y2), color,
           thickness);
  cv::line(image, Point(x1, y2), Point(x1, y2 - cornerlength), color,
           thickness);

  cv::line(image, Point(x2, y1), Point(x2 - cornerlength, y1), color,
           thickness);
  cv::line(image, Point(x2, y1), Point(x2, y1 + cornerlength), color,
           thickness);
}
// Make image (mode 1) or bounding region (mode 2)
// brighter 1-step (brighter 1) or dimmer 1-step (brighter 0)
// dimmer 4-step (brighter -1)

// current_state = 0: No person presented
// current_state = 1: person presenting
// current_state = 2: person indentified, welcome screen
// current_state = 3: person indentified, goodbye screen

void Utils::textCentered(cv::InputOutputArray img, const std::string &text,
                         int x1, int _width, int y, int fontFace,
                         double fontScale, cv::Scalar color, int thickness,
                         int lineType, bool bottomLeftOrigin, bool background) {
  int baseline = 0;
  cv::Size _text_size = cv::getTextSize(text, cv::FONT_HERSHEY_TRIPLEX,
                                        fontScale, thickness, &baseline);
  long x = x1 + (_width - _text_size.width) / 2;

  if (background) {
    cv::Mat overlay;
    double alpha = 0.3;

    // copy the source image to an overlay
    img.copyTo(overlay);

    // draw a filled, yellow rectangle on the overlay copy
    cv::rectangle(overlay, cv::Rect(x - d2, y - _text_size.height - d2,
                                    _text_size.width + 2 * d2,
                                    _text_size.height + 2 * d2),
                  cv::Scalar(0, 125, 125), -1);

    // blend the overlay with the source image
    cv::addWeighted(overlay, alpha, img, 1 - alpha, 0, img);
  }

  cv::putText(img, text, cv::Point(x, y), fontFace, fontScale, color, thickness,
              lineType, bottomLeftOrigin);
}
int Utils::sendDataToImgServer(cv::Mat mat, std::string fileName,
                               std::string pose, std::string vec,
                               std::string camid) {
  bool debug = false;
  StorageUtils storageutils;
  storageutils.uploadFile(mat, fileName, camid);
  if (debug) {
    std::cout << "main::test_send_image >> start" << std::endl;
  }
#if 0
    std::vector<unsigned char> imgBuffer;
    cv::imencode(".jpg", mat, imgBuffer);

    // Buffer to String with pointer/length
    unsigned char *imgBuffPtr = &imgBuffer[0];
    long imgBuffLength = imgBuffer.size();

    // CURL SETUP
    // Set headers as null
    struct curl_slist *headers = NULL;
    struct curl_httppost *formpost = NULL;
    struct curl_httppost *lastptr = NULL;

    // Data type
    headers = curl_slist_append(headers, "Content-Type: multipart/form-data");

    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "imgUploader",
                 CURLFORM_BUFFER, fileName.c_str(), CURLFORM_BUFFERPTR,
                 imgBuffPtr, CURLFORM_BUFFERLENGTH, imgBuffLength,
                 CURLFORM_END);
    // face id
    if (debug) {
      std::cout << "main >> test_send_image_and_vector >> pose = " << pose
                << std::endl;
    }

    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "pose",
                 CURLFORM_COPYCONTENTS, pose.c_str(), CURLFORM_END);

    // vector 128 to string
    //  std::stringstream stream;

    //  int K = vec.size();
    //  std::string vec2string = "";
    //  //  stream << K << " ";
    //  for (int i = 0; i < K; ++i) {
    //    //    stream << std::fixed << std::setprecision(6) << vec(i) << " ";
    //    std::ostringstream ss;
    //    ss << vec(i);
    //    std::string s(ss.str());
    //    vec2string = vec2string + s + " ";
    //  }

    //  if (debug) {
    //    std::cout << "main >> test_send_image_and_vector >> stream = "
    //              << stream.str() << std::endl;
    //  }

    //  //  vec2string = stream.str();
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "vector",
                 CURLFORM_COPYCONTENTS, vec.c_str(), CURLFORM_END);

    // init easy_curl
    curl = curl_easy_init();

    if (!curl) {
      return 1;
    }

    std::string url = "http://api.hrl.vp9.vn/api/vupload";
    //  std::string url = "http://27cf7ae0.ngrok.io/api/vupload";
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

    CURLcode res = curl_easy_perform(curl);
    if (debug) {
      std::cout << "sendData >> sent >> " << curl_easy_strerror(res)
                << std::endl;
    }

    curl_easy_cleanup(curl);
    curl_formfree(formpost);
    curl_slist_free_all(headers);
    if (debug) {
      std::cout << "sendData >> finish" << std::endl;
    }
#endif
  return 0;
}

dlib::matrix<dlib::rgb_pixel> Utils::Convert_Mat2Dlib(Mat image) {
  dlib::cv_image<bgr_pixel> cv_temp(image);
  dlib::matrix<dlib::rgb_pixel> spimg;
  dlib::assign_image(spimg, cv_temp);
  return spimg;
}
