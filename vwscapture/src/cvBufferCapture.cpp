#include "vwscapture/cvBufferCapture.h"

bool CvCapture_FFMPEG::init()
{
    width = 1280;
    height = 720;
    dst2 = NULL;
    buffer2 = NULL;
    av_register_all();
    avcodec_register_all();
    // av_init_packet(&avpkt);
    // av_log_set_level(AV_LOG_QUIET);

    codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!codec) {
        fprintf(stderr, "codec not found\n");
        return false;
    }

    c = avcodec_alloc_context3(codec);
    // picture= av_frame_alloc();

    if(codec->capabilities&CODEC_CAP_TRUNCATED)
        c->flags|= CODEC_FLAG_TRUNCATED; /* we do not send complete frames */

    c->width = width;
    c->height = height;
    c->channels = 1;

    /* For some codecs, such as msmpeg4 and mpeg4, width and height
    MUST be initialized there because this information is not
    available in the bitstream. */

    // open it
    if (avcodec_open2(c, codec, NULL) < 0) {
        fprintf(stderr, "could not open codec\n");
        return false;
    }

    return true;
}

void CvCapture_FFMPEG::close()
{
    if( picture )
    av_free(picture);

    // free last packet if exist
    if (avpkt.data) {
        av_free_packet (&avpkt);
    }
}

bool CvCapture_FFMPEG::reset()
{
    printf("\n\nreset decoder\n\n");
    close();
    return init();
}

IplImage* CvCapture_FFMPEG::retrieveFrame()
{
    return dst2;
}

bool CvCapture_FFMPEG::grabFrame(uint8_t * data, int length)
{
    if (dst2)
    {
        cvReleaseImage(&dst2);
    }

    #ifdef DEBUG_FFMPEG_DECODER
    static int check_step = 0;
    check_step++;
    if (check_step > 100) {
        check_step = 0;
        reset();
    }
    #endif

    avpkt.size = length;
    if (avpkt.size == 0)
        return false;

    avpkt.data = data;
    int count = 0;
    while (avpkt.size > 0) {
        count++;
        int got_picture;

        struct timeval x0;
        gettimeofday(&x0, NULL);
        int len = avcodec_decode_video2(c, picture, &got_picture, &avpkt);
        struct timeval x1;
        gettimeofday(&x1, NULL);
        // float val = (x1.tv_sec - x0.tv_sec) * 1000.0f + (x1.tv_usec - x0.tv_usec) / 1000.0f;
        // printf(" ____ CHECKTIME[avcodec_decode]  %f\n", val);

        if (len < 0) {
//            fprintf(stderr, "Error while decoding frame %d\n", frame);
//            dst2 = NULL;
            return false;
        }
        key_frame = picture->key_frame;
        if (got_picture) {
            AVFrame* rgb_picture = alloc_pictureBGR24(picture->width, picture->height) ;
            struct SwsContext * img_convert_ctx = sws_getContext(
                picture->width, picture->height, AV_PIX_FMT_YUV420P,
                picture->width, picture->height, AV_PIX_FMT_BGR24,
                SWS_BICUBIC,
                NULL, NULL, NULL);

            sws_scale(
                img_convert_ctx, picture->data, picture->linesize, 0, picture->height,
                rgb_picture->data, rgb_picture->linesize);

            sws_freeContext(img_convert_ctx);

            struct timeval x1;
            gettimeofday(&x1, NULL);
            // float val = (x1.tv_sec - x0.tv_sec) * 1000.0f + (x1.tv_usec - x0.tv_usec) / 1000.0f;
            // printf(" ____ CHECKTIME[avcodec_decode]  %f\n", val);

            dst2 = cvCreateImage( cvSize(picture->width, picture->height), IPL_DEPTH_8U, 3);

            struct timeval x2;
            gettimeofday(&x2, NULL);
            // float val2 = (x2.tv_sec - x1.tv_sec) * 1000.0f + (x2.tv_usec - x1.tv_usec) / 1000.0f;
            // printf(" ____ CHECKTIME[cvCreateImage]  %f\n", val2);

            //dst2->imageData = (char*)rgb_picture->data[0];
            memcpy( dst2->imageData, rgb_picture->data[0], dst2->imageSize);
//             for (int i = 0; i < dst2->imageSize; i++) {
//                 dst2->imageData[i] = rgb_picture->data[0][i];
//             }

            struct timeval x3;
            gettimeofday(&x3, NULL);
            // float val3 = (x3.tv_sec - x2.tv_sec) * 1000.0f + (x3.tv_usec - x2.tv_usec) / 1000.0f;
            // printf(" ____ CHECKTIME[imageData]  %f\n", val3);

             delete [] buffer2;

            struct timeval x4;
            gettimeofday(&x4, NULL);
            // float val4 = (x4.tv_sec - x3.tv_sec) * 1000.0f + (x4.tv_usec - x3.tv_usec) / 1000.0f;
            // printf(" ____ CHECKTIME[av_free]  %f\n", val4);
             av_free(rgb_picture);

            struct timeval x5;
            gettimeofday(&x5, NULL);
            // float val5 = (x5.tv_sec - x4.tv_sec) * 1000.0f + (x5.tv_usec - x4.tv_usec) / 1000.0f;
            // printf(" ____ CHECKTIME[av_free]  %f\n", val5);
        }
        avpkt.size -= len;
        avpkt.data += len;
    }
    // printf(" ____ CHECKTIME[count]  %d\n", count);
    return true;
}

AVFrame *CvCapture_FFMPEG::alloc_pictureBGR24(int width, int height)
{
    AVFrame *pFrameRGB = av_frame_alloc();
    if(pFrameRGB==NULL) return NULL;
    int numBytes = avpicture_get_size(AV_PIX_FMT_BGR24, width, height);
    buffer2=new uint8_t[numBytes];
    avpicture_fill((AVPicture *)pFrameRGB, buffer2, AV_PIX_FMT_BGR24, width, height);
    return pFrameRGB;
}

void CvCapture_FFMPEG::init2()
{
    printf("CvCapture_FFMPEG init2\n");
    av_log_set_level(AV_LOG_QUIET);
    av_register_all();
    avcodec_register_all();
    avCodec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!avCodec) {
        // TODO
    }
    avCodecContext = avcodec_alloc_context3(avCodec);
    if (!avCodecContext) {
        // TODO
    }
    else{
        if (avCodec->capabilities & CODEC_CAP_TRUNCATED) {
            // avCodecContext->flags || CODEC_FLAG_TRUNCATED;
            avCodecContext->flags |= CODEC_FLAG_TRUNCATED;
        }
        if (avcodec_open2(avCodecContext, avCodec, NULL) < 0) {
            // TODO
        }
        avCodecContext->thread_count = 4;
        avCodecContext->thread_type = 2;
    }
}

void CvCapture_FFMPEG::close2()
{
    if(avCodecContext && avCodecContext->internal && avCodecContext->refcounted_frames != 0) {
        avcodec_flush_buffers(avCodecContext);
        avcodec_free_context(&avCodecContext);
        avCodecContext = avcodec_alloc_context3(avCodec);
        if(avCodecContext){
            avCodecContext->width = 0;
            avCodecContext->height = 0;
        }
    }
    delete avCodec;
    avCodec = NULL;
    delete avCodecContext;
    avCodecContext = NULL;
}

void CvCapture_FFMPEG::reset2()
{
    if(avCodecContext && avCodecContext->internal && avCodecContext->refcounted_frames != 0) {
        avcodec_flush_buffers(avCodecContext);
        avcodec_free_context(&avCodecContext);
        avCodecContext = avcodec_alloc_context3(avCodec);
        if(avCodecContext){
            avCodecContext->width = 0;
            avCodecContext->height = 0;
        }
    }
    // init2();

    // while (true) {
    //     printf("\ntest ====>> ffmpeg flush\n");
    //     // AVPacket avPacket2;
    //     // av_init_packet(&avPacket2);
    //     avPacket.size = 0;
    //     avPacket.data = NULL;
    
    //     AVFrame *avFrame2 = av_frame_alloc();
    //     int len2;
    //     int decodedFrameCount2;
    //     len2 = avcodec_decode_video2(avCodecContext, avFrame2, &decodedFrameCount2, &avPacket);
    //     if (decodedFrameCount2) {
    //         cout << "=======>>>>>>>>>>> teng teng\n";
    //     }
    //     else {
    //         return;
    //     }
    //     av_frame_unref(avFrame2);
    //     av_frame_free(&avFrame2);
    //     // av_packet_unref(&avPacket);
    //     // av_free_packet(&avPacket);
    // }
}

cv::Mat CvCapture_FFMPEG::grabFrame2(uint8_t * data, int length)
{
    cv::Mat image;
    if(data[3] == 0x67 || data[4] == 0x67) {
        reset2();
    }
    if (length > 0) {
        AVPacket avPacket;
        av_init_packet(&avPacket);
        avPacket.size = length;
        avPacket.data = data;
    
        AVFrame *avFrame = av_frame_alloc();
        if(avFrame == NULL) {
            printf("Unable to allocate frames \n");
        }
        
        int decodedFrameSize = 0, decodedFrameCount = 0;
        decodedFrameSize = avcodec_decode_video2(avCodecContext, avFrame, &decodedFrameCount, &avPacket);
        if (decodedFrameSize > 0 && decodedFrameCount > 0) {
            int width = avCodecContext->width;
            int height = avCodecContext->height;

            if(width > 0 && height >0){
                struct SwsContext *swsContext = sws_getContext(width , height,
                                                                avCodecContext->pix_fmt, avCodecContext->width,
                                                                avCodecContext->height, AV_PIX_FMT_BGR24, SWS_BICUBIC,
                                                                NULL, NULL, NULL);

                uint8_t *outBuffer = (uint8_t *)av_malloc(avpicture_get_size(
                                                                AV_PIX_FMT_BGR24, avCodecContext->width, avCodecContext->height));
                AVPicture avPicture;
                avpicture_fill(&avPicture, outBuffer, AV_PIX_FMT_BGR24,
                                avCodecContext->width, avCodecContext->height);
                if(swsContext)    { //valid
                    sws_scale(swsContext, (const uint8_t *const *)avFrame->data,
                                avFrame->linesize, 0, avCodecContext->height,
                                avPicture.data, avPicture.linesize);

                    image = cv::Mat(avCodecContext->height, avCodecContext->width, CV_8UC3, avPicture.data[0], avPicture.linesize[0]).clone();
                }
                av_free(outBuffer);
                sws_freeContext(swsContext);
            }
        }

        av_frame_unref(avFrame);
        av_frame_free(&avFrame);
        av_packet_unref(&avPacket);
        av_free_packet(&avPacket);
    }
    return image;
}