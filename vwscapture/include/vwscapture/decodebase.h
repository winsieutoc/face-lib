#ifndef TQ_DECODER_BASE_H
#define TQ_DECODER_BASE_H

#include <string>

namespace vws {
    
    struct H264Frame {
        long timestamp = 0;
        char* buffer;
    };
    
    struct RawData {
        int width = 0;
        int height = 0;
        long timestamp = 0;
        int channel = 3;
        char* buffer;
    };
    
    template <typename In, typename Out>
    class DecoderBase
    {
    public:
        virtual In getFrame();
        virtual bool decode(In, Out&);
        virtual void run();
        virtual bool init();
    };
    
    // class H26Decoder_FFMPEG : DecoderBase<H264Frame, RawData>
    // {
    // };
    
    // class H26Decoder_OpenH264 : DecoderBase<H264Frame, RawData>
    // {
    // };
}

#endif // TQ_DECODER_BASE_H
