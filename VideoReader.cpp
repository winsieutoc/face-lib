#include "VideoReader.hpp"
#include <cmath>
#include <iostream>
#include <mutex>
#include <queue>
#include <stack>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <thread>
#include <unistd.h>

using namespace cv;
using std::queue;
VideoReader::VideoReader(std::string url, int width, int height, Rect cropRect,
                         std::queue<FrameData> &queue_)
    : frameQueue(queue_) {
  this->url = url;
  this->width = width;
  this->height = height;
}

void videoreader_http(std::string url, int width, int height,
                      std::queue<FrameData> &frameQueue) {
  VideoCapture cap;
  int nframe = 0;
  while (1) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    std::string urlsearchip = url;
    std::cout << urlsearchip << std::endl;
    cap.open(urlsearchip);
    int delay = 1000 / 25;
    if (cap.isOpened()) {
      long long id = 0;
      while (1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        //        auto startime = CLOCK_NOW();
        id++;
        FrameData this_frame;
        cap >> this_frame.frame;

        if (this_frame.frame.data) {
          if (id % 2 == 0) {
            resize(this_frame.frame, this_frame.frame, cv::Size(width, height));
            this_frame.frame_id = id;
            this_frame.cam_id = "2";
            //            thisFrame.frameTime =
            //            objects.getCurrentDateTime_pushSQL();
            frameQueue.push(this_frame);
          }
        } else
          break;

        nframe++;
      }
    } else {
      std::cout << "\nLost connect!";
      std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    }
  }
}

void VideoReader::operator()() {
  videoreader_http(url, width, height, frameQueue);
}
