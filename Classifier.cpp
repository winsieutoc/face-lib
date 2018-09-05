#include "FaceRecognition.hpp"
#include "Utils.hpp"
#include "curl/curl.h"
#include "jsoncpp/json/json.h"

// Classifier::Classifier(DETECT_TYPE detectType, string placeId, string
// _company,
//                       float _netThreshold) {
//  this->placeId = placeId;
//  this->detector = new Detector(detectType);
//  this->company = _company;
//  this->netThreshold = _netThreshold;
//  deserialize("dlibface_landmark/dlib_face_recognition_resnet_model_v1.dat")
//  >>
//      net;
//  personDatas = std::make_shared<PersonData>();
//  //  PersonInfo info;
//  //  info.faceId = "0";
//  //  info.faceName = "dummy";
//  //  info.url = "dummy";
//  //  dlib::matrix<float, 128, 1> a;
//  //  for (int i = 0; i < 128; ++i)
//  //    a(i) = 0;
//  //  info.vec128 = a;
//  //  personDatas->addData(info);
//  //  this->readFromFolder();
//  readFromDatabase();
//}
Classifier::Classifier() {

  this->detector = new Detector();
  deserialize("dlibface_landmark/dlib_face_recognition_resnet_model_v1.dat") >>
      net;
}

Classifier::~Classifier() {
  delete detector;
  personDatas.reset();
}
void Classifier::readFromFolder() {

  personDatas->clearData();

  boost::filesystem::path _path("");
  //  dlogc << LDEBUG << "[Classifier] Folder path : " << this->getFolderPath();
  BOOST_FOREACH (const boost::filesystem::path &personFolder,
                 std::make_pair(boost::filesystem::directory_iterator(_path),
                                boost::filesystem::directory_iterator())) {
    if (boost::filesystem::is_directory(personFolder)) {
      std::string _face = personFolder.filename().string();

      BOOST_FOREACH (
          const boost::filesystem::path &personFile,
          std::make_pair(boost::filesystem::directory_iterator(personFolder),
                         boost::filesystem::directory_iterator())) {
        if (boost::filesystem::is_directory(personFile)) {
          continue;
        }
        if (!boost::filesystem::is_regular_file(personFile)) {
          continue;
        } else if (personFile.extension().string() == ".jpg") {
          dlib::matrix<rgb_pixel> img;
          dlib::load_image(img, personFile.string());

          PersonInfo info;
          info.faceId = _face;
          info.faceName = _face;
          info.vec128 = this->net(img);
          info.url = personFile.string();
          personDatas->addData(info);
        }
      }
    }
  }
}

void Classifier::addNewProfile(std::string _id, std::string _name,
                               std::string _gender, std::string _age,
                               std::string _url, std::string _vec128) {
  cout << "loadNewProfile" << endl;
  cout << _id << endl;
  cout << _vec128 << endl;
  cout << _url << endl;

  PersonInfo info;

  info.faceId = _id;
  info.faceName = _name;
  info.url = _url;
  info.faceAge = _age;
  info.faceGender = _gender;

  std::stringstream ss(_vec128);
  std::istream_iterator<std::string> begin(ss);
  std::istream_iterator<std::string> end;
  std::vector<std::string> vstrings(begin, end);
  int i = 0;
  dlib::matrix<float, 128, 1> newVec;

  for (std::string a : vstrings) {
    float tmp;
    stringstream ss;
    ss << a;
    ss >> tmp;
    ss.clear();
    newVec(i) = tmp;
    i++;
  }
  info.vec128 = newVec;

  this->personDatas->addData(info);
}

void Classifier::deleteProfile(string id) { this->personDatas->deleteById(id); }

std::vector<string>
Classifier::findCloserPerson(matrix<float, 0, 1> _faceDescriptors) {
  //  cout << "input" << endl;
  //  for (int j = 0; j < 128; ++j)
  //    cout << _faceDescriptors(j);
  //  cout << endl;
  float minlength1 = 100;
  float minlength2 = 100;
  float minlength3 = 100;

  string minfaceID1 = "";
  string minfaceID2 = "";
  string minfaceID3 = "";
  std::vector<string> returnFaceId;
  for (int i = 0; i < personDatas->getTotal(); ++i) {
    //    cout << "compare" << endl;
    //    for (int j = 0; j < 128; ++j)
    //      cout << personDatas->get128Vec(i)(j);
    //    cout << endl;
    float lng = dlib::length(personDatas->get128Vec(i) - _faceDescriptors);
    //    cout << lng << endl;
    if (lng < 0.31) { // 0.40 for vp9_facenet1
      if (minlength1 >= lng) {
        minlength3 = minlength2;
        minfaceID3 = minfaceID2;

        minlength2 = minlength1;
        minfaceID2 = minfaceID1;

        minlength1 = lng;
        minfaceID1 = personDatas->getFaceId(i);
      } else if (minlength2 > lng) {
        minlength3 = minlength2;
        minfaceID3 = minfaceID2;

        minlength2 = lng;
        minfaceID2 = personDatas->getFaceId(i);

      } else if (minlength3 > lng) {
        minlength3 = lng;
        minfaceID3 = personDatas->getFaceId(i);
      }
    }
  }
  returnFaceId.push_back(minfaceID1);
  returnFaceId.push_back(minfaceID2);
  returnFaceId.push_back(minfaceID3);
  //  cout << "Faceid = " << minfaceID1 << endl;
  return returnFaceId;
}
namespace {
std::size_t callback(const char *in, std::size_t size, std::size_t num,
                     std::string *out) {
  const std::size_t totalBytes(size * num);
  out->append(in, totalBytes);
  return totalBytes;
}
}
void Classifier::readFromDatabase() {
  bool debug = false;

  personDatas->clearData();
  CURL *curl2 = curl_easy_init();
  string url = "http://api.hrl.vp9.vn/api/members?limit=100";

  struct curl_slist *chunk = NULL;

  std::string header = "Company:" + this->company;
  chunk = curl_slist_append(chunk, header.c_str());

  /* set our custom set of headers */
  curl_easy_setopt(curl2, CURLOPT_HTTPHEADER, chunk);

  curl_easy_setopt(curl2, CURLOPT_URL, url.c_str());

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
    if (debug)
      std::cout << "\nGot successful response from " << url << std::endl;
    Json::Value jsonData;
    Json::Reader jsonReader;

    if (jsonReader.parse(*httpData.get(), jsonData)) {
      if (debug) {
        std::cout << "Successfully parsed JSON data" << std::endl;
        std::cout << "\nJSON data received:" << std::endl;
      }

      std::map<string, matrix<float, 0, 1>> gallery_face_descriptor;
      for (auto row : jsonData["rows"]) {

        std::string id = row["id"].asString();
        std::string name = row["name"].asString();
        std::string gender = row["gender"].asString();
        std::string age = row["age"].asString();
        if (debug)
          cout << "faceid: " << id << endl;
        for (auto itr : row["photos"]) {
          string src = itr["src"].asString();
          //          string vecto = itr["vector"].asString();
          auto vecto = itr["vector"];
          //          std::stringstream ss(vecto);
          //          std::vector<std::string> vstrings;
          //          while (ss.good()) {
          //            string substr;
          //            getline(ss, substr, ',');
          //            vstrings.push_back(substr);
          //          }

          dlib::matrix<float, 128, 1> newVec;
          //          cout << "load" << endl;
          //          cout << vstrings.size() << endl;
          for (int i = 0; i < 128; ++i) {
            //            float tmp;
            //            stringstream ss;
            //            ss << vstrings[i];
            //            ss >> tmp;
            //            ss.clear();
            newVec(i) = vecto.get(i, 0).asFloat();
            //            cout << newVec(i) << " ";
          }
          //          cout << endl;
          PersonInfo info;
          info.faceId = id;
          info.faceName = name;
          info.vec128 = newVec;
          info.url = src;
          info.faceAge = age;
          info.faceGender = gender;
          this->personDatas->addData(info);
        }
      }
      httpData->clear();
    } else {
      if (debug) {
        std::cout << "Could not parse HTTP data as JSON" << std::endl;
        std::cout << "HTTP data was:\n" << *httpData.get() << std::endl;
      }
      return;
    }
  }
}
