#include "FaceRecognition.hpp"
#include "Utils.hpp"

/**
 * @brief Detector::Detector Constructor
 * @param _detectType
 */

Detector::Detector() {

  caffe_detector = new CaffeDetector("model_v2", CaffeDetector::MODEL_V1);
  deserialize("dlibface_landmark/shape_predictor.dat") >> shapePredictor;
}

Detector::~Detector() {}
/**
 * @brief Detector::SetSingleLevelDetector
 * @param _detector
 */

std::vector<CaffeDetector::BoundingBox>
Detector::detectUsingCaffe(cv::Mat &img, double scale) {
  std::cout << "detectUsingCaffe" << std::endl;
  cv::resize(img, img, cv::Size(img.cols / scale, img.rows / scale));
  std::vector<CaffeDetector::BoundingBox> dets = this->caffe_detector->Detect(
      img, CaffeDetector::BGR, CaffeDetector::ORIENT_UP, 20, 0.8, 0.9, 0.9);
  return dets;
}

bool Detector::isFace(dlib::matrix<rgb_pixel> &img, dlib::rectangle &det) {
  auto shape = this->shapePredictor(img, det);
  // cout << "shape part : " << shape.num_parts() << endl;
  if (shape.num_parts() == 0)
    return false;
  else
    return true;
}
