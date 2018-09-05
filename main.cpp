

#include <opencv2/core/core.hpp>

#include <opencv2/flann/flann.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <curl/curl.h>
#include <exception>
#include <iostream>
#include <string>

#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/posix_time/posix_time_types.hpp"
#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>

#include "FaceRecognition.hpp"
#include "SocketIoUtils.hpp"
#include "VideoReader.hpp"

using namespace dlib;
using namespace cv;
using namespace std;
using std::cerr;
namespace po = boost::program_options;

std::queue<FrameData> frameQueue; // Frames from Video Streaming
std::mutex frameQueueMutex;

int main(int argc, char *argv[]) {
  VideoReader VideoReader("/home/phuclb/Documents/camgocrong3.mp4", 1920, 1080,
                          cv::Rect(0, 0, 1920, 1080), frameQueue);
  FaceRecognition FaceRecognition(cv::Rect(50, 50, 1500, 800), frameQueue,
                                  frameQueueMutex);

  std::vector<std::thread> ths;
  ths.push_back(std::thread(VideoReader));
  ths.push_back(std::thread(FaceRecognition));

  for (auto &t : ths)
    t.join();

  return 0;
}
