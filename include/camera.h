#ifndef CAMERA_H
#define CAMERA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <linux/videodev2.h>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libswscale/swscale.h>
#include <libavutil/pixfmt.h>
}

#include <liveMedia/UsageEnvironment.hh>
#include <liveMedia/FramedSource.hh>

#include "H264Encoder.h"
#include "Config.h"


struct buffer
{
    void *start;
    unsigned int len;
};

class Camera
{
  public:
    Camera(int bufferCount);
    ~Camera();
    bool initDev(const char *devName, int width, int height);
    bool readFrame(AVPicture &picDes, AVPixelFormat picFmt, int picWidth, int picHeight);
    int getWidth();
    int getHeight();
    bool startStream();
    bool stopStream();
  private:
    int fd;
    int width;
    int height;
    buffer *buffers;
    int bufferCount;
    int bytesPerLine;
  

};


class CameraFramedSource : public FramedSource
{
  public:
    CameraFramedSource(UsageEnvironment &env);
    virtual ~CameraFramedSource();

  protected:
    virtual void doGetNextFrame();
    virtual unsigned maxFrameSize() const;

  private:
    static void getNextFrame(void *ptr)
    {
        ((CameraFramedSource *)ptr)->getNextFrame1();
    }
    void getNextFrame1();

    Camera *camera;
    H264Encoder *encoder;
    AVPicture picture;
    void *mp_token;

    static int nalIndex;
};
#endif
