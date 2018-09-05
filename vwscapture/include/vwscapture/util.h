
#ifndef UTIL_FUNCTION_H
#define UTIL_FUNCTION_H

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fstream>
#include <iostream>
#include <iostream>
#include <mutex>
#include <queue>
#include <sstream>
#include <stdint.h>
#include <string>
#include <sys/time.h>
#include <unistd.h>
#include <vector>

// #define H264_FILE "/home/thieutq/ws.h264"
#define DISPLAY_VIDEO

using namespace std;

typedef std::vector<char> DownloadedData;
typedef std::queue<DownloadedData> DownloadedType;

typedef uint8_t uint8;
#define UtilInstance Util::getInstance()

template <typename T>
std::vector<T> operator+(const std::vector<T> &A, const std::vector<T> &B) {
  std::vector<T> AB;
  AB.reserve(A.size() + B.size());         // preallocate memory
  AB.insert(AB.end(), A.begin(), A.end()); // add A;
  AB.insert(AB.end(), B.begin(), B.end()); // add B;
  return AB;
}

template <typename T>
std::vector<T> &operator+=(const std::vector<T> &A, const std::vector<T> &B) {
  A.reserve(A.size() +
            B.size()); // preallocate memory without erase original data
  A.insert(A.end(), B.begin(), B.end()); // add B;
  return A;                              // here A could be named AB
}

template <typename T> std::vector<T> shift(std::vector<T> &A, int x) {
  std::vector<T> AB(A.begin() + x, A.end());
  return AB;
}

template <typename T>
std::vector<T> mid(std::vector<T> &A, int pos, int length) {
  if (-1 == length) {
    length = A.size();
  }
  std::vector<T> AB(A.begin() + pos, A.begin() + pos + length);
  return AB;
}

class Util {
public:
  Util();
  static Util &getInstance(void) {
    static Util instance;
    return instance;
  }
  // struct ws_message parse_ws_message(char* );
  // ws_header_infor parse_ws_header(string header);
  std::vector<std::string> split(const std::string &s, char delim);
  string delSpaces(string str);

  void print_hex_memory(void *mem, int pos, int n);
  void exportVectorToFile(string url, std::vector<char>);

  void writeToFile(string url, string s) {
    std::ofstream outfile;
    outfile.open(url, std::ios_base::app);
    outfile << s;
  }

  template <typename T> int queue_size(std::queue<T> &buffer) {
    int ret = -1;
    mtx.lock();
    ret = (int)buffer.size();
    mtx.unlock();
    return ret;
  }

  template <typename T> void queue_push(std::queue<T> &buffer, T value) {
    mtx.lock();
    buffer.push(value);
    mtx.unlock();
  }

  template <typename T> T queue_pop(std::queue<T> &buffer) {
    T ret;
    while (ret.empty()) {
      mtx.lock();
      if ((int)buffer.size() > 0) {
        ret = buffer.front();
        buffer.pop();
      }
      mtx.unlock();
      usleep(1000);
    }
    return ret;
  }

  template <typename T>
  void print_hex_vector(std::vector<T> p, int pos, int n) {
    int i;
    for (i = pos; i < (n + pos); i++) {
      printf("0x%02x ", p[i] & 0xff);
      if (((i - pos) % 16 == 0) && (i != pos))
        printf("\n");
    }
    printf("\n");
  }

  // template <typename T>
  // std::vector<T> operator+(const std::vector<T> &A, const std::vector<T> &B)
  // {
  //     std::vector<T> AB;
  //     AB.reserve( A.size() + B.size() );                // preallocate memory
  //     AB.insert( AB.end(), A.begin(), A.end() );        // add A;
  //     AB.insert( AB.end(), B.begin(), B.end() );        // add B;
  //     return AB;
  // }

  // template <typename T>
  // std::vector<T> &operator+=(const std::vector<T> &A, const std::vector<T>
  // &B)
  // {
  //     A.reserve( A.size() + B.size() );                // preallocate memory
  //     without erase original data
  //     A.insert( A.end(), B.begin(), B.end() );         // add B;
  //     return A;                                        // here A could be
  //     named AB
  // }

  // template <typename T>
  // std::vector<T> shift(std::vector<T> &A, int x)
  // {
  //     std::vector<T> AB(A.begin()+2,A.end());
  //     return AB;
  // }

  // template <typename T>
  // std::vector<T> mid(std::vector<T> &A, int pos, int length)
  // {
  //     if (-1 == length) {
  //         length = A.size();
  //     }
  //     std::vector<T> AB(A.begin()+pos,A.begin()+pos+length);
  //     return AB;
  // }
private:
  std::mutex mtx;
};

#endif // UTIL_FUNCTION_H
