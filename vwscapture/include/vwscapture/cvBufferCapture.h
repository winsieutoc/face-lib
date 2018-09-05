
extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libavutil/avutil.h>
    #include <libavutil/pixdesc.h>
    #include <libswscale/swscale.h>
    }
    
    #include "opencv2/highgui/highgui.hpp"
    #include "opencv2/imgproc/imgproc.hpp"
    
    #pragma once
    #include "util.h"
    
    using namespace std;
    using namespace cv;
    
    struct CvCapture_FFMPEG
    {
        bool init();
        void close();
        bool reset();
    
        void init2();
        void close2();
        void reset2();
        AVCodec *avCodec;
        AVCodecContext *avCodecContext;
    
    //    Mat retrieveMatFrame();
        IplImage* retrieveFrame();
        bool grabFrame(uint8_t * data, int length);
        cv::Mat grabFrame2(uint8_t * data, int length);
    
        AVFrame *alloc_pictureBGR24(int width, int height);
    
        AVPacket avpkt;
        AVCodec * codec;
        AVCodecContext *c;
        AVFrame* picture;
        char buf[1024*30];
        IplImage* dst2;
    //    Mat matImg;
        int key_frame;
        uint8_t *buffer2;
        int width;
        int height;
    };
    