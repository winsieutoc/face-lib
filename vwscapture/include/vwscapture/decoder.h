
#ifndef WS_DECODER_H
#define WS_DECODER_H

#include "cvBufferCapture.h"
#include "util.h"
#include "wsconnection.h"
#include <cstdlib>
#include <deque>
#include <iostream>
#include <map>
#include <sstream>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <vector>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/pixdesc.h>
#include <libswscale/swscale.h>
}

#ifdef OUTPUT_RGB
typedef AVFrame *img_format;
#else
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
typedef cv::Mat img_format;
#endif

#define MB_NAL_TYPE 0x1F //|00011111|
#define MB_FU_S 0x80
#define MB_FU_E 0x40
#define MB_FU_R 0x20

#define HEADER_LENGTH 12
#define OFSET_TIMESTAMP 1262304000000 // 40 years

using namespace cv;
using namespace std;

typedef long timestampType;

struct frame_packet {
  long index;
  img_format frame;
  timestampType timestamp;
};

class Decoder {
public:
  Decoder(std::queue<download_info> &input, std::queue<frame_packet> &output);
  std::thread spawn() {
    return std::thread([this] { this->run(); });
  }
  frame_packet getFrame();

private:
  std::queue<download_info> &m_downloadedQueue;
  std::queue<frame_packet> &m_decodedFrameQueue;
  std::mutex m_mtx;

  void run();
  void SetRunningStt(bool stt);
  void initializeDecoder();
  void decodeRTPPackage(std::vector<char> rtpPackage);
  void splitData(std::vector<char> rawData, std::string pkt_id,
                 std::string pkt_type);
  void decodeProcess();
  void splitMainRtpPackage(std::vector<char> rtpPackage);
  void checkingFUApackage(std::vector<char> h264Raw);

  bool isRunning();
  void appendFrame(frame_packet item);
  download_info getDownloadedPackage();

  struct frame_packet m_frame;
  bool bRunning;
  bool m_flush = false;
  long m_index = 0;
  string m_packageId;

  CvCapture_FFMPEG cap;
  std::vector<char> IDRPicture;
  std::vector<char> NALUStartCode = {0, 0, 0, 1};
  std::vector<char> IDR_NAL_TYPE = {101}; // 0x65
  bool IDRPictureStarted = false;
  bool checkIFrameFirst = false;
  timestampType curTimeStamp;
};

#endif // WS_DECODER_H
