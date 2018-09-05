#ifndef WS_CAPTURE_H
#define WS_CAPTURE_H

#pragma once

#include "vwscapture/decoder.h"
#include "vwscapture/wsconnection.h"

//#define WSInstance WSCapture::getInstance()
typedef frame_packet WSCaptureData;
class WSCapture {
public:
  WSCapture();
  //  static WSCapture &getInstance(void) {
  //    static WSCapture instance;
  //    return instance;
  //  }
  void open(string url);
  bool isOpened();
  WSCaptureData getNextData();
  WSCaptureData pop();
  WSCaptureData next() { return getNextData(); }
  void test();

private:
  std::queue<download_info> downloadedQueue;
  std::queue<frame_packet> decodedFrameQueue;

  WSConnection *m_wsconnection;
  Decoder *m_decoder;
  void run();
};

#endif // WS_CAPTURE_H
