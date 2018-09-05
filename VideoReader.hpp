#ifndef VIDEOREADER_HPP
#define VIDEOREADER_HPP

#include <chrono>
#include <mutex>
#include <queue>
#include <stack>
#include <string>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <Define.hpp>

class VideoReader {
public:
  VideoReader(std::string url, int width, int height, cv::Rect cropRect,
              std::queue<FrameData> &queue_);
  void operator()();

private:
  std::string url;
  int width;
  int height;
  std::queue<FrameData> &frameQueue;
};

#endif // VIDEOREADER_HPP
