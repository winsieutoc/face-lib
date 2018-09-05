#include <chrono>
#include <cstring>
#include <ctime>
#include <ctime>
#include <fstream>
#include <iostream>
#include <queue>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <thread>

#include "connectCMS.h"
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>
#include <cryptopp/sha.h>
#include <curl/curl.h>
#include <jsoncpp/json/json.h>

#define UT_FUNCTION
#define URL_DEFAULT "http://demo.stmc.vp9.vn:8000/plateInfo"

using namespace std;

void postDataToCms(string face_id, string camera_id, string frametime,
                   string face_img, string face_img2, string speed,
                   string location, string sever_address, int mode,
                   int locationX, int site_member_id, int site_owner_id) {
  cout << "post data to CMS : " << face_id << endl;
  CURLcode ret;
  CURL *hnd;
  struct curl_slist *slist1;
  std::time_t timeGetPlate = std::time(nullptr);
  timeGetPlate = timeGetPlate + 60 * 60 * 7;

  std::string jsonstr = " {\"vehicle_plate\":\" ";
  jsonstr.append(face_id);
  jsonstr.append("\",\"camera_id\":\" ");
  jsonstr.append(camera_id);
  jsonstr.append("\",\"frametime\":\" ");
  jsonstr.append(frametime);
  jsonstr.append("\",\"camera_id\":\" ");
  jsonstr.append(camera_id);
  jsonstr.append("\",\"encoded_plate_image\":\" ");
  jsonstr.append(face_img);
  jsonstr.append("\",\"encoded_vehicle_image\":\" ");
  jsonstr.append(face_img2);
  jsonstr.append("\",\"speed\":\" ");
  jsonstr.append(speed);
  jsonstr.append("\",\"location\":\" ");
  jsonstr.append(location);
  jsonstr.append("\",\"mode\":\" ");
  jsonstr.append(std::to_string(mode));
  jsonstr.append("\",\"locationX\":\" ");
  jsonstr.append(std::to_string(locationX));
  jsonstr.append("\",\"site_member_id\":\" ");
  jsonstr.append(std::to_string(site_member_id));
  jsonstr.append("\",\"site_owner_id\":\" ");
  jsonstr.append(std::to_string(site_owner_id));
  jsonstr.append(" \" }");

  slist1 = NULL;
  slist1 = curl_slist_append(slist1, "Content-Type: application/json");

  hnd = curl_easy_init();
  char url[1000];
  strcpy(url, sever_address.c_str());
  cout << "url : " << url << endl;
  ;
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
  cout << endl;
  //  delete hnd;
}
void push_data_to_CMS(string face_id, string camera_id, string frametime,
                      string face_img, string speed, string location,
                      string sever_address, int mode, int locationX,
                      int site_member_id, int site_owner_id) {

  postDataToCms(face_id, camera_id, frametime, face_img, face_img, speed,
                location, sever_address, mode, locationX, site_member_id,
                site_owner_id);
}

CMSInfo::CMSInfo(std::string _user, std::string _password) {
  user = _user;
  password = _password;
}
namespace {
std::size_t callback(const char *in, std::size_t size, std::size_t num,
                     std::string *out) {
  const std::size_t totalBytes(size * num);
  out->append(in, totalBytes);
  return totalBytes;
}
}

void CMSInfo::connectToCms() {
  CryptoPP::SHA256 hash;
  byte digest[CryptoPP::SHA256::DIGESTSIZE];
  hash.CalculateDigest(digest, (byte *)password.c_str(), password.length());

  CryptoPP::HexEncoder encoder;
  std::string output;
  encoder.Attach(new CryptoPP::StringSink(output));
  encoder.Put(digest, sizeof(digest));
  encoder.MessageEnd();

  //  std::cout << "--------" << output << std::endl;
  CURL *curl2 = curl_easy_init();
  std::string URL = "https://core.cam9.tv/api/"
                    "login?action=login&password=" +
                    output + "&username=" + user;
  curl_easy_setopt(curl2, CURLOPT_URL, URL.c_str());

  // Don't bother trying IPv6, which would increase DNS resolution time.
  curl_easy_setopt(curl2, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);

  // Don't wait forever, time out after 10 seconds.
  curl_easy_setopt(curl2, CURLOPT_TIMEOUT, 10);

  // Follow HTTP redirects if necessary.
  curl_easy_setopt(curl2, CURLOPT_FOLLOWLOCATION, 1L);

  // Response information.
  int httpCode(0);
  std::unique_ptr<std::string> httpData(new std::string());

  // Hook up data handling function.
  curl_easy_setopt(curl2, CURLOPT_WRITEFUNCTION, callback);

  // Hook up data container (will be passed as the last parameter to the
  // callback handling function).  Can be any pointer type, since it will
  // internally be passed as a void pointer.
  curl_easy_setopt(curl2, CURLOPT_WRITEDATA, httpData.get());

  // Run our HTTP GET command, capture the HTTP response code, and clean up.
  curl_easy_perform(curl2);
  curl_easy_getinfo(curl2, CURLINFO_RESPONSE_CODE, &httpCode);
  curl_easy_cleanup(curl2);
  if (httpCode == 200) {
    std::cout << "\nGot successful response from " << URL << std::endl;
    Json::Value jsonData;
    Json::Reader jsonReader;

    if (jsonReader.parse(*httpData.get(), jsonData)) {
      std::cout << "Successfully parsed JSON data" << std::endl;
      std::cout << "\nJSON data received:" << std::endl;
      //      std::cout << jsonData.toStyledString() << std::endl;
      token = jsonData["token"].asString();
      siteId = jsonData["site"]["site_id"].asString();
      httpData->clear();
    } else {
      std::cout << "Could not parse HTTP data as JSON" << std::endl;
      std::cout << "HTTP data was:\n" << *httpData.get() << std::endl;
      return;
    }
  }
}

void CMSInfo::getListCam() {
  CURL *curl2 = curl_easy_init();
  std::string URL = "https://core.cam9.tv/api/cms_api/getCamera?site_id=" +
                    siteId + "&version=004&token=" + token + "&p=1&c=12";
  curl_easy_setopt(curl2, CURLOPT_URL, URL.c_str());

  // Don't bother trying IPv6, which would increase DNS resolution time.
  curl_easy_setopt(curl2, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);

  // Don't wait forever, time out after 10 seconds.
  curl_easy_setopt(curl2, CURLOPT_TIMEOUT, 10);

  // Follow HTTP redirects if necessary.
  curl_easy_setopt(curl2, CURLOPT_FOLLOWLOCATION, 1L);

  // Response information.
  int httpCode(0);
  std::unique_ptr<std::string> httpData(new std::string());

  // Hook up data handling function.
  curl_easy_setopt(curl2, CURLOPT_WRITEFUNCTION, callback);

  // Hook up data container (will be passed as the last parameter to the
  // callback handling function).  Can be any pointer type, since it will
  // internally be passed as a void pointer.
  curl_easy_setopt(curl2, CURLOPT_WRITEDATA, httpData.get());

  // Run our HTTP GET command, capture the HTTP response code, and clean up.
  curl_easy_perform(curl2);
  curl_easy_getinfo(curl2, CURLINFO_RESPONSE_CODE, &httpCode);
  curl_easy_cleanup(curl2);
  if (httpCode == 200) {
    std::cout << "\nGot successful response from " << URL << std::endl;
    Json::Value jsonData;
    Json::Reader jsonReader;

    if (jsonReader.parse(*httpData.get(), jsonData)) {
      for (auto item : jsonData["CAMS_LIST"]) {
        std::string camId = item["camera_id"].asString();
        for (auto it1 : item["profile"]) {
          std::string sourceName = it1["name"].asString();
          if (sourceName.find("HD") != std::string::npos) {
            for (auto it2 : it1["streams"]) {
              std::string wsUrl = it2["source"].asString();
              if (wsUrl.find("ws://cdn.vp9.cam9.vn/evup/") != std::string::npos)
                listCam.push_back(std::make_tuple(camId, wsUrl));
            }
          }
        }
      }
      httpData->clear();
    } else {
      std::cout << "Could not parse HTTP data as JSON" << std::endl;
      std::cout << "HTTP data was:\n" << *httpData.get() << std::endl;
      return;
    }
  }
}
