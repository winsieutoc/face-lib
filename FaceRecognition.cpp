#include "FaceRecognition.hpp"

#include "Configuration.hpp"
#include "SocketIoUtils.hpp"
#include "Utils.hpp"
//#include "wscapture.h"
#include "base64.h"
#include <curl/curl.h>
logger dlogf("FaceCNN.FaceRecognition");

///***@brief FaceRecognition::FaceRecognition Constructor -

// FaceRecognition::FaceRecognition(string url, string camId, string placeId,
//                                 std::string company, cv::Rect recognitionROI,
//                                 bool isFullHD) {
//  this->camUrl = url;
//  this->camId = camId;
//  this->recognitionROI = recognitionROI;

//  this->trackerManager = new TrackerManager(placeId);
//  string folderPath = "Gallery_demo/";
//  this->classifier = new Classifier(DETECT_TYPE::CAFFE, placeId, company,
//  0.35);
//  this->trackerManager->roi = this->recognitionROI;
//  this->trackerManager->camId = this->camId;
//  if (isFullHD) {
//    this->frameWidth = 1920;
//    this->frameHeight = 1080;
//  } else {
//    this->frameWidth = 1280;
//    this->frameHeight = 720;
//  }
//}
FaceRecognition::FaceRecognition(cv::Rect recognitionROI,
                                 std::queue<FrameData> &frame_queue_,
                                 std::mutex &frame_mutex_)
    : frame_queue(frame_queue_), frame_mutex(frame_mutex_) {

  this->recognitionROI = recognitionROI;

  this->trackerManager = new TrackerManager();
  this->classifier = new Classifier();
  this->trackerManager->roi = this->recognitionROI;
  this->isFullHD = true;
  if (isFullHD) {
    this->frameWidth = 1920;
    this->frameHeight = 1080;
  } else {
    this->frameWidth = 1280;
    this->frameHeight = 720;
  }
}

FaceRecognition::~FaceRecognition() {
  delete trackerManager;
  delete classifier;
}
/**
 * @brief FaceRecognition::UpdateParameter Update para
 * @param _configUrl
 */

// bool FaceRecognition::getTrueFaceLocation(cv::Rect inputRect,
//                                          cv::Rect &outRect) {
//  //  cout << "getTrueFaceLocation" << endl;
//  //  cout << inputRect << endl;
//  //  cout << outRect << endl;
//  //    cv::Rect expandRec()
//  cv::Mat tmpImg = this->frame(inputRect);
//  matrix<rgb_pixel> img = Utils::Convert_Mat2Dlib(tmpImg);
//  std::vector<dlib::rectangle> dets =
//      this->getClassifier()->getDetector()->getCascadeDetector()(img);

//  if (dets.size() == 0) {
//    return false;
//  } else {
//    outRect =
//        cv::Rect(inputRect.x + dets[0].left(), inputRect.y + dets[0].top(),
//                 inputRect.width, inputRect.height);
//    //    cout << outRect << endl;
//    return true;
//  }
//}
std::vector<dlib::matrix<float, 0, 1>>
FaceRecognition::get_128D(dlib::matrix<rgb_pixel> &img,
                          std::vector<dlib::rectangle> dets) {
  std::vector<dlib::matrix<rgb_pixel>> img_list;
  for (int i = 0; i < dets.size(); ++i) {
    auto shape = this->classifier->getDetector()->shapePredictor(img, dets[i]);
    matrix<rgb_pixel> faceChip;

    extract_image_chip(img, get_face_chip_details(shape, 150, 0.25), faceChip);
    img_list.push_back(faceChip);
  }
  std::vector<dlib::matrix<float, 0, 1>> discriptor =
      this->classifier->getNet()(img_list);
  return discriptor;
}
/**
 * @brief FaceRecognition::findFaceOnly
 * @param _frame
 * @param countFrame
 */
void FaceRecognition::findFaceOnly(cv::Mat &_frame, int countFrame) {
  bool debug = true;

  if (debug)
    cout << countFrame << endl;
  double scale = 1;
  /*Convert Mat image to dlib img*/

  cv::Mat cvImg = _frame.clone();
  dlib::cv_image<bgr_pixel> cv_temp(cvImg);
  matrix<rgb_pixel> img;
  dlib::assign_image(img, cv_temp);
  dlib::array2d<unsigned char> imgGray;
  dlib::assign_image(imgGray, img);

  //  this->frame = _frame.clone();
  //  cv::imshow("test1", cvImg);

  std::vector<dlib::matrix<rgb_pixel>> imgList;
  double scaleMtcnn = 2;
  if (countFrame % 4 == 0) {
    std::vector<CaffeDetector::BoundingBox> dets =
        this->classifier->detector->detectUsingCaffe(cvImg, scaleMtcnn);
    for (CaffeDetector::BoundingBox d : dets) {
      dlib::rectangle det(d.x1 * scaleMtcnn - 5, d.y1 * scaleMtcnn - 10,
                          d.x2 * scaleMtcnn + 5, d.y2 * scaleMtcnn + 10);
      //      dlib_recs.push_back(det);
      trackerManager->insertTracker(imgGray, det);
    }
  } else {
    trackerManager->updateTracker(imgGray);
  }
#if 0
  if (countFrame % 4 == 0) {

    // auto dets = this->getRecognition()->getDetector()->cnnDetector(img);
    double scaleMtcnn = 3;
    std::vector<CaffeDetector::BoundingBox> dets =
        this->classifier->getDetector()->detectUsingCaffe(cvImg, scaleMtcnn);
    // std::vector<dlib::rectangle> dets = detector(img_gray);
    if (debug)
      cout << "Faces size : " << dets.size() << endl;
    for (CaffeDetector::BoundingBox d : dets) {
      dlib::rectangle det(d.x1 * scaleMtcnn - 5, d.y1 * scaleMtcnn - 10,
                          d.x2 * scaleMtcnn + 5, d.y2 * scaleMtcnn + 10);
      if (det.width() >= 100 && det.height() >= 100) {
        this->trackerManager->insertTracker(imgGray, det);
      }
    }
  } else {
    this->trackerManager->updateTracker(imgGray);
  }

  // get info
  if (this->trackerManager->getTrackerList().size() > 0) {
    if (debug)
      cout << "FaceRecognition::findFaceOnly 1" << endl;
    for (int i = 0; i < this->trackerManager->getTrackerList().size(); ++i) {
      if (debug)
        cout << "FaceRecognition::findFaceOnly 1.1" << endl;
      // Generate info

      dlib::rectangle det =
          this->trackerManager->getTrackerList()[i]->getRect();
      cv::Rect rec((long)(det.left() + this->getRecognitionArea().x),
                   (long)(det.top() + this->getRecognitionArea().y),
                   (long)(det.width()), (long)(det.height()));

      auto shape = this->classifier->getDetector()->shapePredictor(img, det);
      matrix<rgb_pixel> faceChip;

      extract_image_chip(img, get_face_chip_details(shape, 150, 0.25),
                         faceChip);

      FaceInfo currentFaceInfo;
      currentFaceInfo.faceId = "";
      currentFaceInfo.rect = rec;
      currentFaceInfo.faceImg = move(faceChip);
      currentFaceInfo.shape = shape;

      this->trackerManager->getTrackerList()[i]->currentFaceInfo =
          currentFaceInfo;

      imgList.push_back(
          this->trackerManager->getTrackerList()[i]->currentFaceInfo.faceImg);
    }
  }

  if (imgList.size() > 0) {
    std::vector<dlib::matrix<float, 0, 1>> vecList =
        this->classifier->getNet()(imgList);
    for (int i = 0; i < this->trackerManager->getTrackerList().size(); ++i) {
      this->trackerManager->getTrackerList()[i]->currentFaceInfo.discriptor =
          vecList[i];
    }
    if (countFrame % 60 == 0) {
      this->trackerManager->search_by_vector();
    }
  }
#endif
}

/**
 * @brief FaceRecognition::UpdateNewFace
 * @param i
 */
void FaceRecognition::sendToServer(int i) {

  bool debug = false;
  if (this->trackerManager->getTrackerList()[i]->pose.empty())
    return;
  if (this->trackerManager->getTrackerList()[i]
          ->currentFaceInfo.faceImg.size() == 0)
    return;
  if (this->trackerManager->getTrackerList()[i]
          ->currentFaceInfo.shape.num_parts() != 68)
    return;

  // Create new Id
  string str;

  time_t now = time(0);
  if (this->trackerManager->getTrackerList()[i]->tmpName.empty()) {
    static char autoFaceID[30];
    strftime(autoFaceID, sizeof(autoFaceID), "Customer-%d%H%M%S",
             localtime(&now));
    this->getTrackerManager()->getTrackerList()[i]->tmpName = autoFaceID;
  }
  //  boost::filesystem::create_directory(this->getClassifier()->getFolderPath()
  //  +
  //                                      faceTrack->tmpName);

  //  string filename =
  //      "straight" +ue
  //      std::to_string(this->trackerManager->getTrackerList()[i]
  //                                      ->getNoFaceFoundFrameNumber());
  //  cout << "trace : " << this->trackerManager->getTrackerList()[i]->trace
  //       << endl;
  string filename =
      this->trackerManager->getTrackerList()[i]->pose +
      std::to_string(this->trackerManager->getTrackerList()[i]->trace);

  str = this->trackerManager->getTrackerList()[i]->tmpName + "-" + filename +
        ".jpg";

  this->trackerManager->getTrackerList()[i]->noSendDataFrameCount++;
  if (debug) {
    cout << "no Frame :"
         << this->trackerManager->getTrackerList()[i]->noSendDataFrameCount
         << endl;
  }
  if (this->trackerManager->getTrackerList()[i]->noSendDataFrameCount % 5 != 1)
    return;

  if (this->trackerManager->getTrackerList()[i]->previousSentVec128.nr() != 128)
    this->trackerManager->getTrackerList()[i]->previousSentVec128 =
        this->trackerManager->getTrackerList()[i]->currentFaceInfo.discriptor;
  else {
    if (dlib::length(
            this->trackerManager->getTrackerList()[i]->previousSentVec128 -
            this->trackerManager->getTrackerList()[i]
                ->currentFaceInfo.discriptor) < 0.2)
      return;
    this->trackerManager->getTrackerList()[i]->previousSentVec128 =
        this->trackerManager->getTrackerList()[i]->currentFaceInfo.discriptor;
  }

  //  cv::Mat img = dlib::toMat(
  //      this->trackerManager->getTrackerList()[i]->currentFaceInfo.faceImg);
  //  cv::cvtColor(img, img, CV_BGR2RGB);
  //  if (this->trackerManager->getTrackerList()[i]
  //          ->currentFaceInfo.shape.num_parts() != 68)
  //    return;
  cv::Rect imgCropRect(
      this->trackerManager->getTrackerList()[i]->currentFaceInfo.rect.x -
          this->trackerManager->getTrackerList()[i]
                  ->currentFaceInfo.rect.width *
              0.2,
      this->trackerManager->getTrackerList()[i]->currentFaceInfo.rect.y -
          this->trackerManager->getTrackerList()[i]
                  ->currentFaceInfo.rect.width *
              0.2,
      this->trackerManager->getTrackerList()[i]->currentFaceInfo.rect.width *
          1.4,
      this->trackerManager->getTrackerList()[i]->currentFaceInfo.rect.width *
          2.5);

  auto isRectInside = [](cv::Rect rec) -> bool {
    if (rec.x >= 0 && rec.y >= 0 && rec.br().x <= 1920 && rec.br().y <= 1080)
      return true;
    return false;
  }(imgCropRect);

  //  cout << "test : " << isRectInside << endl;
  if (!isRectInside)
    return;
  cv::Mat img = this->trackerManager->frame(imgCropRect);

  cv::resize(img, img, cv::Size(210, (int)(210 * img.rows / img.cols)));

  double blurry = getBlurryValue(img);
  if (debug) {
    cout << this->trackerManager->getTrackerList()[i]->trace << endl;
    cout << "test : "
         << this->trackerManager->getTrackerList()[i]->isUploadFirst << endl;
  }
  if (this->trackerManager->getTrackerList()[i]->isUploadFirst) {
    if (!this->trackerManager->getTrackerList()[i]->isLookingStraight())
      return;
  } else {
    this->trackerManager->getTrackerList()[i]->isUploadFirst = true;
  }
  /* Save image */
  if (debug)
    cout << "Save image : " << str << endl;

  int K = this->trackerManager->getTrackerList()[i]
              ->currentFaceInfo.discriptor.size();
  std::string vec2string = "";
  //  stream << K << " ";
  for (int j = 0; j < K; ++j) {
    //    stream << std::fixed << std::setprecision(6) << vec(i) << " ";
    std::ostringstream ss;
    ss << this->trackerManager->getTrackerList()[i]->currentFaceInfo.discriptor(
        j);
    std::string s(ss.str());
    vec2string = vec2string + s + " ";
  }
  if (vec2string.empty())
    return;

  for (int k = 0; k < 128; ++k) {
    this->trackerManager->getTrackerList()[i]->vec128Sum(k) +=
        this->trackerManager->getTrackerList()[i]->currentFaceInfo.discriptor(
            k);
    //    cout << this->trackerManager->getTrackerList()[i]
    //                ->currentFaceInfo.discriptor(k)
    //         << " ";
  }
  //  cout << endl;
  //  cout << "+++-------------------------------" << endl;
  //  cout << vec2string << endl;
  //  cout << "xxx-------------------------------" << endl;

  if (this->trackerManager->getTrackerList()[i]->getDataVec().size() <= 20)
    this->trackerManager->getTrackerList()[i]->getDataVec().push_back(
        std::make_tuple(str, vec2string,
                        this->trackerManager->getTrackerList()[i]->pose, img,
                        blurry, this->trackerManager->getTrackerList()[i]
                                    ->currentFaceInfo.discriptor));
}

/**
 * @brief FaceRecognition::FindingFace Find bounding box of Face on frame
 * @param _frame
 * @param countFrame
 */

void FaceRecognition::FindingFace(cv::Mat &_frame, int countFrame) {
  dlogf << LDEBUG << "[START] FaceRecognition::FindingFace ";
  bool debug = false;
  if (debug)
    cout << countFrame << endl;
  double scale = 1;
  std::vector<matrix<rgb_pixel>> faceList;
  /*Convert Mat image to dlib img*/
  cv::Mat cvImg = _frame(this->getRecognitionArea());
  //  cv::imshow("test1", cvImg);
  matrix<rgb_pixel> img = Utils::Convert_Mat2Dlib(cvImg);
  dlib::array2d<unsigned char> imgGray;
  dlib::assign_image(imgGray, img);

  this->trackerManager->updateTracker(imgGray);

  // get info
  if (this->trackerManager->getTrackerList().size() > 0) {
    for (int i = 0; i < this->trackerManager->getTrackerList().size(); ++i) {
      // Generate info
      if (debug)
        cout << "Quality :"
             << this->trackerManager->getTrackerList()[i]->getTrackQuality()
             << endl;
      dlib::rectangle det =
          this->trackerManager->getTrackerList().at(i)->getRect();
      cv::Rect rec((long)(det.left() + this->getRecognitionArea().x),
                   (long)(det.top() + this->getRecognitionArea().y),
                   (long)(det.width()), (long)(det.height()));

      auto shape = this->classifier->getDetector()->shapePredictor(img, det);
      matrix<rgb_pixel> faceChip;

      extract_image_chip(img, get_face_chip_details(shape, 150, 0.25),
                         faceChip);

      FaceInfo currentFaceInfo;
      currentFaceInfo.faceId = "";
      currentFaceInfo.rect = rec;
      currentFaceInfo.faceImg = move(faceChip);
      currentFaceInfo.shape = shape;

      this->trackerManager->getTrackerList()[i]->currentFaceInfo =
          currentFaceInfo;

      if (this->trackerManager->getTrackerList()[i]->getTrackName().empty() &&
          currentFaceInfo.shape.num_parts() == 68) {
        faceList.push_back(currentFaceInfo.faceImg);
      }
    }
  }

  //  if (countFrame % 10 == 0) {
  //    // auto dets =
  //    this->getRecognition()->getDetector()->cnnDetector(img);
  //    std::vector<dlib::rectangle> dets =
  //        this->classifier->getDetector()->detectUsingCaffe(cvImg, 3);
  //    // std::vector<dlib::rectangle> dets = detector(img_gray);
  //    //    cout << "Faces size : " << dets.size() << endl;
  //    for (dlib::rectangle d : dets) {
  //      dlib::rectangle drec(d.left() / scale - 5, d.top() / scale - 10,
  //                           d.right() / scale + 5, d.bottom() / scale +
  //                           10);
  //      this->trackerManager->insertTracker(imgGray, drec);
  //    }
  //  }
  if (countFrame % 4 == 0) {

    // auto dets = this->getRecognition()->getDetector()->cnnDetector(img);
    double scaleMtcnn = 3;
    std::vector<CaffeDetector::BoundingBox> dets =
        this->classifier->getDetector()->detectUsingCaffe(cvImg, scaleMtcnn);
    // std::vector<dlib::rectangle> dets = detector(img_gray);
    //    cout << "Faces size : " << dets.size() << endl;
    for (CaffeDetector::BoundingBox d : dets) {
      dlib::rectangle det(d.x1 * scaleMtcnn - 5, d.y1 * scaleMtcnn - 10,
                          d.x2 * scaleMtcnn + 5, d.y2 * scaleMtcnn + 10);
      std::vector<dlib::dpoint> shape5P;
      for (int l = 0; l < 5; ++l) {
        shape5P.push_back(dlib::dpoint(d.points_x[l] * scaleMtcnn,
                                       d.points_y[l] * scaleMtcnn));
      }
      this->trackerManager->insertTracker(imgGray, det, shape5P);
    }
  }
  // Encode Face

  //  if (faceList.size() > 0)
  //    this->EncodingFaces(faceList);
}

/**
 * @brief FaceRecognition::EncodingFaces Convert face image to 128 dimension
 * vector
 * @param faceLists
 */

void FaceRecognition::EncodingFaces(std::vector<matrix<rgb_pixel>> &faceLists) {
  dlogf << LDEBUG << "[START] FaceRecognition::EncodingFaces ";
  bool debug = false;

  std::vector<matrix<float, 0, 1>> faceDiscriptorList =
      this->classifier->getNet()(faceLists);

  int index = 0;
  if (debug)
    cout << "Track size : " << this->trackerManager->getTrackerList().size()
         << endl;
  for (size_t i = 0; i < this->trackerManager->getTrackerList().size(); i++) {
    if (debug)
      cout << "noFaceFoundFrameNumber : "
           << this->trackerManager->getTrackerList()[i]
                  ->getNoFaceFoundFrameNumber()
           << endl;
    if (this->trackerManager->getTrackerList()[i]->getTrackName().empty() &&
        this->trackerManager->getTrackerList()[i]
                ->currentFaceInfo.shape.num_parts() == 68) {

      this->trackerManager->getTrackerList()[i]->currentFaceInfo.discriptor =
          faceDiscriptorList[index];

      if (!this->trackerManager->getTrackerList()[i]
               ->currentFaceInfo.Is_Looking_Straight()) {
        //      cout << "Khong phai mat thang" << endl;
        continue;
      }

      // 17 frame cann't found id so app decide this face is new
      // Set new customer here
      if (this->trackerManager->getTrackerList()[i]
                  ->getNoFaceFoundFrameNumber() >= 15 &&
          this->trackerManager->getTrackerList()[i]
                  ->getNoFaceFoundFrameNumber() <= 70 &&
          this->trackerManager->getTrackerList()[i]->getFaceInfoVec().empty()) {

        this->UpdateNewFace(i, 0);
        this->trackerManager->getTrackerList()[i]->noFaceFoundFrameNumber++;

        return;
      }

      //      if (!this->trackerManager->getTrackerList()[i]
      //               ->currentFaceInfo.Is_Looking_Straight()) {
      //        //      cout << "Khong phai mat thang" << endl;
      //        continue;
      //      }

      std::vector<string> tmpFaceIdVec = this->classifier->findCloserPerson(
          this->trackerManager->getTrackerList()[i]
              ->currentFaceInfo.discriptor);

      if (tmpFaceIdVec.at(0).empty()) {
        //        currentTrack->currentFaceInfo.faceId = "Unknown";
        this->trackerManager->getTrackerList()[i]->noFaceFoundFrameNumber++;

      } else {
        this->trackerManager->getTrackerList()[i]->noFaceFoundFrameNumber = 0;
        if (this->trackerManager->getTrackerList()[i]
                ->getFaceInfoVec()
                .size() <= 20) {
          this->trackerManager->getTrackerList()[i]->currentFaceInfo.faceId =
              tmpFaceIdVec[0];
          this->UpdateNewFace(i, 1);

          for (int j = 0; j < tmpFaceIdVec.size(); ++j)
            if (!tmpFaceIdVec[j].empty()) {
              this->trackerManager->getTrackerList()[i]
                  ->getFaceInfoVec()
                  .push_back(tmpFaceIdVec[j]);
            }
        }
      }
      index++;
    }
  }
}
/**
 * @brief FaceRecognition::UpdateNewFace
 * @param i
 */
void FaceRecognition::UpdateNewFace(int i, int type) {
  bool debug = false;

  if (this->trackerManager->getTrackerList()[i]->pose.empty())
    return;
  if (!this->trackerManager->getTrackerList()[i]->isLookingStraight())
    return;
  if (this->trackerManager->getTrackerList()[i]
          ->currentFaceInfo.faceImg.size() == 0)
    return;
  //  if
  //  (this->trackerManager->getTrackerList()[i]->currentFaceInfo.Is_Blurry())
  //    return;
  //  if (!this->trackerManager->getTrackerList()[i]
  //           ->currentFaceInfo.Is_Looking_Straight())
  //    return;
  // Create new Id
  string str;
  if (type == 0) {
    time_t now = time(0);
    if (this->trackerManager->getTrackerList()[i]->tmpName.empty()) {
      static char autoFaceID[30];
      strftime(autoFaceID, sizeof(autoFaceID), "Customer-%d%H%M%S",
               localtime(&now));
      this->getTrackerManager()->getTrackerList()[i]->tmpName = autoFaceID;
    }
    //  boost::filesystem::create_directory(this->getClassifier()->getFolderPath()
    //  +
    //                                      faceTrack->tmpName);

    string filename =
        "straight" + std::to_string(this->trackerManager->getTrackerList()[i]
                                        ->getNoFaceFoundFrameNumber());

    str = this->getTrackerManager()->getTrackerList()[i]->tmpName + "-" +
          filename + ".jpg";
  } else {
    string filename =
        "straight" +
        std::to_string(
            this->trackerManager->getTrackerList()[i]->getFaceInfoVec().size());
    str = this->trackerManager->getTrackerList()[i]->currentFaceInfo.faceId +
          "-" + filename + ".jpg";
  }

  this->trackerManager->getTrackerList()[i]->noSendDataFrameCount++;
  if (this->trackerManager->getTrackerList()[i]->noSendDataFrameCount % 5 ==
          1 &&
      this->trackerManager->getTrackerList()[i]->getTrackQuality() > 15) {

    if (this->trackerManager->getTrackerList()[i]->previousSentVec128.NR != 128)
      this->trackerManager->getTrackerList()[i]->previousSentVec128 =
          this->trackerManager->getTrackerList()[i]->currentFaceInfo.discriptor;
    else {
      if (dlib::length(
              this->trackerManager->getTrackerList()[i]->previousSentVec128 -
              this->trackerManager->getTrackerList()[i]
                  ->currentFaceInfo.discriptor) < 0.15)
        return;

      this->trackerManager->getTrackerList()[i]->previousSentVec128 =
          this->trackerManager->getTrackerList()[i]->currentFaceInfo.discriptor;
    }
  } else {

    return;
  }

  cv::Mat img = dlib::toMat(
      this->trackerManager->getTrackerList()[i]->currentFaceInfo.faceImg);
  cv::cvtColor(img, img, CV_BGR2RGB);
  //  if (!isBlurry(img))
  //    return;
  double blurry = getBlurryValue(img);
  /* Save image */
  if (debug)
    cout << "Save image : " << str << endl;

  std::stringstream stream;

  std::string vec2string = "";
  //  stream << K << " ";

  for (int j = 0; j < 128; ++j) {
    //    stream << std::fixed << std::setprecision(6) << vec(i) << " ";
    std::ostringstream ss;
    ss << this->trackerManager->getTrackerList()[i]->currentFaceInfo.discriptor(
        j);
    std::string s(ss.str());
    vec2string = vec2string + s + " ";
  }
  //  cout << "Size ------------ "
  //       << this->trackerManager->getTrackerList()[i]->getDataVec().size()
  //       << endl;
  //  this->trackerManager->getTrackerList()[i]->getDataVec().push_back(
  //      std::make_tuple(
  //          str, vec2string, this->trackerManager->getTrackerList()[i]->pose,
  //          blurry,
  //          this->trackerManager->getTrackerList()[i]->isLookingStraight(),
  //          img));

  //  Utils::sendDataToImgServer(img, str,
  //                             this->trackerManager->getTrackerList()[i]->tmpName,
  //                             vec2string, this->camId);
}
/**
 * @brief FaceRecognition::DrawNotification
 * @param frame
 * @param warningText
 * @param color
 * @param fontScale
 * @param thickness
 * @param lineType
 * @param posY
 */

void FaceRecognition::DrawNotification(Mat &frame, string warningText,
                                       cv::Scalar color, double fontScale,
                                       int thickness, int lineType, int posY) {
  int baseline = 0;
  cv::Size textSize = cv::getTextSize(warningText, FONT_HERSHEY_TRIPLEX,
                                      fontScale, thickness, &baseline);
  long x = this->getRecognitionArea().x +
           (this->getRecognitionArea().width - textSize.width) / 2;
  cv::putText(frame, warningText, cvPoint(x, posY), FONT_HERSHEY_TRIPLEX,
              fontScale, color, thickness, lineType);
}

void FaceRecognition::DrawRecognitionArea(cv::Mat &frame) {
  dlogf << LDEBUG << "[START] FaceRecognition::DrawRecognitionArea ";

  double fontscale = 0.6 * this->getFrameWidth() / 1280;

  //  cv::rectangle(frame, this->getRecognitionArea(), cv::Scalar(0, 255, 255),
  //  2);
  std::string text;
  if (this->getTrackerManager()->getTrackerList().size() > 0) {
    for (size_t i = 0; i < this->getTrackerManager()->getTrackerList().size();
         i++) {
      //      SingleTracker *singleTrack =
      //          this->getTrackerManager()->getTrackerList()[i];

      //      string textID = std::to_string(singleTrack->getTrackID());
      bool isDetecting = false;
      std::tuple<std::string, std::string, std::string> faceInfo;
      if (this->getTrackerManager()
              ->getTrackerList()[i]
              ->getTrackName()
              .empty()) {
        text = "Recognizing...";
      } else {
        //        faceInfo = this->getClassifier()->personDatas->findInfoById(
        //            this->getTrackerManager()->getTrackerList()[i]->getTrackName());
        std::string name =
            this->getTrackerManager()->getTrackerList()[i]->getTrackName();
        text = name.substr(1, name.length() - 3);
      }
      cv::Rect currentRect =
          this->getTrackerManager()->getTrackerList()[i]->currentFaceInfo.rect;
      int linespace = (int)30 * this->getFrameHeight() / 720;
      //            float ratioleft = 1.3;
      Scalar color = Scalar(255, 255, 255);
      int thickness = 1;

      int baseline = 0;

      cv::Size face_id_text_size = cv::getTextSize(
          text, cv::FONT_HERSHEY_TRIPLEX, fontscale * 1, thickness, &baseline);
      int y_tmp = currentRect.y - linespace;
      int y_face_id = y_tmp > face_id_text_size.height
                          ? y_tmp
                          : currentRect.y + (int)currentRect.height +
                                face_id_text_size.height + linespace;
      Utils::textCentered(frame, text, currentRect.x, currentRect.width,
                          y_face_id, cv::FONT_HERSHEY_TRIPLEX, fontscale * 1.3,
                          color, thickness, cv::LINE_8, false, true);

      Utils::Drawing_Detection(frame, currentRect, color);
    }
  }
}

/**
 * @brief FaceRecognition::operator ()
 * @param frame
 * @param countFrame
 * @param mode
 */

void FaceRecognition::processVideo() {
  XInitThreads();
  bool debug = false;
  int key = 0, cntForDetect = 0, cnt = 0;
  cv::Mat frame;
  cv::VideoCapture cap;
  dlogf << LINFO << "Video URL: " << this->camUrl;

  cv::namedWindow(this->camUrl, CV_WINDOW_NORMAL);
  //  cv::setWindowProperty(this->url, CV_WND_PROP_FULLSCREEN,
  //                        CV_WINDOW_FULLSCREEN);

  if (this->camUrl.empty())
    cap.open(0);
  else
    cap.open(this->camUrl);

  while (key != 27) {
    if (!cap.read(frame)) {
      std::cout << "Unable to retrieve frame from video stream." << std::endl;
      continue;
    }
    cv::resize(frame, frame, cv::Size(this->frameWidth, this->frameHeight));

    this->trackerManager->frame = frame;

    int64_t start = cv::getTickCount();
    cntForDetect++;
    cnt++;

    if (this->camUrl[0] == 'r') {
      if (cnt < 2) {
        continue;
      } else
        cnt = 0;
    }
    if (cntForDetect > 20)
      cntForDetect = 0;

    //    if (key == 65 || key == 117) // Key 'U' or 'u'
    //      this->UpdateParameter(_configUrl);

    this->FindingFace(frame, cntForDetect);
    //    if (this->camId.compare("01") == 0) {
    this->DrawRecognitionArea(frame);
    cv::imshow(this->camUrl, frame);
    //    }
    if (debug) {
      double fps = cv::getTickFrequency() / (cv::getTickCount() - start);
      std::cout << "FPS : " << fps << std::endl;
    }
    //    if (this->camId.compare("01") == 0)
    key = cv::waitKey(1);
  }
}

/**
 * @brief FaceRecognition::updateProfile
 * @param _id
 * @param _name
 * @param _gender
 * @param _age
 * @param _url
 * @param _vec128
 */
void FaceRecognition::updateProfile(std::string _id, std::string _name,
                                    std::string _gender, std::string _age,
                                    std::string _url, std::string _vec128) {
  this->getClassifier()->addNewProfile(_id, _name, _gender, _age, _url,
                                       _vec128);
}

void FaceRecognition::clearTrack() {
  this->trackerManager->getTrackerList().clear();
}
void sendSocket(std::string jsonstr) {
  CURLcode ret;
  CURL *hnd;
  struct curl_slist *slist1;

  // send to CMS
  slist1 = NULL;
  slist1 = curl_slist_append(slist1, "Content-Type: application/json");
  hnd = curl_easy_init();
  char url[1000];
  strcpy(url, "http://192.168.100.5:8069/face");
  // get plate
  curl_easy_setopt(hnd, CURLOPT_URL, url);
  curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
  curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, jsonstr.c_str());
  curl_easy_setopt(hnd, CURLOPT_USERAGENT, "VP9-ANPR");
  curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, slist1);
  curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
  curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "POST");
  curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);

  ret = curl_easy_perform(hnd);

  curl_easy_cleanup(hnd);
  hnd = NULL;
  curl_slist_free_all(slist1);
  slist1 = NULL;
}

void FaceRecognition::processTestVideo(bool isShow) {
  //  SocketIoUtils socket;
  //  socket.Connect2Server();
  XInitThreads();
  int key = 0, cntForDetect = 0, cnt = 0;
  cv::Mat frame;
  cv::VideoCapture cap;
  int unableFrameCount = 0;
  dlogf << LINFO << "Video URL: " << this->camUrl;

  if (this->camUrl.empty())
    cap.open(0);
  else
    cap.open(this->camUrl);

  while (1) {
    bool success = cap.read(frame);
    if (!success) {
      cap.open(this->camUrl);
      continue;
    }
    //    cap >> frame;
    cv::resize(frame, frame, cv::Size(this->frameWidth, this->frameHeight));

    this->trackerManager->frame = frame.clone();

    // int64_t start = cv::getTickCount();
    cntForDetect++;
    cnt++;

    if (cnt < 2) {
      continue;
    } else
      cnt = 0;

    if (cntForDetect > 61)
      cntForDetect = 0;

    //    this->setMode(MODE::ARLERT);
    findFaceOnly(frame, cntForDetect);

    this->DrawRecognitionArea(frame);

    std::vector<uchar> buf;
    imencode(".jpg", frame, buf);

    uchar *enc_msg = new uchar[buf.size()];
    for (int i = 0; i < (int)buf.size(); i++)
      enc_msg[i] = buf[i];

    string imgString;
    imgString = base64_encode(enc_msg, buf.size());
    std::string jsonstr = "{";

    jsonstr.append("\"frame\":\"");
    jsonstr.append(imgString);
    jsonstr.append("\"");

    jsonstr.append(",\"time_stamp\":\"");
    jsonstr.append(std::to_string(std::time(0)));
    jsonstr.append("\"");

    Json::Value face_arr(Json::arrayValue);
    for (int i = 0; i < this->trackerManager->getTrackerList().size(); i++) {
      Json::Value mini_value;
      mini_value["mongo_id"] =
          this->trackerManager->getTrackerList()[i]->mongo_id;
      mini_value["path"] = this->trackerManager->getTrackerList()[i]->path;
      face_arr.append(mini_value);
    }
    cout << face_arr.size() << endl;
    Json::FastWriter fastWriter;
    jsonstr.append(",\"face\":");
    jsonstr.append(fastWriter.write(face_arr));
    jsonstr.append("");

    jsonstr.append(",\"camera_id\":\"");
    jsonstr.append(camId);
    jsonstr.append("\"");

    jsonstr.append("}");
    //    cout << jsonstr << endl;
    sendSocket(jsonstr);
    //      socket.h.socket()->emit("getFace", jsonstr);
    cv::resize(frame, frame, cv::Size(640, 320));
    cv::imshow(this->camUrl, frame);

    //    double fps = cv::getTickFrequency() / (cv::getTickCount() - start);
    //    std::cout << "FPS : " << fps << std::endl;
    //    if (camId.compare("35761") == 0) {
    key = cv::waitKey(1);
  }
}
#if 0
void FaceRecognition::processWSVideo(
    std::function<void(std::string jsonData)> SendDataDelegate, bool isShow) {
  XInitThreads();
  this->trackerManager->SendDataFunction =
      std::bind(SendDataDelegate, std::placeholders::_1);

  int key = 0, cntForDetect = 0, cnt = 0;
  WSCapture WSInstance1;
  WSInstance1.open(camUrl);
  dlogf << LINFO << "Video URL: " << camUrl;

  this->trackerManager->roi = this->recognitionROI;
  this->trackerManager->camId = this->camId;
  //  cv::VideoCapture cap;
  //  cap.open(camUrl);
  cv::Mat frame;
  while (true) {
    this_thread::sleep_for(chrono::milliseconds(1));
    //    usleep(1);
    auto data = WSInstance1.getNextData();

    frame = data.frame.clone();
    int ts = data.timestamp;
    //    cv::Mat frame;
    //    cap.read(frame);
    if (frame.empty())
      continue;
    //    cout << frame.cols << "    " << frame.rows << endl;
    cv::resize(frame, frame, cv::Size(this->frameWidth, this->frameHeight));
    this->trackerManager->frame = frame.clone();

    // int64_t start = cv::getTickCount();
    cntForDetect++;
    cnt++;

    if (cnt < 2) {
      continue;
    } else
      cnt = 0;

    if (cntForDetect > 6)
      cntForDetect = 0;

    findFaceOnly(frame, cntForDetect);

    if (isShow) {
      this->DrawRecognitionArea(frame);
      cv::resize(frame, frame, cv::Size(640, 360));
      cv::imshow(camUrl, frame);
    }
    //    if (camId.compare("01") == 0)
    key = cv::waitKey(20);
    //    else
    //      usleep(20);
  }
}
#endif
std::string FaceRecognition::wrapJsonDataDrawCanvas() {
  Json::Value jdata;
  Json::Value jrect;
  Json::Value jspeed;
  jdata["engine_id"] = this->camId;
  jdata["timestamp"] = to_string(trackerManager->ts);
  jdata["length"] = 1 + (int)trackerManager->getTrackerList().size();
  //  jrect["x"] = recognitionROI.x;
  //  jrect["y"] = recognitionROI.y;
  //  jrect["width"] = recognitionROI.width;
  //  jrect["height"] = recognitionROI.height;
  //  jdata["rects"][0] = jrect;
  //  jspeed["x"] = recognitionROI.x;
  //  jspeed["y"] = recognitionROI.y;
  jrect["x"] = 0;
  jrect["y"] = 0;
  jrect["width"] = 1;
  jrect["height"] = 1;
  jdata["rects"][0] = jrect;
  jspeed["x"] = 0;
  jspeed["y"] = 0;
  std::string textId;
  jspeed["value"] = "";
  jdata["speeds"][0] = jspeed;
  for (int i = 0; i < (int)trackerManager->getTrackerList().size(); i++) {
    // jrect["key"] = info2.key[i];
    jrect["x"] = trackerManager->getTrackerList()[i]->currentFaceInfo.rect.x;
    jrect["y"] = trackerManager->getTrackerList()[i]->currentFaceInfo.rect.y;
    jrect["width"] =
        trackerManager->getTrackerList()[i]->currentFaceInfo.rect.width;
    jrect["height"] =
        trackerManager->getTrackerList()[i]->currentFaceInfo.rect.height;
    jdata["rects"][i + 1] = jrect;
  }
  for (int i = 0; i < (int)trackerManager->getTrackerList().size(); i++) {
    int x = trackerManager->getTrackerList()[i]->currentFaceInfo.rect.x;
    int y = trackerManager->getTrackerList()[i]->currentFaceInfo.rect.y - 20;
    jspeed["x"] = x;
    jspeed["y"] = y;
    std::string textId;
    //    if (!trackerManager->getTrackerList()[i]->getTrackName().empty())
    //      textId = this->getClassifier()
    //                   ->personDatas
    //                   ->findNameById(
    //                       trackerManager->getTrackerList()[i]->getTrackName())
    //                   .c_str();
    //    else
    textId = "Detecting..";
    jspeed["value"] = textId;
    jdata["speeds"][i + 1] = jspeed;
  }
  return jdata.toStyledString();
}
#if 0
void FaceRecognition::processWSVideoCanvas(
    std::function<void(std::string json)> syncInfoForCanvas, bool isShow) {

  cv::Rect camRoi(200, 150, 800, 600);
  cout << camRoi << endl;
  int key = 0, cntForDetect = 0, cnt = 0;
  WSCapture WSInstance;
  WSInstance.open(camUrl);
  dlogf << LINFO << "Video URL: " << camUrl;
  if (isShow)
    cv::namedWindow(camUrl, CV_WINDOW_NORMAL);

  this->trackerManager->roi = this->recognitionROI;
  camId = this->camId;
  //  cv::VideoCapture cap;
  //  cap.open(camUrl);
  while (true) {
    //    this_thread::sleep_for(chrono::milliseconds(1));
    usleep(1);
    auto data = WSInstance.getNextData();

    cv::Mat frame = data.frame.clone();
    this->trackerManager->ts = data.timestamp;
    //    cv::Mat frame;
    //    cap.read(frame);
    if (frame.empty())
      continue;
    //    cout << frame.cols << "    " << frame.rows << endl;
    cv::resize(frame, frame, cv::Size(this->frameWidth, this->frameHeight));
    this->trackerManager->frame = frame.clone();

    // int64_t start = cv::getTickCount();
    cntForDetect++;
    cnt++;

    if (this->camUrl[0] == 'r' || this->camUrl[0] == 'w') {
      if (cnt < 2) {
        continue;
      } else
        cnt = 0;
    }
    if (cntForDetect > 10)
      cntForDetect = 0;

    //    this->setMode(MODE::ARLERT);
    this->FindingFace(frame, cntForDetect);
    //    if (trackerManager->getTrackerList().size() > 0)
    syncInfoForCanvas(wrapJsonDataDrawCanvas());

    if (isShow)
      this->DrawRecognitionArea(frame);

    if (isShow)
      cv::imshow(camUrl, frame);

    //    if (camId.compare("01") == 0) {
    key = cv::waitKey(30);
  }
}
#endif
void mergeOverlappingBoxes(std::vector<cv::Rect> &inputBoxes, cv::Mat &image,
                           std::vector<cv::Rect> &outputBoxes) {
  cv::Mat mask =
      cv::Mat::zeros(image.size(), CV_8UC1); // Mask of original image
  cv::Size scaleFactor(10, 10); // To expand rectangles, i.e. increase
                                // sensitivity to nearby rectangles. Doesn't
                                // have to be (10,10)--can be anything
  for (int i = 0; i < inputBoxes.size(); i++) {
    cv::Rect box = inputBoxes.at(i) + scaleFactor;
    cv::rectangle(mask, box, cv::Scalar(255),
                  CV_FILLED); // Draw filled bounding boxes on mask
  }

  std::vector<std::vector<cv::Point>> contours;
  // Find contours in mask
  // If bounding boxes overlap, they will be joined by this function call
  cv::findContours(mask, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
  for (int j = 0; j < contours.size(); j++) {
    if (contourArea(contours[j]) > image.rows / 4 * image.cols / 4)
      outputBoxes.push_back(cv::boundingRect(contours.at(j)));
  }
}

void FaceRecognition::bgsProcess() {
  std::string fileName = "2018_01_16_222.mp4";
  VideoCapture cap(fileName);
  cv::Mat image, image2, image3, image4, gray, frame;
  if (!cap.isOpened()) {
    std::cerr << "Failed to open video capture" << std::endl;
    return;
  }
  cv::Mat kernel = getStructuringElement(MORPH_CROSS, Size(3, 3));
  cv::Mat1b fgMask;
  cv::Ptr<BackgroundSubtractor> pBgSub;
  cv::Ptr<BackgroundSubtractorMOG2> pBgSubMOG2 =
      createBackgroundSubtractorMOG2();
  pBgSubMOG2->setDetectShadows(false);
  pBgSubMOG2->setNMixtures(3);
  pBgSubMOG2->setShadowThreshold(0.8);
  pBgSubMOG2->setShadowValue(0);
  // pBgSubMOG2->setVarThreshold(0.9);
  pBgSub = pBgSubMOG2;

  frontal_face_detector _detector = get_frontal_face_detector();
  double scale = 1.5;
  while (true) {
    cap >> frame;
    //    cv::Mat frameCp = frame.clone();
    //    cv::resize(frame, frame,
    //               cv::Size((int)frame.cols / scale, (int)frame.rows /
    //               scale));
    cv::resize(frame, frame, cv::Size(1280, 720));
    std::vector<cv::Rect> detection;

    cv::Mat colorForeground = cv::Mat::zeros(frame.size(), frame.type());
    Mat gray;
    cvtColor(frame, gray, COLOR_BGR2GRAY);
    pBgSub->apply(gray, fgMask);
    // Clean foreground from noise
    morphologyEx(fgMask, fgMask, MORPH_OPEN, kernel);

    Mat Mat_res;
    std::vector<std::vector<Point>> contours;

    Mat_res = fgMask.clone();

    // Find contours
    findContours(Mat_res, contours, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);
    int width = frame.cols;
    int height = frame.rows;
    if (!contours.empty()) {
      // Get largest contour

      for (int i = 0; i < (int)contours.size(); ++i) {
        double area = contourArea(contours[i]);
        Rect roi = boundingRect(contours[i]);
        // std::cout << area <<std::endl;
        //        if (area > width / 8 * height / 8) {

        // Only add detection with a shape of human body
        // if ((float)roi.height*0.9 > (float)roi.width){
        detection.push_back(roi);
        // }
        //        }
      }
    }
    std::vector<cv::Rect> mergeDetection;
    mergeOverlappingBoxes(detection, frame, mergeDetection);
    std::vector<cv::Rect> rects;
    for (int i = 0; i < mergeDetection.size(); ++i) {

      cv::rectangle(frame, mergeDetection[i], cv::Scalar(0, 0, 255), 1);
      //

      cv::Mat motionArea = frame(mergeDetection[i]);
      matrix<rgb_pixel> img = Utils::Convert_Mat2Dlib(motionArea);
      dlib::array2d<unsigned char> imgGray;
      dlib::assign_image(imgGray, img);

      std::vector<dlib::rectangle> dets = _detector(imgGray);
      for (dlib::rectangle det : dets) {
        cv::Rect rec((det.left() + mergeDetection[i].x),
                     (det.top() + mergeDetection[i].y), det.width(),
                     det.height());
        cv::rectangle(frame, rec, cv::Scalar(255, 255, 255), 2);
      }
    }

    imshow("bgs", Mat_res);
    imshow("window", frame);
    int k = waitKey(1) & 0xff;
    if ('q' == k || 27 == k)
      break;
  }
}

void FaceRecognition::operator()() {
  //  SocketIoUtils socket;
  //  socket.Connect2Server();
  while (true) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    if (frame_queue.empty())
      continue;
    frame_mutex.lock();
    FrameData data = frame_queue.front();
    frame_queue.pop();
    frame_mutex.unlock();

    cv::Mat frame = data.frame.clone();
    cv::Mat frame_copy = data.frame.clone();
    std::vector<dlib::rectangle> dlib_recs;
    dlib::cv_image<bgr_pixel> cv_temp(frame_copy);
    matrix<rgb_pixel> img;
    dlib::assign_image(img, cv_temp);
    dlib::array2d<unsigned char> imgGray;
    dlib::assign_image(imgGray, img);

    //        cv::imshow("before", frame);
    double scaleMtcnn = 2;
    if (data.frame_id % 4 == 0) {
      std::vector<CaffeDetector::BoundingBox> dets =
          this->classifier->detector->detectUsingCaffe(frame_copy, scaleMtcnn);
      for (CaffeDetector::BoundingBox d : dets) {
        dlib::rectangle det(d.x1 * scaleMtcnn - 5, d.y1 * scaleMtcnn - 10,
                            d.x2 * scaleMtcnn + 5, d.y2 * scaleMtcnn + 10);
        // dlib_recs.push_back(det);
        trackerManager->insertTracker(imgGray, det);
      }
    } else {
      trackerManager->updateTracker(imgGray);
    }
    int size = trackerManager->getTrackerList().size();

    if (size > 0) {
      std::vector<dlib::matrix<float, 0, 1>> discriptors =
          this->get_128D(img, dlib_recs);
      for (int i = 0; i < size; ++i) {
        // trackerManager->getTrackerList()[i]->vector128 = discriptors[i];

        // cv::Rect rec = trackerManager->getTrackerList()[i]->;

        // cv::rectangle(frame, rec, cv::Scalar(0, 255, 0));
      }
      if (data.frame_id % 80 == 0) {
        // track_manager.search_by_vector();
      }
    }

    cv::imshow("aaa", frame);
    cv::waitKey(20);
  }
}
