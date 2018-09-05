#ifndef CONNECT_CMS
#define CONNECT_CMS

#include <chrono>
#include <iostream>
#include <mutex>
#include <vector>

using namespace std;

void push_data_to_CMS(string face_id, string camera_id, string frametime,
                      string face_img, string speed, string location,
                      string sever_address, int mode, int locationX,
                      int site_member_id, int site_owner_id);
class CMSInfo {
public:
  std::string token;
  std::string siteId;
  std::string user;
  std::string password;
  std::vector<std::tuple<std::string, std::string>> listCam;

public:
  CMSInfo(std::string _user, std::string _password);
  void connectToCms();
  void getListCam();
};

#endif
