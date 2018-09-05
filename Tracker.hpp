#ifndef DTRACKER_HPP
#define DTRACKER_HPP

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "jsoncpp/json/json.h"
#include <Define.hpp>

#include <boost/shared_ptr.hpp>
#include <dlib/dnn.h>
#include <dlib/image_processing.h>
#include <dlib/opencv.h>
using namespace dlib;
using namespace std;

/**
 * @brief The SingleTracker class
 * Represent one Face object
 */

class SingleTracker {
public:
  correlation_tracker tracker;
  int trackId;
  int noFaceFoundFrameNumber;
  string trackName;
  std::vector<string> faceIdVec;
  dlib::rectangle rect;
  dlib::point center;
  double trackQuality;
  int noSendDataFrameCount = 0;
  std::vector<std::string> faceImgVec;

  FaceInfo currentFaceInfo;

  bool isNeedDelete;
  int lostTrackCount;

  int maxFaceIdForVoting = 10;
  string tmpName;
  string path;
  string mongo_id;
  int trace;

  bool isUploadFirst;

  std::vector<std::tuple<std::string, std::string, std::string, cv::Mat, double,
                         dlib::matrix<float, 0, 1>>>
      dataVec;
  dlib::matrix<float, 0, 1> previousSentVec128;
  dlib::matrix<float, 128, 1> vec128Sum;

  std::vector<dlib::dpoint> shape5point;
  std::string
      direction_h; // store text left or right or empty for straight face
  std::string direction_v; // store text up or down or empty for straight face
  int angle_h;             // The corresponding angle
  int angle_v;
  std::string pose;

public:
  SingleTracker(dlib::array2d<unsigned char> &_img, dlib::rectangle _rect,
                int _trackId);
  SingleTracker(dlib::array2d<unsigned char> &_img, dlib::rectangle _rect,
                int _trackId, std::vector<dlib::dpoint> shape);
  /*Get method*/
  int getTrackID() { return this->trackId; }

  string getTrackName() { return this->trackName; }

  dlib::rectangle getRect() { return this->rect; }

  dlib::point getCenter() { return this->center; }

  double getTrackQuality() { return this->trackQuality; }

  std::vector<string> &getFaceInfoVec() { return this->faceIdVec; }

  std::vector<std::string> &getFaceImgVec() { return this->faceImgVec; }

  int getNoFaceFoundFrameNumber() { return this->noFaceFoundFrameNumber; }

  std::vector<std::tuple<std::string, std::string, std::string, cv::Mat, double,
                         dlib::matrix<float, 0, 1>>> &
  getDataVec() {
    return this->dataVec;
  }

  /*Set method*/

  void setTrackID(int _trackId) { this->trackId = _trackId; }

  void setTrackName(string _trackName) { this->trackName = _trackName; }

  void setRect(dlib::drectangle _drect) { this->rect = _drect; }

  void setCenter(dlib::drectangle _drect) {
    this->center =
        dlib::point((long)(_drect.tl_corner().x() + (_drect.width() / 2)),
                    (long)(_drect.tl_corner().y() + (_drect.height() / 2)));
  }

  void setTrackQuality(double _trackQuality) {
    this->trackQuality = _trackQuality;
  }

  void setCurrentFaceInfo(FaceInfo _currentFaceInfo) {
    this->currentFaceInfo = _currentFaceInfo;
  }

  void setNoFaceFoundFrameNumber(int _noFaceFoundFrameNumber) {
    this->noFaceFoundFrameNumber = _noFaceFoundFrameNumber;
  }
  /* Core Function */
  void doSingleTrack(dlib::array2d<unsigned char> &_img);
  void doSingleTrackWithNewBox(dlib::array2d<unsigned char> &_img,
                               dlib::rectangle newBox);
  void doSingleTrackWithNewBox(dlib::array2d<unsigned char> &_img,
                               dlib::rectangle newBox,
                               std::vector<dlib::dpoint> shape);

  bool selectFaceId();

  bool isNeedToDeleted();

  bool isDeletedNextFrame();

  bool isMatchWithNewTrack(dlib::rectangle _rect);

  void calculateAngleDirectionLandmarkBased5Points();

  bool isLookingStraight();

  int removeOutlierData();
};

/**
 * @brief The TrackerManager class
 * Manage all single track
 */

class TrackerManager {
public:
  std::function<void(std::string jsonData)> SendDataFunction;
  std::vector<std::shared_ptr<SingleTracker>> trackList;
  long numberOfPeople = 0;
  std::queue<SendData> data_queue;
  int lastTrackId = 0;
  cv::Rect roi;
  cv::Mat frame;
  std::string camId;
  long ts;
  std::string placeId;

public:
  TrackerManager() {}
  TrackerManager(std::string _placeId) { placeId = _placeId; }
  /* Get Function */
  std::vector<std::shared_ptr<SingleTracker>> &getTrackerList() {
    return this->trackList;
  }
  std::queue<SendData> &getDataQueue() { return this->data_queue; }
  int getNumberOfPeople() { return this->numberOfPeople; }

  /* Core Function */

  int findMatchedTracker(dlib::rectangle _rect);

  void insertTracker(dlib::array2d<unsigned char> &_img, dlib::rectangle _rect);
  void insertTracker(dlib::array2d<unsigned char> &_img, dlib::rectangle _rect,
                     std::vector<dlib::dpoint> d);

  void updateTracker(dlib::array2d<unsigned char> &_img);

  int findTracker(int _trackId);

  void deleteTracker(int _trackId);

  std::string wrapJsonData(int index, int indexCover);

  void addSendData(int _index, std::string _camId, std::string status);
  void search_by_vector();
};

#endif // DTRACKER_HPP
