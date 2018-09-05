#include "vwscapture/decoder.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
// #include "cmsconnection.hpp"

#define MAX_DECODED_BUFFER_SIZE     38 // 18
#define MIN_DECODED_BUFFER_SIZE     4
#define SKIP_UNIT                   4

Decoder::Decoder(std::queue<download_info>& input, std::queue<frame_packet>& output)
    :m_downloadedQueue(input),
      m_decodedFrameQueue(output)
{
    printf("Decoder variable A is at address: %p\n", (void*)&m_downloadedQueue);
}

bool Decoder::isRunning()
{
    return true;
//    return bRunning; // TBD
}
void Decoder::SetRunningStt(bool stt)
{
    cout << "SetRunningStt: "<<stt<<endl;
    bRunning = stt;
}
frame_packet Decoder::getFrame()
{
    frame_packet lastItem;
    while (isRunning() && lastItem.frame.empty()) {
        m_mtx.lock();
        if (!m_decodedFrameQueue.empty()) {
            lastItem = m_decodedFrameQueue.front();
            m_decodedFrameQueue.pop();
        }
        m_mtx.unlock();
        usleep(1000);
    }

    #ifdef DEBUG_SAVE_IMAGES
    if (!lastItem.frame.empty()) {
        string name = m_packageId+"_"+std::to_string(lastItem.timestamp);
        string path = "/home/vface/Pictures/wsreader/"+name+".jpg";
        static std::queue<std::string> q;
        if ((int)q.size()>1000) {
            auto lastFile = q.front();
            q.pop();
            if(remove(lastFile.c_str()) != 0 )
                perror( "Error deleting file" );
        }
        q.push(path);
        cv::imwrite(path.c_str(), lastItem.frame);
        // static int xx = 0;
        // xx+=50;
        // if (xx>1280) xx=0;
        // Rect test = cv::Rect(xx,150,200,199);
        // std::vector<cv::Rect> prects;
        // prects.push_back(test);
        // CMSInstance.push(prects, lastItem.timestamp);
    }
    #endif

    return lastItem;
}

#ifndef DEV_NEW_FUNC
void Decoder::appendFrame(frame_packet item)
{
    std::lock_guard<std::mutex> lock(m_mtx);
    int qsize = m_decodedFrameQueue.size();
    if (qsize < MAX_DECODED_BUFFER_SIZE) {
        int unit = (int)(qsize/SKIP_UNIT);
        int check = 0;
        if (unit > 0) {
            check = item.index%unit;
        }
        // printf("____Check [%d] [%d] [%d] [%d]\n", (int)item.index, qsize, unit, check);
        if (check == 0) {
            m_decodedFrameQueue.push(item);
        }
        else {
            // printf("ERROR [FlushingSlow] >>> SkipData[%d] [%d] [%d] [%d]\n", (int)item.index, qsize, unit, check);
        }
    }
}
#else
void Decoder::appendFrame(frame_packet item) {
    std::lock_guard<std::mutex> lock(m_mtx);
    int qsize = m_decodedFrameQueue.size();

    if (!m_flush && qsize>MAX_DECODED_BUFFER_SIZE) {
        // printf("ERROR [FlushingSlow]\n");
        m_flush = true;
    }
    else if (m_flush && qsize<MIN_DECODED_BUFFER_SIZE) {
        m_flush = false;
    }
    if (!m_flush) {
        m_decodedFrameQueue.push(item);
    }
    else {
        int unit = (int)(qsize/SKIP_UNIT)-1;
        int check = -1;
        if (unit > 0) {
            check = item.index%unit;
        }
        // printf("____Check [%d] [%d] [%d] [%d]\n", (int)item.index, qsize, unit, check);
        if (check == 0) {
            m_decodedFrameQueue.push(item);
        }
        else {
            // printf("ERROR [FlushingSlow] >>> SkipData[%d] [%d] [%d] [%d]\n", (int)item.index, qsize, unit, check);
        }
    }
}
#endif //DEV_NEW_FUNC

download_info Decoder::getDownloadedPackage()
{
    download_info lastItem;
    while (isRunning() && lastItem.buffer.empty()) {
        m_mtx.lock();
        if (!m_downloadedQueue.empty()) {
            lastItem = m_downloadedQueue.front();
            m_downloadedQueue.pop();
        }
        m_mtx.unlock();
        usleep(1000);
    }
    return lastItem;
}

void Decoder::run()
{
    cout << "Decoder running..."<<endl;
    bRunning = true;
    initializeDecoder();
    decodeProcess();
}

void Decoder::decodeProcess()
{
    while (isRunning()) {
        download_info curPackage;
        curPackage = getDownloadedPackage();
        if (!curPackage.buffer.empty() && curPackage.status == MSG_Downloaded) {
            m_packageId = curPackage.msg_pkt.id;
            splitData(curPackage.buffer, curPackage.msg_pkt.id, curPackage.msg_pkt.type);
            usleep(1000);
        }
        else {
            std::cout << "ERROR! Nodata for decodeProcess" << std::endl;
            usleep(40000);
        }
    }
}

void Decoder::initializeDecoder() {
    bRunning = true;
    cap.init2();
}

void Decoder::checkingFUApackage(vector<char> h264Raw) {
    unsigned int nalType = h264Raw[4] & MB_NAL_TYPE;
    switch (nalType) {
    case 7:  // SPS Type
    case 8:  // PPS Type
    case 5:  // IDR Typ
    case 1:  // P-Frame  Pictur Type
        decodeRTPPackage(NALUStartCode + h264Raw);
        break;
    default:
        break;
    }
}

void Decoder::decodeRTPPackage(vector<char> rtpPackage) {
    int rtpSize = rtpPackage.size();
#ifdef TTQ_DEBUG_SPLIT
    cout << "decodeRTPPackage "<<rtpPackage.size() <<endl;
    print_hex_vector(rtpPackage,0,rtpSize>10?10:rtpSize);
#endif
#ifdef H264_FILE
    exportVectorToFile(H264_FILE,rtpPackage);
#endif
    uint8_t *data = new uint8_t[rtpSize];
    memcpy(data, rtpPackage.data(), rtpSize);

//     cap.grabFrame(data,rtpSize);
//     IplImage* img2 = cap.retrieveFrame();
//     if (img2) {
//         cv::Mat image2;
//         image2 = cvarrToMat(img2);
// //        image2 = cap.retrieveMatFrame();
//         m_frame.frame = image2.clone();
//         m_frame.index++;
//         m_frame.timestamp = curTimeStamp;
//         appendFrame(m_frame);
//     }

    cv::Mat image2 = cap.grabFrame2(data,rtpSize);
    if (!image2.empty()) {
        m_frame.frame = image2;
        m_frame.index++;
        m_frame.timestamp = curTimeStamp;
        appendFrame(m_frame);
        // cv::imshow("..", image2);
        // cv:waitKey(20);
    }
    rtpPackage.clear();
    free(data);
}

void Decoder::splitData(vector<char> rawData, std::string pkt_id, std::string pkt_type) {
    // cout << "====>>> "<<pkt_id<<endl;
    if (!rawData.empty()) {
        long passedBytes = 0;
        bool rtpIsOk = true;
        while (isRunning() && passedBytes < (int)rawData.size() && rtpIsOk) {
            // Parse raw data
            int payloadSize = (rawData[passedBytes+1] & 0xFF) | ((rawData[passedBytes] & 0xFF) << 8);
            // Phần data h264 bị ngắn hơn chiều dài.
            int lengthOfH264RawData = payloadSize - 10;
            if (passedBytes + HEADER_LENGTH + lengthOfH264RawData > (int)rawData.size()) {
                rtpIsOk = false;
                cout << " something went wrong... in RTP passedBytes + HEADER_LENGTH + lengthOfH264RawData="
                     << (passedBytes + HEADER_LENGTH + lengthOfH264RawData) << "/" << rawData.size() <<endl;
                break;
            }
            vector<char> h264Raw = mid(rawData,passedBytes+HEADER_LENGTH,lengthOfH264RawData);
            vector<char> timeData = mid(rawData,passedBytes+2,8);
            passedBytes = passedBytes + payloadSize + 2;
            
            uint64_t mtime = 0;
            for (int i =0;i<8;i++) {
                mtime += (uint8_t)timeData[i];
                if (i!=7)   mtime = mtime << 8;
            }
            double timestamp = reinterpret_cast <double&>(mtime); 
            // memcpy(&mtime, b, sizeof(double));
            long timestampLong = (long)timestamp;
//             cout << " ________________ | timestamp: "<<timestampLong<<endl;
            curTimeStamp = timestampLong;


            #ifdef DEBUG_SAVE_IMAGES
            static std::queue<string> q2;
            static string lastId;
            if (m_packageId != lastId) {
                lastId = m_packageId;
                q2.push(lastId);
                string curPath = "/home/vface/temp/"+m_packageId+".txt";
                UtilInstance.writeToFile(curPath, m_packageId+"\n\n");
            }
            if ((int)q2.size()>100) {
                string firstPath = "/home/vface/temp/"+q2.front();
                q2.pop();
                if(remove(firstPath.c_str()) != 0 )
                    perror( "Error deleting file" );
            }
            string name2 = std::to_string(timestampLong)+"\n";
            string curPath = "/home/vface/temp/"+m_packageId+".txt";
            UtilInstance.writeToFile(curPath, name2);
            #endif

            unsigned int nalType = h264Raw[0] & MB_NAL_TYPE;
            switch (nalType) {
            case 7:  // SPS Type
            case 8:  // PPS Type
            case 1:  // P-Frame  Pictur Type
                decodeRTPPackage(NALUStartCode + h264Raw);
                break;
            case 5:  // IDR Type
                decodeRTPPackage(NALUStartCode + h264Raw);
                checkIFrameFirst = true;
                break;
            case 28: {  // IDR Picture Type MULTIPLE PART
                // cout << "cham \n";
                char FU_A_byte = h264Raw[1];
                if (FU_A_byte & MB_FU_S) {
                    IDRPicture = IDR_NAL_TYPE + shift(h264Raw, 2);
                    IDRPictureStarted = true;
                }
                else if (FU_A_byte & MB_FU_E) {  // end of Fragment Unit
                    if (IDRPictureStarted == false) {
                        rtpIsOk = false;
                        break;
                    }
                    IDRPicture = IDRPicture + shift(h264Raw, 2);
                    decodeRTPPackage(NALUStartCode + IDRPicture);
                    checkIFrameFirst = true;
                    IDRPictureStarted = false;  // end of IDR Picture
                }
                else {
                    // printf("FU_A_byte No: 0x%2x \n",h264Raw[1]);
                    if (IDRPictureStarted == false) {
                        rtpIsOk = false;
                        break;
                    }
                    IDRPicture = IDRPicture + mid(h264Raw, 2, payloadSize - HEADER_LENGTH);
                }
            } break;

            case 0: // End Of 10 minute type
                break;
            default:
                cout << " something went wrong... in RTP passedBytes=" << passedBytes << "/" << rawData.size() << " nalType=" << nalType <<endl;
                break;
            }
        }
    }
}
