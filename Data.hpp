#ifndef DATA_H
#define DATA_H
#include <boost/bind.hpp>
#include <dlib/data_io.h>
#include <dlib/dnn.h>
#include <dlib/gui_widgets.h>
#include <dlib/image_processing.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/opencv.h>
struct PersonInfo {
  std::string faceId; // faceID (number) of this vector
  std::string faceName;
  std::string faceAge;
  std::string faceGender;
  std::string url;
  dlib::matrix<float, 0, 1> vec128; // vector 128
};

class PersonData {
public:
  PersonData() { total = 0; }
  inline int getTotal() { return total; }
  inline std::string getFaceId(int i) { return D.at(i).faceId; }
  inline std::string getFaceName(int i) { return D.at(i).faceName; }
  inline std::string getFaceAge(int i) { return D.at(i).faceAge; }
  inline std::string getFaceGender(int i) { return D.at(i).faceGender; }
  inline std::string getUrl(int i) { return D.at(i).url; }
  inline dlib::matrix<float, 0, 1> get128Vec(int i) { return D.at(i).vec128; }

  void readFromFolder(std::string folder_path);
  void clearData() {
    //    std::cout << "clear data" << std::endl;

    D.clear();
    total = 0;
    //    mapIdName.clear();
  }
  void addData(PersonInfo info) {
    //    std::cout << "add data" << std::endl;
    total++;
    D.push_back(info);

    //    if (mapIdName.find(info.faceId) == mapIdName.end()) {
    //      mapIdName.insert(
    //          std::pair<std::string, std::string>(info.faceId,
    //          info.faceName));
    //    }
  }
  std::tuple<std::string, std::string, std::string>
  findInfoById(std::string id) {
    std::vector<PersonInfo>::iterator it = std::find_if(
        D.begin(), D.end(), boost::bind(&PersonInfo::faceId, _1) == id);
    return std::tuple<std::string, std::string, std::string>(
        it->faceName, it->faceGender, it->faceAge);
  }

  void deleteById(std::string id) {
    //    std::cout << "Size  : " << total << std::endl;
    D.erase(std::remove_if(D.begin(), D.end(),
                           [&](PersonInfo const &info) {
                             return info.faceId.compare(id) == 0;
                           }),
            D.end());

    total = D.size();
    //    mapIdName.erase(id);
    //    std::cout << "Size : " << total << std::endl;
  }

private:
  int total;
  std::vector<PersonInfo> D;
  //  std::map<std::string, std::string> mapIdName;

  //  personTuple mapInfo;
};

#endif // DATA_H
