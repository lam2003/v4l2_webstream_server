#ifndef H264ENCODER_H
#define H264ENCODER_H

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libswscale/swscale.h>
#include <libavutil/pixfmt.h>
#include <x264.h>
}
#include "Config.h"

namespace h264{

class H264Encoder{
public:
    H264Encoder();
    ~H264Encoder();
    void x264Init(AVPicture picture,int width,int height);
    void x264Encode();
    x264_nal_t *nals;
    int nnal;
private:
    x264_t *x264Encoder;
    x264_param_t param;
    x264_picture_t xPic;
    int64_t i_pts;
    x264_picture_t *picOut;
};
}

#endif 