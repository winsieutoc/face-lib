
#ifndef WS_CONNECTION_H
#define WS_CONNECTION_H

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fstream>
#include <iostream>
#include <map>
#include <queue>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include <condition_variable>

#include "util.h"
#include "websocket.h"

struct ws_header_infor {
  string request;
  string host;
  string mac;
  string cam;
};
enum RTCSocketState {
  RTCSS_Disconnected,
  RTCSS_Connected,
  RTCSS_SentSignature,
  RTCSS_ReceivedSignature,
  RTCSS_ReceivedInfomation,
  RTCSS_ReceivedDownloadLink,
  RTCSS_ReceivedGOP
};
struct ws_message {
  string id;
  int size = 0;
  int num;
  string type;
  bool empty() { return size == 0; }
};
enum WS_MESSAGE_STATUS {
  MSG_Nodata = 0,
  MSG_Received = 1,
  MSG_Downloading = 2,
  MSG_Downloaded = 3,
  MSG_Download_error = 4
};
struct download_info {
  WS_MESSAGE_STATUS status;
  std::vector<char> buffer;
  int msgIndex;
  ws_message msg_pkt;
};
typedef ws_message MsgStruct;
typedef download_info OutputData;

class WSConnection : WebSocket {
public:
  WSConnection(std::queue<download_info> &);
  std::thread spawn() {
    return std::thread([this] { this->run(); });
  }
  void init(std::queue<download_info> &);
  void open(string url);
  bool isOpened() { return WebSocket::isOpened(); }

private:
  RTCSocketState socketState;
  ws_header_infor headerInfo;
  std::string m_url;
  std::mutex m_mtx;
  bool m_reconected_flag = false;
  std::vector<std::thread> m_threadPool;

  // buffer data
  std::queue<string> m_msgQueue;
  std::queue<MsgStruct> m_linkQueue;
  std::map<string, download_info> m_downloadedMap;
  std::queue<download_info> &m_downloadedQueue;

  int msgIndex = 0;
  int count_downloadedRequest = 0;
  int count_downloadSuccess = 0;
  int count_downloadErr = 0;

  void run();
  void reconect();
  void createThreadPool();
  void download();

  ws_header_infor parserUrl(string url);
  void sendKey();
  void handleSocketResponse(string msg);
  void handlePackageInfo(string);
  MsgStruct parserMessage(string msg);
  std::string makeLink(string msg);
  void download(MsgStruct); // download and push to buffer
  void HTTPRequest(MsgStruct);

  int downloadingNumber();
  void sort();
  download_info getNextPackage();
  string getNextWsMessage();

  //    std::condition_variable condition;
};

#endif // WS_CONNECTION_H
