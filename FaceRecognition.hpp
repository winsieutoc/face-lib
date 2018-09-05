#ifndef MODEL_H
#define MODEL_H
#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <dlib/data_io.h>
#include <dlib/dnn.h>
#include <dlib/gui_widgets.h>
#include <dlib/image_processing.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/opencv.h>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

#include <X11/Xlib.h>
#include <fstream>
#include <functional>
#include <iostream>
#include <jsoncpp/json/json.h>
#include <map>
#include <queue>
#include <string>
#include <thread>
#include <vector>

#include "CaffeDetector.hpp"
#include <Data.hpp>
#include <Tracker.hpp>
//#include <StorageUtils.hpp>

using namespace dlib;
using namespace cv;
using namespace std;

/**
 * @brief The Detector class
 * Manage all detector (CNN, HOG ,..)
 */

class Detector {
public:
  shape_predictor shapePredictor;

  CaffeDetector *caffe_detector;

public:
  Detector();
  ~Detector();
  /*Get method*/

  shape_predictor getShapePredictor() { return this->shapePredictor; }

  /*Set method*/
  void setShapePredictor(shape_predictor &_shapePredictor) {
    this->shapePredictor = _shapePredictor;
  }

  /*Core functions*/

  std::vector<CaffeDetector::BoundingBox> detectUsingCaffe(cv::Mat &img,
                                                           double scale);
  bool isFace(dlib::matrix<rgb_pixel> &img, dlib::rectangle &det);
};

/**
 * @brief The Classifier class
 * Manage all face datas and find FaceID API
 */

class Classifier {
public:
  // std::vector<std::vector<float>> outputs;
  anet_type net;

  //  TrackerManager *trackerManager;
  string company;
  float netThreshold;
  long numberOfFaceData;
  long sizeOfFaceData;
  // std::vector<DLib128> faceDatas;
  std::map<string, int> uniqueFaceIdList;
  std::map<string, std::map<string, matrix<float, 0, 1>>> faceDatas;
  Detector *detector;
  string placeId;
  // new data object
  std::shared_ptr<PersonData> personDatas;

public:
  Classifier(DETECT_TYPE detectType, string placeId, string _company,
             float _netThreshold);
  Classifier();
  ~Classifier();
  /*Get method*/
  anet_type getNet() { return this->net; }

  float getNetThreshold() { return this->netThreshold; }
  Detector *getDetector() { return this->detector; }

  /*Set method*/

  void setNetThreshold(float _netThreshold) {
    this->netThreshold = _netThreshold;
  }

  /*Core Function*/
  void readFromFolder();
  void readFromDatabase();

  void LoadGalleryFromFile();
  void WriteToFile();

  void addNewProfile(std::string _id, std::string _name, std::string _gender,
                     std::string _age, std::string _url, std::string _vec128);
  void deleteProfile(std::string id);
  std::vector<string> findCloserPerson(matrix<float, 0, 1> _faceDescriptors);
};

/**
 * @brief The FaceRecognition class
 * Manage Classifier object, Track manager and all main function.
 */

class FaceRecognition {
public:
  int max_length_recent_faces_list;

  cv::Rect recognitionROI;
  cv::Mat frame;
  bool isFullHD;
  int frameWidth;
  int frameHeight;
  string camUrl;
  string camId;

  Classifier *classifier;
  TrackerManager *trackerManager;
  std::queue<FrameData> &frame_queue;
  std::mutex &frame_mutex;

public:
  FaceRecognition(string url, string camId, string placeId, std::string company,
                  cv::Rect recognitionROI, bool isFullHD);
  FaceRecognition(cv::Rect recognitionROI, std::queue<FrameData> &frame_queue_,
                  std::mutex &frame_mutex_);
  ~FaceRecognition();

  /*Get method*/
  TrackerManager *getTrackerManager() { return this->trackerManager; }

  Classifier *getClassifier() { return this->classifier; }
  cv::Rect getRecognitionArea() { return this->recognitionROI; }
  int getFrameWidth() { return this->frameWidth; }
  int getFrameHeight() { return this->frameHeight; }
  string getCamUrl() { return this->camUrl; }

  /*Set method*/

  /*Core functions*/

  void UpdateParameter(string _configUrl);

  void FindingFace(cv::Mat &_frame, int countFrame);

  void findFaceOnly(cv::Mat &_frame, int countFrame);

  bool getTrueFaceLocation(cv::Rect inputRect, cv::Rect &outRect);
  void sendToServer(int i);

  void generateShape(cv::Mat &image, cv::Rect currentRec);

  void EncodingFaces(std::vector<matrix<rgb_pixel>> &faceLists);
  void UpdateNewFace(int i, int type);

  void DrawNotification(Mat &frame, string warningText,

                        cv::Scalar color, double fontScale, int thickness,
                        int lineType, int posY);

  void DrawRecognitionArea(cv::Mat &frame);

  /* operator function */
  void operator()();

  void processVideo();
  // send data fpt
  void processTestVideo(bool isShow);

  void
  processWSVideo(std::function<void(std::string jsonData)> SendDataDelegate,
                 bool isShow);

  // canvas
  std::string wrapJsonDataDrawCanvas();
  void
  processWSVideoCanvas(std::function<void(std::string json)> syncInfoForCanvas,
                       bool isShow);

  void updateProfile(std::string _id, std::string _name, std::string _url);
  void updateProfile(std::string _id, std::string _name, std::string _gender,
                     std::string _age, std::string _url, std::string _vec128);
  void updateSetting(cv::Rect roi);
  void clearTrack();
  void bgsProcess();
  std::vector<dlib::matrix<float, 0, 1>>
  get_128D(dlib::matrix<rgb_pixel> &img, std::vector<dlib::rectangle> dets);
};
static double getBlurryValue(cv::Mat &matImg) {
  cv::Mat lapacianImg, imgGray;
  cv::cvtColor(matImg, imgGray, CV_RGB2GRAY);

  cv::Laplacian(imgGray, lapacianImg, CV_64F);
  cv::Scalar mean, stddev; // 0:1st channel, 1:2nd channel and 2:3rd channel
  cv::meanStdDev(lapacianImg, mean, stddev, cv::Mat());
  double variance = stddev.val[0] * stddev.val[0];
  //  std::cout << "blurry :" << variance << endl;
  return variance;
}
#endif
