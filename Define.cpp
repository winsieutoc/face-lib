#include "Define.hpp"
bool FaceInfo::Calculate_Angle_Direction_Landmarkbased() //, int& angle_h, int&
                                                         // angle_v, string&
// direction_h, string&
// direction_v)
{
  if (this->shape.num_parts() != 68)
    return false;
  // bool smiling;

  cv::Point pt_nosetop(shape.part(30)(0), shape.part(30)(1));
  cv::Point pt_noseup(shape.part(27)(0), shape.part(27)(1));
  cv::Point pt_earleft(shape.part(1)(0), shape.part(1)(1));
  cv::Point pt_earright(shape.part(15)(0), shape.part(15)(1));
  cv::Point pt_eyeleft(shape.part(39)(0), shape.part(39)(1));
  cv::Point pt_eyeright(shape.part(42)(0), shape.part(42)(1));

  cv::Point pt_noseleft(shape.part(31)(0), shape.part(31)(1));
  cv::Point pt_noseright(shape.part(35)(0), shape.part(35)(1));
  cv::Point pt_nosemiddle(shape.part(33)(0), shape.part(33)(1));

  cv::Point pt_liptop(shape.part(62)(0), shape.part(62)(1));
  cv::Point pt_lipbottom(shape.part(66)(0), shape.part(66)(1));

  cv::Point diff = pt_nosetop - pt_earleft;
  int earleft_to_nose = diff.x * diff.x + diff.y * diff.y;
  diff = pt_nosetop - pt_earright;
  int earright_to_nose = diff.x * diff.x + diff.y * diff.y;
  diff = pt_nosetop - pt_noseup;
  int noseup_to_nosetop = diff.x * diff.x + diff.y * diff.y;
  diff = pt_eyeleft - pt_eyeright;
  int eyeright_to_eyeleft = diff.x * diff.x + diff.y * diff.y;

  diff = pt_noseleft - pt_noseright;
  int noseright_to_noseleft = diff.x * diff.x + diff.y * diff.y;
  diff = pt_nosemiddle - pt_nosetop;
  int nosemiddle_to_nosetop = diff.x * diff.x + diff.y * diff.y;

  diff = pt_liptop - pt_lipbottom;
  // int liptop_to_lipbottom = diff.x*diff.x + diff.y*diff.y;

  // cout<<earright_to_nose<<" "<<earleft_to_nose<< " "<< (float)
  // earleft_to_nose / earright_to_nose<< endl;

  float threshold_0 = 0.6;
  float threshold_30 = 0.3;

  // Horizontal
  if ((float)earleft_to_nose / earright_to_nose > threshold_0 &&
      (float)earright_to_nose / earleft_to_nose > threshold_0) {
    this->direction_h = "";
    this->angle_h = 0;
  }
  if ((float)earleft_to_nose / earright_to_nose <= threshold_0 &&
      (float)earleft_to_nose / earright_to_nose >= threshold_30) {
    this->direction_h = "left";
    this->angle_h = 30;
  }
  if ((float)earleft_to_nose / earright_to_nose < threshold_30) {
    this->direction_h = "left";
    this->angle_h = 45;
  }
  if ((float)earright_to_nose / earleft_to_nose <= threshold_0 &&
      (float)earright_to_nose / earleft_to_nose >= threshold_30) {
    this->direction_h = "right";
    this->angle_h = 30;
  }
  if ((float)earright_to_nose / earleft_to_nose < threshold_30) {
    this->direction_h = "right";
    this->angle_h = 45;
  }

  // Vertial
  if ((float)noseup_to_nosetop / eyeright_to_eyeleft > threshold_0) {
    this->direction_v = "";
    this->angle_v = 0;
  }
  if ((float)noseup_to_nosetop / eyeright_to_eyeleft <= threshold_0) {
    this->direction_v = "up";
    this->angle_v = 30;
  }
  if ((float)nosemiddle_to_nosetop / noseright_to_noseleft <= 0.2) {
    this->direction_v = "down";
    this->angle_v = 30;
  }
  this->pose = direction_h;
  if (this->direction_h.empty() && this->direction_v.empty())
    this->pose = "straight";

  return true;
}

bool FaceInfo::Is_Looking_Straight() {
  if (this->direction_h.empty() && this->direction_v.empty())
    return true;
  if (this->direction_h.compare("right") == 0 && this->angle_h == 30)
    return true;
  if (this->direction_h.compare("left") == 0 && this->angle_h == 30)
    return true;
  else
    return false;
}

// bool FaceInfo::Is_Blurry() {
//  cv::Mat matImg = dlib::toMat(this->faceImg);
//  cv::Mat lapacianImg, imgGray;
//  cv::cvtColor(matImg, imgGray, CV_BGR2GRAY);

//  cv::Laplacian(imgGray, lapacianImg, CV_64F);
//  cv::Scalar mean, stddev; // 0:1st channel, 1:2nd channel and 2:3rd channel
//  cv::meanStdDev(lapacianImg, mean, stddev, cv::Mat());
//  double variance = stddev.val[0] * stddev.val[0];

//  double threshold = 250;
//  //  std::cout << "blurry :" << variance << std::endl;
//  if (variance <= threshold) {
//    return true;
//  } else {
//    return false;
//  }
//}
