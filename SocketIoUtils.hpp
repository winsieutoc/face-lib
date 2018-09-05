#ifndef SOCKETIOUTILS_HPP
#define SOCKETIOUTILS_HPP

#include "jsoncpp/json/json.h"
#include "sio_client.h"
#include "sys/sysinfo.h"
#include "sys/types.h"
#include <functional>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
using namespace sio;
using namespace std;

#define BIND_EVENT(IO, EV, FN) IO->on(EV, FN)
class connection_listener {
  sio::client &handler;

public:
  connection_listener(sio::client &h) : handler(h) {}

  void on_connected();
  void on_close(client::close_reason const &reason);
  void on_fail();
};

class SocketIoUtils {
  struct sysinfo memInfo;

public:
  socket::ptr current_socket;
  sio::client h;

public:
  SocketIoUtils();
  ~SocketIoUtils();
  //  bool connect_finish = false;
  // delegate call every time engine return data
  std::function<void(std::string jsonData)> SendDataDelegate;
  std::function<void(std::string data)> syncInfoForCanvasDelegate;
  /*
   * set Server address to url
   */
  void SetUrl(std::string address) { url = address; }

  std::string getCamID() const;
  void setCamID(const std::string &value);

public:
  /*
    * pointer of SenDataDelegate
    * faceID:
    *  +name: people already in database with identity
    *  +numberID: people already in database without identity
    *  +empty: people not available, cant recognize
    * img: image of people
    * camID: identity number of camera
    */

  void SendData(std::string jsonData);

  void syncInfoForCanvas(std::string data);

  // open connection to server
  void Connect2Server();

  // url of server
  std::string url;
  // keep cpu usage
  std::string cpuUsage;
  // keep ram usage
  std::string ramUsage;
  // keep disk usage
  std::string diskUsage;
  // keep base64 image data
  std::string base64Data;
  // keep face ID
  std::string faceID;
  // keep camID
  std::string camID;
  // keep current time
  std::string name;
  // Criminal status

  std::string alertUrl;
  std::string alertId;
  std::string alertName;

public:
  // get time
  const std::string currentDateTime();
};

#endif // SOCKETIOUTILS_HPP
