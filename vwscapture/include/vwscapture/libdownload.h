
#ifndef LIB_DOWNLOAD_H
#define LIB_DOWNLOAD_H

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <stdio.h>
#include <memory.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <vector>
#include <sys/time.h>

using namespace std;
using std::string;
using std::ofstream;
using std::stringstream;

#define MAX_PACKET_SIZE 50000

struct ws_download_infor{
    stringstream dl_request;
    string dl_ip;
    string dl_port;
    string dl_host;
    string dl_get;
};

#define HTTPInstance HTTPDownload::getInstance()
class HTTPDownload
{
public:
    static HTTPDownload& getInstance(void) {
        static HTTPDownload instance;
        return instance;
    }
    HTTPDownload();

    int download(const string& url, vector<char>& b);
private:
    // LINUX
    timeval tv;

    vector<string> dns_lookup(const string &host_name, int ipv=4); //ipv: default=4
    bool is_ipv6_address(const string& str);
    bool is_ipv4_address(const string& str);
    int socket_connect(string ip_address, int port);
    string header_value(const string& full_header, const string& header_name);
    int http_get(const string& request, const string& ip_address, int port, vector<char>& b);
    ws_download_infor parse_dl_header(string header);
};
//int main(int argc, char* argv[])
//{
//    download("https://www.cplusplus.com/img/cpp-logo.png", "./demo.png");
////     download("http://code.jquery.com/jquery-1.11.1.js", "./jquery-1.11.1.js");
//    return 0;
//}

#endif // LIB_DOWNLOAD_H