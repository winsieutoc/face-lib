#ifndef DEFINE_H_
#define DEFINE_H_
#include <dlib/dnn.h>
#include <dlib/logger.h>
//#include <dlib/misc_api.h>
#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace dlib;

#define FILE_NOT_FOUND_ERROR_CODE 20

union FloatChar {
  float floatValue;
  unsigned char charArray[4];
};

//-----Define-CNN

//---------------CNN-4-DETECT----------------------------------------------------
// —------------------------------------------------------------------------------------—
// A 5x5 conv layer that does 2x downsampling
template <long num_filters, typename SUBNET>
using con5d = con<num_filters, 5, 5, 2, 2, SUBNET>;
// A 5x5 conv layer that doesn't do any downsampling
template <long num_filters, typename SUBNET>
using con5 = con<num_filters, 5, 5, 1, 1, SUBNET>;
// For testing net
template <typename SUBNET>
using downsampler = relu<affine<
    con5d<32, relu<affine<con5d<32, relu<affine<con5d<16, SUBNET>>>>>>>>>;
template <typename SUBNET>
using downsampler_tr = relu<bn_con<
    con5d<32, relu<bn_con<con5d<32, relu<bn_con<con5d<16, SUBNET>>>>>>>>>;
template <typename SUBNET> using rcon5 = relu<affine<con5<45, SUBNET>>>;
template <typename SUBNET> using rcon5_tr = relu<bn_con<con5<45, SUBNET>>>;
using cnnnet_type = loss_mmod<
    con<1, 9, 9, 1, 1, rcon5<rcon5<rcon5<downsampler<
                           input_rgb_image_pyramid<pyramid_down<6>>>>>>>>;
using cnnnet_type_tr = loss_mmod<
    con<1, 9, 9, 1, 1, rcon5_tr<rcon5_tr<rcon5_tr<downsampler_tr<
                           input_rgb_image_pyramid<pyramid_down<6>>>>>>>>;

/**/

template <template <int, template <typename> class, int, typename> class block,
          int N, template <typename> class BN, typename SUBNET>
using residual = add_prev1<block<N, BN, 1, tag1<SUBNET>>>;

template <template <int, template <typename> class, int, typename> class block,
          int N, template <typename> class BN, typename SUBNET>
using residual_down =
    add_prev2<avg_pool<2, 2, 2, 2, skip1<tag2<block<N, BN, 2, tag1<SUBNET>>>>>>;

template <int N, template <typename> class BN, int stride, typename SUBNET>
using block =
    BN<con<N, 3, 3, 1, 1, relu<BN<con<N, 3, 3, stride, stride, SUBNET>>>>>;

template <int N, typename SUBNET>
using res = relu<residual<block, N, bn_con, SUBNET>>;
template <int N, typename SUBNET>
using ares = relu<residual<block, N, affine, SUBNET>>;
template <int N, typename SUBNET>
using res_down = relu<residual_down<block, N, bn_con, SUBNET>>;
template <int N, typename SUBNET>
using ares_down = relu<residual_down<block, N, affine, SUBNET>>;

// ----------------------------------------------------------------------------------------

template <typename SUBNET> using level0 = res_down<256, SUBNET>;
template <typename SUBNET>
using level1 = res<256, res<256, res_down<256, SUBNET>>>;
template <typename SUBNET>
using level2 = res<128, res<128, res_down<128, SUBNET>>>;
template <typename SUBNET>
using level3 = res<64, res<64, res<64, res_down<64, SUBNET>>>>;
template <typename SUBNET> using level4 = res<32, res<32, res<32, SUBNET>>>;

template <typename SUBNET> using alevel0 = ares_down<256, SUBNET>;
template <typename SUBNET>
using alevel1 = ares<256, ares<256, ares_down<256, SUBNET>>>;
template <typename SUBNET>
using alevel2 = ares<128, ares<128, ares_down<128, SUBNET>>>;
template <typename SUBNET>
using alevel3 = ares<64, ares<64, ares<64, ares_down<64, SUBNET>>>>;
template <typename SUBNET> using alevel4 = ares<32, ares<32, ares<32, SUBNET>>>;

// training network type
using net_type = loss_metric<fc_no_bias<
    128,
    avg_pool_everything<level0<level1<level2<level3<level4<max_pool<
        3, 3, 2, 2, relu<bn_con<con<32, 7, 7, 2, 2, input_rgb_image>>>>>>>>>>>>;

// testing network type (replaced batch normalization with fixed affine
// transforms)
using anet_type = loss_metric<fc_no_bias<
    128,
    avg_pool_everything<alevel0<alevel1<alevel2<alevel3<alevel4<max_pool<
        3, 3, 2, 2, relu<affine<con<32, 7, 7, 2, 2, input_rgb_image>>>>>>>>>>>>;
/**/

/// \brief The DETECT_TYPE enum

enum class DETECT_TYPE { CNN = 0, CASCADE, HOG, CAFFE };

enum class MODE { NORMAL, ARLERT };

struct FrameData {
  long long frame_id;
  cv::Mat frame;
  std::string frame_time;
  std::string cam_id;
};

class FaceInfo {
public:
  std::string faceId;
  cv::Rect rect;
  matrix<rgb_pixel> faceImg;
  full_object_detection shape;

  matrix<float, 0, 1> discriptor;
  std::string pose;

  std::string
      direction_h; // store text left or right or empty for straight face
  std::string direction_v; // store text up or down or empty for straight face
  int angle_h;             // The corresponding angle
  int angle_v;

public:
  FaceInfo() {}
  ~FaceInfo() {}

  bool Calculate_Angle_Direction_Landmarkbased();

  bool Is_Looking_Straight();
  bool Is_Blurry();
};

typedef scan_fhog_pyramid<pyramid_down<6>> image_scanner_type;

/**
 * @brief The Face struct
 * Information for each vector 128
 */
struct FaceVector {
  int face_id;                          // faceID (number) of this vector
  dlib::matrix<float, 0, 1> descriptor; // vector 128
  std::string path_to_image;            // path to face-image
  int time_since_epoch;                 // time since 1/1/1970 in ms
  unsigned char selected; // a vector may be selected (<=30 each person), others
                          // for training
};

/**
 * @brief The Person struct
 * contains a number of face - vector 128
 */
struct Person {
  int face_id; // faceID (number) of this vector
  std::vector<std::shared_ptr<FaceVector>>
      face_vectors;   // face - vectors of this person
  int person_type_id; // eg. 0: employee; 1: customer
};

struct SendData {
  std::string faceId;
  std::string base64Img;
  std::string camId;
  std::string status;
};

struct CriminalInfo {
  std::string faceId; // faceID (number) of this vector
  std::string faceName;
  std::string url;
  dlib::matrix<float, 0, 1> vec128; // vector 128
};

class CriminalData {
public:
  CriminalData() { total = 0; }
  inline int getTotal() { return total; }
  inline std::string getFaceId(int i) { return D.at(i).faceId; }
  inline std::string getFaceName(int i) { return D.at(i).faceName; }
  inline std::string getUrl(int i) { return D.at(i).url; }
  inline dlib::matrix<float, 0, 1> get128Vec(int i) { return D.at(i).vec128; }
  void addData(CriminalInfo info) {
    //    std::cout << "add data" << std::endl;
    total++;
    D.push_back(info);
    //    std::cout << total << std::endl;
    //    for (int i = 0; i < 128; ++i)
    //      std::cout << D.at(total - 1).vec128(i) << "  ";
  }

private:
  int total;
  std::vector<CriminalInfo> D;
};
#endif
