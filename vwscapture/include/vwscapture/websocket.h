#ifndef WEB_SOCKET_CLIENT_H
#define WEB_SOCKET_CLIENT_H

#include <string>
#include <vector>

typedef int socket_t;
typedef enum readyStateValues { CLOSING, CLOSED, CONNECTING, OPEN } readyStateValues;
using namespace std;

struct wsheader_type {
    unsigned header_size;
    bool fin;
    bool mask;
    enum opcode_type {
        CONTINUATION = 0x0,
        TEXT_FRAME = 0x1,
        BINARY_FRAME = 0x2,
        CLOSE = 8,
        PING = 9,
        PONG = 0xa,
    } opcode;
    int N0;
    uint64_t N;
    uint8_t masking_key[4];
};

#define SocketInstance WebSocket::getInstance()
class WebSocket {
public:
    static WebSocket& getInstance(void) {
        static WebSocket instance;
        return instance;
    }
    WebSocket();
    void open(string url);
    void poll(int timeout = 0); // timeout in milliseconds
    void send(const std::string& message);
    void sendBinary(const std::string& message);
    void sendBinary(const std::vector<uint8_t>& message);
    void sendPing();
    void close();
    bool isOpened() {return readyState != CLOSED;}
    string response();
private:
    socket_t hostname_connect(const std::string& hostname, int port);
    void dispatch(string& callable);
    readyStateValues getReadyState();
    template<class Iterator>
    void sendData(wsheader_type::opcode_type type, uint64_t message_size, 
        Iterator message_begin, Iterator message_end);

    std::vector<uint8_t> rxbuf;
    std::vector<uint8_t> txbuf;
    std::vector<uint8_t> receivedData;

    socket_t sockfd;
    readyStateValues readyState;
    bool useMask;
};

#endif /* WEB_SOCKET_CLIENT_H */
