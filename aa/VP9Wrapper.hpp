#ifndef VP9WRAPPER_HPP
#define VP9WRAPPER_HPP
#include "FaceRecognition.hpp"

#include <boost/program_options.hpp>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <unistd.h>

class VP9Wrapper {
private:
  std::vector<FaceRecognition *> engineVec;
  std::vector<std::tuple<std::string, std::string, cv::Rect>> listCam;
  std::string placeId;
  std::string company;
public:
  FaceRecognition *vp9Face;

public:
  VP9Wrapper(
      std::vector<std::tuple<std::string, std::string, cv::Rect>> listCam,
      std::string placeId, std::string company);
  void run(std::function<void(std::string json)> syncInfoForCanvas);
  void run(std::function<void(std::string json)> SendDataDelegate, int type);
  void process(std::function<void(std::string jsonData)> SendDataDelegate,
               int index);
  void OnGetRequestNewProfile(string const &name, message::ptr const &data,
                              bool isAck, message::list &ack_resp);
  void OnGetRequestDeleteProfile(string const &name, message::ptr const &data,
                                 bool isAck, message::list &ack_resp);
  void OnGetRequestEditProfile(string const &name, message::ptr const &data,
                               bool isAck, message::list &ack_resp);
  void OnGetRequestAlertProfile(string const &name, message::ptr const &data,
                                bool isAck, message::list &ack_resp);
};

#endif // VP9WRAPPER_HPP
