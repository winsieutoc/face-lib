#include "VP9Wrapper.hpp"
#define SHOW false
/**
 * @brief VP9Wrapper::VP9Wrapper
 */
VP9Wrapper::VP9Wrapper(
    std::vector<std::tuple<std::string, std::string, cv::Rect>> _listCam,
    std::string _placeId, std::string _company) {
  listCam = _listCam;
  placeId = _placeId;
  company = _company;
  //  cout << listCam.size() << endl;
  std::string cam_00 = "rtsp://192.168.10.86:554/av0_0";
  std::string cam_000 = "rtsp://10.12.11.111:554/av0_0";
  std::string cam_01 = "rtsp://192.168.10.18:554/av0_0";
  std::string video = "new.mp4";
  std::string cam_02 = "rtsp://admin:1111@192.168.10.60/av0_0";
  std::string cam_03 = "rtsp://192.168.10.57:554/av0_0"; // Cau thang may
  std::string cam_04 = "rtsp://192.168.10.42:554/av0_0"; // Cau thang may
  std::string cam_05 = "rtsp://192.168.10.35:554/av0_0"; // Bep

  std::string demnguoi_tm1 = "rtsp://192.168.10.37:554/av0_0";
  std::string demnguoi_tm2 = "rtsp://192.168.10.38:554/av0_0";
  std::string demnguoi_tm3 = "rtsp://192.168.10.76:554/av0_0";
  std::string demnguoi_tm4 = "rtsp://192.168.10.74:554/av0_0";
  std::string demnguoi_tm5 = "rtsp://192.168.10.75:554/av0_0";
  std::string demnguoi_pm3 = "rtsp://192.168.10.77:554/av0_0";
  std::string demnguoi_pm1 = "rtsp://192.168.10.78:554/av0_0";

  std::string viettel_1 = "rtsp://10.10.10.181/stream1";
  std::string viettel_2 = "rtsp://10.10.10.182/stream1";
  std::string viettel_3 = "rtsp://10.10.10.93";

  std::string ws_chamcong_1 =
      "ws://2c1.cam9.tv/evup/2132_1524732483/005a205289e4xyz17856";
  std::string ws_chamcong_2 =
      "ws://2c1.cam9.tv/evup/2132_1524732483/1082171a777cxyz17852";
  std::string ws_demnguoi_tm3 = "rtsp://192.168.10.39:554/av0_0";
  std::string ws_demnguoi_tm4 =
      "ws://2c1.cam9.tv/evup/2132_1524732483/005a205289e4xyz17856";
  std::string ws_demnguoi_tm5 = "rtsp://192.168.10.75:554/av0_0";
  string cam01 = "ws://cdn.vp9.cam9.vn/evup/4ccc6afc4992/d02212d8c4efxyz14921";
  string cam02 = "ws://cdn.vp9.cam9.vn/evup/7085c23be18a/005a20528b07xyz16012";
  string cam03 = "ws://cdn.vp9.cam9.vn/evup/a81702bcca04/005a20528c73xyz15120";
  string cam04 = "ws://cdn.vp9.cam9.vn/evup/4ccc6afc4992/005a20528ad8xyz14353";
  string cam05 = "ws://cdn.vp9.cam9.vn/evup/a81702bcca04/d02212d8c6bdxyz14907";

  for (int i = 0; i < listCam.size(); ++i) {
    engineVec.push_back(
        new FaceRecognition(std::get<0>(listCam[i]), std::get<1>(listCam[i]),
                            _placeId, _company, std::get<2>(listCam[i]), true));
  }
}

/**
 * @brief VP9Wrapper::run
 * @param syncInfoForCanvas
 */
void VP9Wrapper::run(std::function<void(std::string json)> syncInfoForCanvas) {
  //  vp9Face->processWSVideoCanvas(syncInfoForCanvas, true);
}

void VP9Wrapper::run(std::function<void(std::string jsonData)> SendDataDelegate,
                     int type) {
  std::vector<std::thread> threadPool;

  for (int i = 0; i < engineVec.size(); ++i) {
    //    auto f = [&](int index) {
    //      std::this_thread::sleep_for(chrono::milliseconds(5000));
    //      engineVec[index]->processTestVideo(SendDataDelegate, SHOW);
    //    }(i);
    threadPool.push_back(
        std::thread(&VP9Wrapper::process, this, SendDataDelegate, i));
  }
  for (auto &t : threadPool)
    t.join();
}
/**
 * @brief VP9Wrapper::run
 */

void VP9Wrapper::process(
    std::function<void(std::string jsonData)> SendDataDelegate, int index) {
  std::this_thread::sleep_for(chrono::milliseconds(5000));
  engineVec[index]->processTestVideo(SendDataDelegate, SHOW);
}

/**
 * @brief VP9Wrapper::OnGetRequestNewProfile
 * @param name
 * @param data
 * @param isAck
 * @param ack_resp
 */
void VP9Wrapper::OnGetRequestNewProfile(string const &name,
                                        message::ptr const &data, bool isAck,
                                        message::list &ack_resp) {
  cout << "OnGetRequestNewProfile" << endl;
  if (data->get_flag() == message::flag_string) {
    cout << data->get_string() << endl;
    Json::Value jsonData;
    Json::Reader reader;
    bool parsingSuccessful =
        reader.parse(data->get_string().c_str(), jsonData); // parse process
    if (!parsingSuccessful) {
      std::cout << "Failed to parse" << reader.getFormattedErrorMessages();
    }

    std::string id = jsonData["id"].asString();
    std::string name = jsonData["name"].asString();
    std::string gender = jsonData["gender"].asString();
    std::string age = jsonData["age"].asString();

    for (auto itr : jsonData["photos"]) {
      PersonInfo info;

      string src = itr["src"].asString();
      auto vecto = itr["vector"];

      dlib::matrix<float, 128, 1> newVec;

      for (int i = 0; i < 128; ++i)
        newVec(i) = vecto.get(i, 0).asFloat();

      info.faceId = id;
      info.faceName = name;
      info.vec128 = newVec;
      info.url = src;
      info.faceAge = age;
      info.faceGender = gender;
      for (int i = 0; i < engineVec.size(); ++i)
        engineVec[i]->getClassifier()->personDatas->addData(info);
    }
  }
}

/**
 * @brief VP9Wrapper::OnGetRequestDeleteProfile
 * @param name
 * @param data
 * @param isAck
 * @param ack_resp
 */
void VP9Wrapper::OnGetRequestDeleteProfile(string const &name,
                                           message::ptr const &data, bool isAck,
                                           message::list &ack_resp) {
  cout << "OnGetRequestDeleteProfile" << endl;
  if (data->get_flag() == message::flag_string) {
    cout << data->get_string() << endl;
    Json::Value jsonData;
    Json::Reader reader;
    bool parsingSuccessful =
        reader.parse(data->get_string().c_str(), jsonData); // parse process
    if (!parsingSuccessful) {
      std::cout << "Failed to parse" << reader.getFormattedErrorMessages();
    }

    std::string id = jsonData["id"].asString();
    for (int i = 0; i < engineVec.size(); ++i)
      engineVec[i]->getClassifier()->deleteProfile(id);
  }
}

/**
 * @brief VP9Wrapper::OnGetRequestEditProfile
 * @param name
 * @param data
 * @param isAck
 * @param ack_resp
 */
void VP9Wrapper::OnGetRequestEditProfile(string const &name,
                                         message::ptr const &data, bool isAck,
                                         message::list &ack_resp) {
  cout << "OnGetRequestEditProfile" << endl;
  if (data->get_flag() == message::flag_string) {
    cout << data->get_string() << endl;
    Json::Value jsonData;
    Json::Reader reader;
    bool parsingSuccessful =
        reader.parse(data->get_string().c_str(), jsonData); // parse process
    if (!parsingSuccessful) {
      std::cout << "Failed to parse" << reader.getFormattedErrorMessages();
    }

    std::string id = jsonData["id"].asString();
    std::string name = jsonData["name"].asString();
    std::string gender = jsonData["gender"].asString();
    std::string age = jsonData["age"].asString();
    for (int i = 0; i < engineVec.size(); ++i)
      engineVec[i]->getClassifier()->deleteProfile(id);
    for (auto itr : jsonData["photos"]) {
      PersonInfo info;

      string src = itr["src"].asString();
      auto vecto = itr["vector"];

      dlib::matrix<float, 128, 1> newVec;

      for (int i = 0; i < 128; ++i)
        newVec(i) = vecto.get(i, 0).asFloat();

      info.faceId = id;
      info.faceName = name;
      info.vec128 = newVec;
      info.url = src;
      info.faceAge = age;
      info.faceGender = gender;
      for (int i = 0; i < engineVec.size(); ++i) {

        engineVec[i]->getClassifier()->personDatas->addData(info);
      }
    }
  }
}

/**
 * @brief VP9Wrapper::OnGetRequestAlertProfile
 * @param name
 * @param data
 * @param isAck
 * @param ack_resp
 */
void VP9Wrapper::OnGetRequestAlertProfile(string const &name,
                                          message::ptr const &data, bool isAck,
                                          message::list &ack_resp) {
  cout << "OnGetRequestEditProfile" << endl;
  if (data->get_flag() == message::flag_string) {
    cout << data->get_string() << endl;
    Json::Value jsonData;
    Json::Reader reader;
    bool parsingSuccessful =
        reader.parse(data->get_string().c_str(), jsonData); // parse process
    if (!parsingSuccessful) {
      std::cout << "Failed to parse" << reader.getFormattedErrorMessages();
    }

    //    std::string alertUrl = jsonData.get("photo", "").asCString();
    //    std::string alertId = jsonData.get("id", "").asCString();
    //    std::string alertName = jsonData.get("name", "").asCString();

    //    std::for_each(engineVec.begin(), engineVec.end(),
    //                  [&](FaceRecognition *ptr) {
    //                    ptr->updateProfile(alertId, alertName, alertUrl);
    //                    ptr->clearTrack();
    //                  });

    //    vp9Face->updateProfile(alertId, alertName, alertUrl);

    //    vp9Face->clearTrack();
  }
}
