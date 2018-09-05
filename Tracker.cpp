#include "Tracker.hpp"
#include "StorageUtils.hpp"
#include "Utils.hpp"
#include "base64.h"
logger trackerLogger("FaceCNN.Tracker");
#include "restclient-cpp/restclient.h"
/**
 * @brief SingleTracker::SingleTracker Create new track. Match with one object
 * @param _img
 * @param _rect
 */
SingleTracker::SingleTracker(dlib::array2d<unsigned char> &_img,
                             dlib::rectangle _rect, int _trackId) {
  this->setTrackID(_trackId);
  //  trackerLogger << LDEBUG << "[START] tracking!!" << this->getTrackID();
  //  cout << "[START] tracking!!" << this->getTrackID() << endl;
  this->setRect(_rect);
  this->setCenter(_rect);
  this->tracker.start_track(_img, _rect);
  this->trackName = "";
  this->trackName.clear();
  this->lostTrackCount = 0;
  this->noFaceFoundFrameNumber = 0;
  this->trace = 0;

  for (int i = 0; i < 128; ++i)
    vec128Sum(i) = 0;
}

/**
 * @brief SingleTracker::SingleTracker Create new track. Match with one object
 * @param _img
 * @param _rect
 */
SingleTracker::SingleTracker(dlib::array2d<unsigned char> &_img,
                             dlib::rectangle _rect, int _trackId,
                             std::vector<dlib::dpoint> shape) {
  this->setTrackID(_trackId);
  //  trackerLogger << LDEBUG << "[START] tracking!!" << this->getTrackID();
  //  cout << "[START] tracking!!" << this->getTrackID() << endl;
  this->setRect(_rect);
  this->setCenter(_rect);
  this->tracker.start_track(_img, _rect);
  this->trackName = "";
  this->trackName.clear();
  this->lostTrackCount = 0;
  this->noFaceFoundFrameNumber = 0;
  this->isUploadFirst = false;
  this->trace = 0;
  for (int i = 0; i < 128; ++i)
    vec128Sum(i) = 0;
  this->shape5point = shape;
}
/**
 * @brief SingleTracker::doSingleTrack update existed track
 * @param _img
 */

void SingleTracker::doSingleTrack(dlib::array2d<unsigned char> &_img) {
  //  trackerLogger << LDEBUG << "[START] dosingleTrack   " << trackId << "
  //  ----- "
  //                << trackName.c_str();
  this->trackQuality = this->tracker.update(_img);
  dlib::rectangle pos = this->tracker.get_position();
  this->rect = pos;
  //  this->movingDistance = std::sqrt(
  //      std::pow(pos.tl_corner().x() + (pos.width() / 2) - center.x(), 2) +
  //      std::pow(pos.tl_corner().y() + (pos.height() / 2) - center.y(), 2));
  this->setCenter(pos);
  //  if (this->trackName.empty() && this->faceIdVec.size() >=
  //  maxFaceIdForVoting)
  //    this->selectFaceId();
  this->isNeedDelete = this->isNeedToDeleted();
  // cout << "Moving distance " << this->movingDistance << endl;
}

/**
 * @brief SingleTracker::doSingleTrack update existed track
 * @param _img
 */

void SingleTracker::doSingleTrackWithNewBox(dlib::array2d<unsigned char> &_img,
                                            dlib::rectangle newBox) {
  //  trackerLogger << LDEBUG << "[START] doSingleTrackWithNewBox   " << trackId
  //                << " ----- " << trackName.c_str();
  this->tracker.start_track(_img, newBox);
  this->trackQuality = 20;
  this->rect = newBox;
  this->setCenter(newBox);
  this->lostTrackCount = 0;

  //  if (this->trackName.empty() && this->faceIdVec.size() >=
  //  maxFaceIdForVoting)
  //    this->selectFaceId();
  this->isNeedDelete = this->isNeedToDeleted();
  // cout << "Moving distance " << this->movingDistance << endl;
}

void SingleTracker::doSingleTrackWithNewBox(dlib::array2d<unsigned char> &_img,
                                            dlib::rectangle newBox,
                                            std::vector<dlib::dpoint> shape) {
  //  trackerLogger << LDEBUG << "[START] doSingleTrackWithNewBox   " << trackId
  //                << " ----- " << trackName.c_str();
  this->tracker.start_track(_img, newBox);
  this->trackQuality = 20;
  this->rect = newBox;
  this->setCenter(newBox);
  this->lostTrackCount = 0;
  this->shape5point = shape;
  if (this->trackName.empty() && this->faceIdVec.size() >= maxFaceIdForVoting)
    this->selectFaceId();
  this->isNeedDelete = this->isNeedToDeleted();
  // cout << "Moving distance " << this->movingDistance << endl;
}
/**
 * @brief SingleTracker::isNeedToDeleted Check condition to delete track in next
 * 15 frames
 * @return true/false
 */

bool SingleTracker::isNeedToDeleted() {
  bool res = false;
  if (this->getTrackQuality() > 8) {
    return false;
  } else {
    return true;
    //    this->lostTrackCount++;
    //    if (this->lostTrackCount > 10)
    //      res = true;
    //    cout << lostTrackCount << " xxxx " << lostTrackCountThreshold << " xxx
    //    "
    //         << trackQuality << endl;
  }
  return res;
}

/**
 * @brief SingleTracker::isDeletedNextFrame Detele in next frame
 * @return
 */

bool SingleTracker::isDeletedNextFrame() {
  if (this->isNeedDelete) {
    if (this->lostTrackCount == 1)
      return true;
    else
      this->lostTrackCount++;
  }
  return false;
}

/**
 * @brief SingleTracker::isMatchWithNewTrack Check new track match with existed
 * track or not
 * @param _rect
 * @return
 */

bool SingleTracker::isMatchWithNewTrack(dlib::rectangle _rect) {
  long x_bar = _rect.left() + _rect.width() * 0.5;
  long y_bar = _rect.top() + _rect.height() * 0.5;
  drectangle pos = this->rect;
  dlib::point center = this->center;

  if ((pos.left() <= x_bar) && (x_bar <= pos.right()) && (pos.top() <= y_bar) &&
      (y_bar <= pos.bottom()) && (_rect.left() <= center.x()) &&
      (center.x() <= _rect.right()) && (_rect.top() <= center.y()) &&
      (center.y() <= _rect.bottom())) {
    return true;
  } else
    return false;
}

/**
 * @brief SingleTracker::selectFaceId Decide trackName by faceId Vector
 * @return True if have trackName / False if not
 */

bool SingleTracker::selectFaceId() {
  bool res = false;
  std::map<std::string, int> duplicate;
  for (auto element : this->faceIdVec)
    ++duplicate[element];

  std::map<int, std::string, greater<int>> new_map;
  for (std::map<std::string, int>::iterator it = duplicate.begin();
       it != duplicate.end(); ++it) {
    new_map.insert(make_pair(it->second, it->first));
  }
  for (std::map<int, std::string, greater<int>>::iterator it = new_map.begin();
       it != new_map.end(); ++it) {

    //    cout << it->first << "----z----" << it->second << endl;
  }
  if (new_map.begin()->first / maxFaceIdForVoting >= 0.45)
    this->trackName = new_map.begin()->second;
  else
    this->trackName = "Unknown";

  // if (tmpFaceId)
  // this->trackName = tmpFaceId.empty() ? "Unknown" : tmpFaceId;
  //  cout << this->trackName << endl;
  res = this->trackName.empty() ? true : false;
  return res;
}
/**
 * @brief TrackerManager::findMatchedTracker Find matched new track with track
 * list
 * @param _rect
 * @return
 */

int TrackerManager::findMatchedTracker(dlib::rectangle _rect) {
  trackerLogger << LDEBUG << "[START] TrackerManager::findMatchedTracker";
  int res = -1;
  if (this->trackList.size() > 0) {
    for (int i = 0; i < this->trackList.size(); ++i) {
      if (this->trackList[i]->isMatchWithNewTrack(_rect))
        res = this->trackList[i]->getTrackID();
    }
  }
  return res;
}

/**
 * @brief TrackerManager::insertTracker insert new track into track list
 * @param _img
 * @param _rect
 */

void TrackerManager::insertTracker(dlib::array2d<unsigned char> &_img,
                                   dlib::rectangle _rect) {
  trackerLogger << LDEBUG << "[START] TrackerManager::insertTracker";
  int matchId = this->findMatchedTracker(_rect);

  if (matchId == -1) {

    this->lastTrackId++;
    std::shared_ptr<SingleTracker> newTracker(
        new SingleTracker(_img, _rect, lastTrackId));
    this->trackList.push_back(newTracker);

  } else {
    for (int i = 0; i < this->trackList.size(); ++i) {
      if (this->trackList[i]->getTrackID() == matchId)
        this->trackList[i]->doSingleTrackWithNewBox(_img, _rect);
    }
  }
  trackerLogger << LDEBUG << "[END] TrackerManager::insertTracker";
}

void TrackerManager::insertTracker(dlib::array2d<unsigned char> &_img,
                                   dlib::rectangle _rect,
                                   std::vector<dlib::dpoint> d) {
  trackerLogger << LDEBUG << "[START] TrackerManager::insertTracker";
  int matchId = this->findMatchedTracker(_rect);

  if (matchId == -1) {

    this->lastTrackId++;
    std::shared_ptr<SingleTracker> newTracker(
        new SingleTracker(_img, _rect, lastTrackId, d));
    this->trackList.push_back(newTracker);

  } else {
    for (int i = 0; i < this->trackList.size(); ++i) {
      if (this->trackList[i]->getTrackID() == matchId) {
        this->trackList[i]->doSingleTrackWithNewBox(_img, _rect, d);

        //        this->trackList[i]->getFaceImgVec().push_back(CvtMat2Base64(face));
      }
    }
  }
  trackerLogger << LDEBUG << "[END] TrackerManager::insertTracker";
}
const string currentDateTime() {
  time_t now = time(0);
  struct tm tstruct;
  char buf[80];
  tstruct = *localtime(&now);
  // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
  // for more information about date/time format
  strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

  return buf;
}
void TrackerManager::updateTracker(dlib::array2d<unsigned char> &_img) {
  // update track
  StorageUtils storageutils;
  std::vector<std::thread> threadPool;
  std::for_each(trackList.begin(), trackList.end(),
                [&](std::shared_ptr<SingleTracker> ptr) {
                  threadPool.emplace_back(
                      [ptr, &_img]() { ptr->doSingleTrack(_img); });
                });

  for (int i = 0; i < threadPool.size(); i++)
    threadPool[i].join();

  // Check for delete

  for (int i = 0; i < trackList.size(); ++i) {
    std::string f_name = "img_" + std::to_string(std::time(0)) + ".jpg";

    cv::Rect rec = trackList.at(i)->currentFaceInfo.rect;
    if (rec.x < 0)
      rec.x = 0;
    if (rec.y < 0)
      rec.y = 0;

    cv::Mat frame_cp = this->frame.clone()(rec);
    storageutils.uploadFile(frame_cp, f_name, this->camId);

    this->trackList[i]->path = "/faces/" + f_name;
    this->trackList[i]->trace++;
    if (this->trackList[i]->isDeletedNextFrame()) {
      this->deleteTracker(this->trackList[i]->getTrackID());
    }
  }
}

/**
 * @brief TrackerManager::findTracker Find track by trackId
 * @param _trackId
 * @return
 */

int TrackerManager::findTracker(int _trackId) {
  trackerLogger << LDEBUG << "[START] TrackerManager::findTracker";
  auto target =
      std::find_if(this->trackList.begin(), this->trackList.end(),
                   [&, _trackId](std::shared_ptr<SingleTracker> ptr) -> bool {
                     return ptr->getTrackID() == _trackId;
                   });

  if (target == this->trackList.end())
    return -1;
  else
    return target - this->trackList.begin();
}

/**
 * @brief TrackerManager::deleteTracker Delete track by trackId
 * @param _trackId
 */

void TrackerManager::deleteTracker(int _trackId) {
  trackerLogger << LDEBUG << "[START] TrackerManager::deleteTracker "
                << _trackId;
  int result_idx = this->findTracker(_trackId);

  if (result_idx != -1) {
    //    cout << this->trackList[result_idx]->currentFaceInfo.rect << endl;
    //    cout << roi << endl;
    //    cout << this->trackList[result_idx]->currentFaceInfo.rect << endl;
    //    if ((this->trackList[result_idx]->currentFaceInfo.rect.x <=
    //         (this->roi.x + 30)) ||
    //        //        (this->trackList[result_idx]->currentFaceInfo.rect.y <=
    //        //         this->roi.y + 30) ||
    //        (this->trackList[result_idx]->currentFaceInfo.rect.br().x >=
    //         (this->roi.br().x - 30)) ||
    //        (this->trackList[result_idx]->currentFaceInfo.rect.br().y >=
    //         (this->roi.br().y) - 30)) {
    //      //      cout << "Size before: "
    //      //           << this->trackList[result_idx]->getDataVec().size() <<
    //      endl;

    //      int pos = this->trackList[result_idx]->removeOutlierData();
    //      //      cout << "After before: "
    //      //           << this->trackList[result_idx]->getDataVec().size() <<
    //      endl;
    //      if (this->trackList[result_idx]->getDataVec().size() != 0) {
    //        SendDataFunction(wrapJsonData(result_idx, pos));
    //      }
  }

  this->trackList[result_idx].reset();

  this->trackList.erase(trackList.begin() + result_idx);
  //    cout << "Delete " << result_idx << endl;
}

std::string TrackerManager::wrapJsonData(int index, int indexCover) {
  //  const clock_t begin_time = clock();
  Json::Value jdata;
  Json::Value jimage;
  std::string faceId;

  StorageUtils storageutils;
  if (this->trackList[index]->trackName.empty() ||
      this->trackList[index]->trackName.compare("Unknown") == 0) {
    faceId = this->trackList[index]->tmpName;
    jdata["type"] = "customer";
  } else {
    faceId = this->trackList[index]->trackName.c_str();
    jdata["type"] = "staff";
  }

  std::string path = "http://facedb.vp9.vn/human/" + faceId + "/";
  jdata["FaceId"] = faceId;
  jdata["CamId"] = this->camId;
  jdata["placeId"] = this->placeId;
  jdata["Time"] = currentDateTime();
  std::string centerVec = "";
  int size = trackList[index]->getDataVec().size();

  jdata["vector"] = std::get<1>(trackList[index]->getDataVec()[indexCover]);

  for (int i = 0; i < size; ++i) {

    std::string url =
        faceId + "/" + std::get<0>(trackList[index]->getDataVec()[i]);
    storageutils.uploadFile(std::get<3>(trackList[index]->getDataVec()[i]), url,
                            this->camId);
    std::string s3Url = path + std::get<0>(trackList[index]->getDataVec()[i]);
    jimage["url"] = s3Url;
    jimage["vector"] = std::get<1>(trackList[index]->getDataVec()[i]);
    jimage["pose"] = std::get<2>(trackList[index]->getDataVec()[i]);
    jimage["blurry"] = std::get<4>(trackList[index]->getDataVec()[i]);

    jdata["data"][i] = jimage;
  }

  jdata["cover"] =
      path + std::get<0>(trackList[index]->getDataVec()[indexCover]);
  std::string status = std::get<2>(trackList[index]->getDataVec()[indexCover])
                                   .compare("straight") != 0
                           ? "False"
                           : "True";
  jdata["status"] = status;

  return jdata.toStyledString();
}

// binhnp
void SingleTracker::calculateAngleDirectionLandmarkBased5Points() {
  bool debug = false;
  if (shape5point[0].x() == 0.0)
    return;
  // bool smiling;
  double eyeleft_to_nose = pow(shape5point[2].x() - shape5point[0].x(), 2) +
                           pow(shape5point[2].y() - shape5point[0].y(), 2);
  double eyeright_to_nose = pow(shape5point[2].x() - shape5point[1].x(), 2) +
                            pow(shape5point[2].y() - shape5point[1].y(), 2);

  Point middle_of_2_eyes((shape5point[1].x() + shape5point[1].x()) / 2,
                         (shape5point[1].y() + shape5point[1].y()) / 2);

  double middle_eyes_to_nose = pow(middle_of_2_eyes.x - shape5point[2].x(), 2) +
                               pow(middle_of_2_eyes.y - shape5point[2].y(), 2);

  Point middle_of_mouth((shape5point[3].x() + shape5point[4].x()) / 2,
                        (shape5point[3].y() + shape5point[4].y()) / 2);

  double middle_mouth_to_nose = pow(middle_of_mouth.x - shape5point[2].x(), 2) +
                                pow(middle_of_mouth.y - shape5point[2].y(), 2);
  double mouthleft_to_mouthright =
      pow(shape5point[3].x() - shape5point[4].x(), 2) +
      pow(shape5point[3].y() - shape5point[4].y(), 2);

  double eyeleft_to_eyeright = pow(shape5point[0].x() - shape5point[1].x(), 2) +
                               pow(shape5point[0].y() - shape5point[1].y(), 2);
  if (debug) {
    cout << "eyeleft_to_nose = " << eyeleft_to_nose << endl;
    cout << "eyeright_to_nose = " << eyeright_to_nose << endl;
  }
  // cout << "mouthleft_to_nose = " << mouthleft_to_nose << endl;

  double threshold_h_15 = 0.6;
  double threshold_h_30 = 0.45;
  double threshold_h_45 = 0.3;
  double threshold_h_60 = 0.2;
  double threshold_h_75 = 0.1;

  // Horizontal
  double h1_ratio = eyeleft_to_nose / eyeright_to_nose;
  double h2_ratio = eyeright_to_nose / eyeleft_to_nose;

  if ((shape5point[2].x() <= shape5point[0].x() &&
       shape5point[2].x() <= shape5point[3].x()) ||
      (shape5point[2].x() >= shape5point[1].x() &&
       shape5point[2].x() >= shape5point[4].x())) {
    if (shape5point[2].x() <= shape5point[0].x() &&
        shape5point[2].x() <= shape5point[3].x()) {
      direction_h = "left";
      angle_h = 90;
    } else {
      direction_h = "right";
      angle_h = 90;
    }
  } else {
    if (h1_ratio >= h2_ratio) {
      if (h2_ratio > threshold_h_15) {
        direction_h = "";
        angle_h = 0;
      } else if (h2_ratio <= threshold_h_15 && h2_ratio >= threshold_h_30) {
        direction_h = "right";
        angle_h = 15;
      } else if (h2_ratio <= threshold_h_30 && h2_ratio >= threshold_h_45) {
        direction_h = "right";
        angle_h = 30;
      } else if (h2_ratio <= threshold_h_45 && h2_ratio >= threshold_h_60) {
        direction_h = "right";
        angle_h = 45;
      } else if (h2_ratio <= threshold_h_60 && h2_ratio >= threshold_h_75) {
        direction_h = "right";
        angle_h = 60;
      } else if (h2_ratio <= threshold_h_75) {
        direction_h = "right";
        angle_h = 75;
      }
    } else {
      if (h1_ratio > threshold_h_15) {
        direction_h = "";
        angle_h = 0;
      } else if (h1_ratio <= threshold_h_15 && h1_ratio >= threshold_h_30) {
        direction_h = "left";
        angle_h = 15;
      } else if (h1_ratio <= threshold_h_30 && h1_ratio >= threshold_h_45) {
        direction_h = "left";
        angle_h = 30;
      } else if (h1_ratio <= threshold_h_45 && h1_ratio >= threshold_h_60) {
        direction_h = "left";
        angle_h = 45;
      } else if (h1_ratio <= threshold_h_60 && h1_ratio >= threshold_h_75) {
        direction_h = "left";
        angle_h = 60;
      } else if (h1_ratio <= threshold_h_75) {
        direction_h = "left";
        angle_h = 75;
      }
    }
  }

  // Vertial
  double threshold_v_up_15 = 0.2;
  double threshold_v_up_30 = 0.16;
  double threshold_v_up_45 = 0.12;
  double threshold_v_up_60 = 0.08;
  double threshold_v_up_75 = 0.04;
  double threshold_v_up_90 = 0.01;

  double threshold_v_down_15 = 0.25;
  double threshold_v_down_30 = 0.21;
  double threshold_v_down_45 = 0.17;
  double threshold_v_down_60 = 0.13;
  double threshold_v_down_75 = 0.09;
  double threshold_v_down_90 = 0.04;

  double h3_ratio = middle_eyes_to_nose / eyeleft_to_eyeright;
  double h4_ratio = middle_mouth_to_nose / mouthleft_to_mouthright;
  if (h3_ratio > h4_ratio) {
    if (h4_ratio > threshold_v_down_15) {
      direction_v = "";
      angle_v = 0;
    } else if (h4_ratio <= threshold_v_down_15 &&
               h4_ratio >= threshold_v_down_30) {
      direction_v = "down";
      angle_v = 15;
    } else if (h4_ratio <= threshold_v_down_30 &&
               h4_ratio >= threshold_v_down_45) {
      direction_v = "down";
      angle_v = 30;
    } else if (h4_ratio <= threshold_v_down_45 &&
               h4_ratio >= threshold_v_down_60) {
      direction_v = "down";
      angle_v = 45;
    } else if (h4_ratio <= threshold_v_down_60 &&
               h4_ratio >= threshold_v_down_75) {
      direction_v = "down";
      angle_v = 60;
    } else if (h4_ratio <= threshold_v_down_75 &&
               h4_ratio >= threshold_v_down_90) {
      direction_v = "down";
      angle_v = 75;
    } else if (h4_ratio < threshold_v_down_90) {
      direction_v = "down";
      angle_v = 90;
    }
  } else {
    if (h3_ratio > threshold_v_up_15) {
      direction_v = "";
      angle_v = 0;
    } else if (h3_ratio <= threshold_v_up_15 && h3_ratio >= threshold_v_up_30) {
      direction_v = "up";
      angle_v = 15;
    } else if (h3_ratio <= threshold_v_up_30 && h3_ratio >= threshold_v_up_45) {
      direction_v = "up";
      angle_v = 30;
    } else if (h3_ratio <= threshold_v_up_45 && h3_ratio >= threshold_v_up_60) {
      direction_v = "up";
      angle_v = 45;
    } else if (h3_ratio <= threshold_v_up_60 && h3_ratio >= threshold_v_up_75) {
      direction_v = "up";
      angle_v = 60;
    } else if (h3_ratio <= threshold_v_up_75 && h3_ratio >= threshold_v_up_90) {
      direction_v = "up";
      angle_v = 75;
    } else if (h3_ratio < threshold_v_up_90) {
      direction_v = "up";
      angle_v = 90;
    }
  }
  if (direction_v.empty() && direction_h.empty())
    pose = "straight";
  else {
    pose = direction_v + "" + direction_h;
  }

  //  cout << h1_ratio << "   " << h2_ratio << "  " << h3_ratio << "   " <<
  //  h4_ratio
  //       << endl;
  //  cout << "direction_h = " << direction_h << "  angle_h = " << angle_h <<
  //  endl;
  //  cout << "direction_v = " << direction_v << "  angle_v = " << angle_v <<
  //  endl;
}

bool SingleTracker::isLookingStraight() {
  if (pose.compare("straight") == 0)
    return true;
  //  else if (angle_v <= 30 || angle_h <= 30)
  //    return true;
  else
    return false;
}

int SingleTracker::removeOutlierData() {
  int num = this->dataVec.size();
  for (int i = 0; i < 128; ++i)
    vec128Sum(i) /= num;
  std::vector<double> distanceVec;
  for (int j = 0; j < this->dataVec.size(); ++j) {
    double d = dlib::length(vec128Sum - std::get<5>(dataVec[j]));
    if (d > 0.3) {
      dataVec.erase(dataVec.begin() + j);
    } else {
      distanceVec.push_back(d);
    }
  }

  int minPos = distance(distanceVec.begin(),
                        min_element(distanceVec.begin(), distanceVec.end()));
  cout << "Center vec : " << std::get<0>(dataVec[minPos]) << endl;
  //  for (int i = 0; i < 128; ++i)
  //  vec128Sum = std::get<5>(dataVec[minPos]);
  return minPos;
}

void TrackerManager::search_by_vector() {
  //

  Json::Value jsonData;

  int total = trackList.size();
  jsonData["total"] = total;
  Json::Value arrData(Json::arrayValue);
  for (int i = 0; i < total; i++) {
    Json::Value miniValue;
    miniValue["track_id"] = trackList.at(i)->getTrackID();
    miniValue["camid"] = this->camId;
    miniValue["path"] = trackList.at(i)->path;

    std::vector<double> vt(
        dlib::trans(trackList.at(i)->currentFaceInfo.discriptor).begin(),
        dlib::trans(trackList.at(i)->currentFaceInfo.discriptor).end());
    Json::Value vtData(Json::arrayValue);
    for (int j = 0; j < vt.size(); j++) {
      vtData.append(Json::Value(vt[j]));
    }
    miniValue["vector"] = vtData;

    arrData.append(miniValue);
  }
  jsonData["data"] = arrData;
  //  RestClient::Response res = api_conn.QueryAPI(URL_POST_API, jsonData);

  Json::FastWriter fastWriter1;
  std::string jsString = fastWriter1.write(jsonData);
  //  cout << "============= jsstring =======" << endl;
  //  cout << jsString << endl;
  RestClient::Response res =
      RestClient::post("http://192.168.100.5:8024/searchbyvector?isface=0",
                       "application/json", jsString);
  Json::Value body_res;
  Json::Reader reader;
  bool parsingSuccessful = reader.parse(res.body, body_res);

  //  cout << body_res << endl;

  // ** save data from response
  Json::FastWriter fastWriter;
  if (body_res["person"]["data"].size() > 0) {
    for (int i = 0; i < body_res["person"]["data"].size(); i++) {
      std::string jsString;
      std::string id_query =
          fastWriter.write(body_res["person"]["data"][i]["id"]);
      //      removeCharsFromString(jsString, "\"");
      //      obj_m.id_query = jsString;
      jsString = fastWriter.write(body_res["person"]["data"][i]["mongo_id"]);
      cout << "--------------- MONGO ID: " << jsString << endl;
      trackList.at(i)->mongo_id = jsString;
      //      obj_m.mongo_id = jsString;
      jsString = fastWriter.write(body_res["person"]["data"][i]["score"]);
      //      obj_m.score = stoi(jsString);
      jsString = fastWriter.write(body_res["person"]["data"][i]["type"]);
      //      obj_m.type = jsString;
      jsString = fastWriter.write(body_res["person"]["data"][i]["track_id"]);
      int track_id = stoi(jsString);
      cout << id_query << endl;
      trackList.at(i)->setTrackName(id_query);
    }
  }
}
