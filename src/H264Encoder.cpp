#include "H264Encoder.h"

using namespace h264;

H264Encoder::H264Encoder()
{
    i_pts = 0;
    x264Encoder = NULL;
    picOut = NULL;
    nnal = 0;
    nals = NULL;
}
H264Encoder::~H264Encoder()
{

}

void H264Encoder::x264Init(AVPicture picture,int width,int height)
{
    x264_param_default_preset(&param,"veryfast","zerolatency");
    param.i_width = width;
    param.i_height = height;
    param.i_fps_num = 25;
    param.i_fps_den = 1;

    param.i_keyint_max = 25;
    param.b_intra_refresh = 1;
    param.b_annexb = 1;
    x264_param_apply_profile(&param, "baseline");
    x264Encoder = x264_encoder_open(&param);
    x264_picture_alloc(&xPic, X264_CSP_I420, width, height);
    xPic.img.plane[0] = picture.data[0];
    xPic.img.plane[1] = picture.data[1];
    xPic.img.plane[2] = picture.data[2];
    picOut = new x264_picture_t;
}

void H264Encoder::x264Encode()
{
    xPic.i_pts = i_pts++;
    x264_encoder_encode(x264Encoder, &nals, &nnal, &xPic, picOut);
}