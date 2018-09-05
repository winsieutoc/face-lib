#include "wscapture.h"
#include <stdint.h>
#include <iostream>
#include <unistd.h>
#include <thread>

/***************************************************
|   | Socket |      | Decoder |     | Example |    |
****************************************************
_____________________________________________________________
    Socket      - Create WebSocket protocol connection
                - Create threads to download package(maybe ThreadPool)
                - Push downloaded packages to Buffer[1]
_____________________________________________________________
    Decoder     - Analyze RTP packages from Buffer[1]
                - Push to Decoder
                - Convert to cv::Mat
                - Push images to Buffer[2]
_____________________________________________________________
    Watcher     - TBD
_____________________________________________________________
    Example     - Show images => Example getting frames from myLibrary(Buffer[2])

****************************************************/

WSCapture::WSCapture()
{
}

void WSCapture::open(string url)
{
    printf("WSCapture variable downloadedQueue is at address: %p\n", (void*)&downloadedQueue);
    m_wsconnection = new WSConnection(downloadedQueue);
    m_decoder = new Decoder(downloadedQueue, decodedFrameQueue);

    m_wsconnection->open(url);
    run();
}

bool WSCapture::isOpened()
{
    return m_wsconnection->isOpened();
}

void WSCapture::test()
{
    while(true) {
        usleep(30000);
        m_decoder->getFrame();
    }
}

WSCaptureData WSCapture::getNextData()
{
    return m_decoder->getFrame();
}

WSCaptureData WSCapture::pop()
{
    return m_decoder->getFrame();
}

void WSCapture::run()
{
    std::vector<std::thread> ths;
    ths.push_back(m_wsconnection->spawn());
    ths.push_back(m_decoder->spawn());
	for (auto &t : ths)
        t.detach();
}
