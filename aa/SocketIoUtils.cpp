#include "SocketIoUtils.hpp"

using namespace sio;
using namespace std;
std::mutex _lock;
#define SERVER_ADDRESS "https://9c1.vcam.viettel.vn/insertMultilLicensePlate"
std::condition_variable_any _cond;

bool connect_finish = false;

void connection_listener::on_connected() {
  _lock.lock();
  _cond.notify_all();
  connect_finish = true;
  _lock.unlock();
}
void connection_listener::on_close(client::close_reason const &reason) {
  std::cout << "sio closed " << std::endl;
  exit(0);
}

void connection_listener::on_fail() {
  std::cout << "sio failed " << std::endl;
  exit(0);
}

SocketIoUtils::SocketIoUtils() {

  SendDataDelegate = [=](std::string jsonData) { SendData(jsonData); };
  syncInfoForCanvasDelegate = [=](std::string data) {
    syncInfoForCanvas(data);
  };
  url = cpuUsage = ramUsage = diskUsage = base64Data = faceID = "";
}

void SocketIoUtils::syncInfoForCanvas(string data) {
  //  cout << "Start syncInfoForCanvas" << endl;
  this->h.socket()->emit("engineInfo", data);
}
void SocketIoUtils::SendData(string jsonData) {
  //  cout << "Start SendData" << endl;
  //  cout << jsonData << endl;
  this->h.socket()->emit("customer.add", jsonData);
  //  }
}

void SocketIoUtils::Connect2Server() {

  if (url.empty()) {
    cout << "Url is empty, can't connect to server";
    throw 1;
    return;
  } else {
    connection_listener l(h);
    h.set_open_listener(std::bind(&connection_listener::on_connected, &l));
    h.set_close_listener(
        std::bind(&connection_listener::on_close, &l, std::placeholders::_1));
    h.set_fail_listener(std::bind(&connection_listener::on_fail, &l));
    h.connect(this->url);
    current_socket = h.socket();
    std::string msg = "";
    current_socket->emit("cleartrack", msg);
  }
}
SocketIoUtils::~SocketIoUtils() {
  cout << "Closing..." << endl;

  h.sync_close();
  h.clear_con_listeners();
}

const string SocketIoUtils::currentDateTime() {
  time_t now = time(0);
  struct tm tstruct;
  char buf[80];
  tstruct = *localtime(&now);
  // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
  // for more information about date/time format
  strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

  return buf;
}
