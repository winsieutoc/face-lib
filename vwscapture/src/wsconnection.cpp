#include "vwscapture/wsconnection.h"
#include <thread>
#include "vwscapture/libdownload.h"

#define MAX_DOWNLOAD_THREAD     8
#define MAX_WAITING_DOWNLOAD    5
//#define DEBUG_PROCESS

WSConnection::WSConnection(std::queue<OutputData>& downloadedQueue):
    m_downloadedQueue(downloadedQueue)
{
    printf("WSConnection variable A is at address: %p\n", (void*)&m_downloadedQueue);
    socketState = RTCSS_Disconnected;
    createThreadPool();
}
void WSConnection::init(std::queue<OutputData>& downloadedQueue)
{
    m_downloadedQueue = downloadedQueue;
    printf("WSConnection variable A is at address: %p\n", (void*)&m_downloadedQueue);
}
int WSConnection::downloadingNumber()
{
    int ret = -1;
    ret = count_downloadedRequest-count_downloadSuccess-count_downloadErr;
    return ret;
}
void WSConnection::open(string url)
{
    m_url = url;
    headerInfo = parserUrl(m_url);
    WebSocket::open(m_url);
}
void WSConnection::reconect()
{
    WebSocket::close();
    usleep(100000);
    socketState = RTCSS_Disconnected;
    WebSocket::open(m_url);
}
ws_header_infor WSConnection::parserUrl(string header)
{
    ws_header_infor hd_info;
    size_t start =  header.find_first_of("//");
    string sub_str = header.substr(start+2,header.size()-1);
    start =  sub_str.find_first_of("/");
    hd_info.host = sub_str.substr(0,start);
    start =  hd_info.host.find_first_of(":");//remove :80
    hd_info.host = hd_info.host.substr(0,start);
    start = sub_str.find_last_of("/");
    hd_info.cam = sub_str.substr(start+1,sub_str.size()-1);
    sub_str = sub_str.substr(0,start);
    start = sub_str.find_last_of("/");
    hd_info.mac = sub_str.substr(start+1,sub_str.size()-1);
    hd_info.request = "http://"+hd_info.host+"/live/g/"+hd_info.cam+"/";
    return hd_info;
}

void WSConnection::handleSocketResponse(std::string message)
{
    switch (socketState) {
    case RTCSS_Disconnected: {
        socketState = RTCSS_SentSignature;
        sendKey();
    } break;
    case RTCSS_SentSignature: {
        socketState = RTCSS_ReceivedGOP;
    } break;
    case RTCSS_ReceivedGOP: {
        socketState = RTCSS_ReceivedDownloadLink;
    } break;
    case RTCSS_ReceivedDownloadLink: {
        handlePackageInfo(message);
    } break;
    default:
        break;
    }
}

void WSConnection::handlePackageInfo(string msg)
{
    std::lock_guard<std::mutex> lock(m_mtx);
    m_reconected_flag = false;
#ifdef DEBUG_PROCESS
    {
        cout <<endl;
        cout << "NEW MESSAGE: "<<msg<<endl;
        cout << " __ downloadingNumber: "<<downloadingNumber()<<endl;
        cout << " __ m_downloadedMap: "<<m_downloadedMap.size()<<endl;
        cout << " __ HTTP request: "
             << count_downloadedRequest
             << " " << count_downloadSuccess
             << " " << count_downloadErr
             <<endl;
        cout <<endl;
    }
#endif // DEBUG_PROCESS
    if ((int)m_linkQueue.size()<MAX_WAITING_DOWNLOAD) {

        MsgStruct msgdetail = parserMessage(msg);
        msgIndex++;
        download_info newItem;
        newItem.msgIndex = msgIndex;
        newItem.status = MSG_Received;
        m_downloadedMap[msgdetail.id] = newItem;
        m_msgQueue.push(msgdetail.id);
        m_linkQueue.push(msgdetail);
    }
    else {
        // cout << "ERROR [Network] Ignore message: "<<msg<<endl;
    }
}

std::string WSConnection::makeLink(string msg)
{
    return headerInfo.request+msg;
}

MsgStruct WSConnection::parserMessage(string msg_recived)
{
    // string test = "[1512544409058, 65178, 7, 1111111]"; msg = test;
    MsgStruct msg_info;

    std::string str_std(msg_recived);
    //Split "[" and "]"
    size_t start =  str_std.find_first_of("[");
    size_t end =  str_std.find_first_of("]");
    string str_std_2 = str_std.substr(start+1,end-start-1);

    //Extract id and size
    vector<string> info = UtilInstance.split(str_std_2,',');
    if(info.size() > 3)
    {
        msg_info.id = UtilInstance.delSpaces(info[0]);
        msg_info.size = stoi(UtilInstance.delSpaces(info[1]));
        msg_info.num = stoi(UtilInstance.delSpaces(info[2]));
        msg_info.type = UtilInstance.delSpaces(info[3]);
    }
    else {
        cout << "ERROR parserMessage"<<endl
             << "message: "<<msg_recived<<endl
             << "parser: " << str_std_2 << endl;
    }
    // << " "<<msg_info.size<<endl
    // << " "<<msg_info.num<<endl
    // << " "<<msg_info.type<<endl;
    return msg_info;
}

void WSConnection::sendKey()
{
    string msg = "{ \"action\":\"hello\", \"version\":\"2.0\", \"host_id\":\""
            + headerInfo.mac
            +"\", \"signature\":\"RESERVED\", \"timestamp\":\"1503907530054\" }";
    socketState = RTCSS_SentSignature;
    printf("sendKey: \n");
    cout << msg << endl;
    WebSocket::send(msg);
}

void WSConnection::run()
{
    while(true) {
        while(isOpened())
        {
            string msg = response();
            handleSocketResponse(msg);
            usleep(1000);
        }
        usleep(100000);
    }
}

void WSConnection::createThreadPool()
{
    for (int i=0; i<MAX_DOWNLOAD_THREAD;i++) {
        m_threadPool.push_back(std::thread( [this] { this->download(); } ));
    }
    m_threadPool.push_back(std::thread( [this] { this->sort(); } ));
    for (auto &t : m_threadPool) t.detach();
}
void WSConnection::sort()
{
    // convert map to queue
    while(true) {
        usleep(10000);
        download_info pkg = getNextPackage();
        m_mtx.lock();
        m_downloadedQueue.push(pkg);
//         cout << "sort...: "<<(int)m_downloadedQueue.size()<<" "<<pkg.buffer.size()<<endl;
        m_mtx.unlock();
    }
}

string WSConnection::getNextWsMessage()
{
    string msg;
    int i_sleep = 5000;
    int timeout = 15000000; // 1 000 000 = 1s

    while (msg.empty()) {
        m_mtx.lock();
        if (!m_msgQueue.empty()) {
            msg = m_msgQueue.front();
            m_msgQueue.pop();
        }
        m_mtx.unlock();
        timeout -= i_sleep;
        if(timeout < 0) {
            cout << endl<<endl;
            cout << "*************************************" << endl;
            cout << "Connection timeout" <<endl;
            cout << "*************************************" << endl;
            cout << endl<<endl;
            m_reconected_flag = true;
            usleep(1000000); // Note *****
            if (m_reconected_flag) {
                reconect();
            }
            break;
        }
        usleep(i_sleep);
    }
    return msg;
}

download_info WSConnection::getNextPackage()
{
    int i_sleep = 10000;
    int timeout = 2000000; // 1 000 000 = 1s

    download_info pkg;
    string next_msg = getNextWsMessage();
    while(next_msg.empty()) {
        usleep(40000);
        next_msg = getNextWsMessage();
    }
    if(!next_msg.empty()) {
        std::map<string, download_info>::iterator it;
        std::map<string, download_info>::iterator pEnd;

        m_mtx.lock();
        pEnd = m_downloadedMap.end();
        it = m_downloadedMap.find(next_msg);
        m_mtx.unlock();

        if(it!=pEnd) {
            while(timeout>=0) {
                m_mtx.lock();
                WS_MESSAGE_STATUS stt = it->second.status;
                m_mtx.unlock();
                switch (stt) {
                case MSG_Downloaded:
                    m_mtx.lock();
                    pkg = it->second;
                    m_mtx.unlock();
                    timeout = -1; // break
                    break;
                case MSG_Download_error:
                    timeout = -1; // break
                    break;
                default:
                    timeout-=i_sleep; // waiting for download to complete
                    break;
                }
                usleep(i_sleep);
            }
            m_mtx.lock();
            m_downloadedMap.erase(it);
            m_mtx.unlock();
        }
        else {
            cout << "[timeout] ERROR !!! Not found in listDownloadPackage"<<endl;
        }
    }
    return pkg;
}

#ifndef CMCODE
void WSConnection::download()
{
    while(true) {
        usleep(10000);
        auto liveid = UtilInstance.queue_pop(m_linkQueue);
//         cout << "DownloadThreadRunning: "<<liveid<<endl;
        HTTPRequest(liveid);
    }
}
#else
void WSConnection::download()
{
    while(true) {
        usleep(1000);
        string liveid;
        {
            std::unique_lock<std::mutex> lk(m_mtx);
            this->condition.wait(lk, [this]{return !this->m_linkQueue.empty();});
            liveid = UtilInstance.queue_pop(m_linkQueue);
            // lk.unlock();
            // this->condition.notify_one();
        }
        cout << "DownloadThreadRunning: "<<liveid<<endl;
        HTTPRequest(liveid);
    }
}
#endif
void WSConnection::HTTPRequest(MsgStruct msg_pkt)
{
    string cur_msg = msg_pkt.id;
    string url = makeLink(cur_msg);
    // cout << url <<endl;
    download_info cur_packet;
    int status;
    count_downloadedRequest++;
    status = HTTPInstance.download(url, cur_packet.buffer);

    std::lock_guard<std::mutex> lock(m_mtx);
    {
        std::map<string, download_info>::iterator it;
        it = m_downloadedMap.find(cur_msg);
        if(it!=m_downloadedMap.end()) {
            cur_packet.msgIndex = m_downloadedMap[cur_msg].msgIndex;
            cur_packet.msg_pkt = msg_pkt;
            if(status > 0) {
                count_downloadSuccess++;
                cur_packet.status = MSG_Downloaded;
                m_downloadedMap[cur_msg] = cur_packet;
            }
            else {
                count_downloadErr++;
                cur_packet.status = MSG_Download_error;
                cout << "ERROR ! Download packet error: "<<cur_msg<<endl;
                m_downloadedMap[cur_msg] = cur_packet;
            }
        }
        else {
            cout << "ERROR ! cannot find id in map: "<<cur_msg<<endl;
        }
    }
}
