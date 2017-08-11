#include "Camera.h"


Camera::Camera(int bufferCount)
{
    fd = -1;
    buffers = NULL;
    width = 0;
    height = 0;
	this->bufferCount = bufferCount;
}
bool Camera::initDev(const char *devName, int width, int height)
{
    struct v4l2_capability cap;
    fd = open(devName, O_RDWR);
    if (-1 == fd)
    {
        printf("Camera::initDev can't open the device\n");
        return false;
    }
    if (ioctl(fd, VIDIOC_QUERYCAP, &cap) == -1)
    {
        printf("Camera::initDev can't VIDIOC_QUERYCAP\n");
        return false;
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
    {
        printf("Camera::initDev This device doesn't support V4L2_CAP_VIDEO_CAPTURE\n");
        return false;
    }
    if (!(cap.capabilities & V4L2_CAP_STREAMING))
    {
        printf("Camera::initDev This device doesn't support V4L2_CAP_STREAMING\n");
        return false;
    }

    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof(struct v4l2_format));

    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = width;
    fmt.fmt.pix.height = height;
    fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;

    if (ioctl(fd, VIDIOC_S_FMT, &fmt) == -1)
    {
        printf("Camera::initDev can't VIDIOC_S_FMT\n");
        return false;
    }
    if (ioctl(fd, VIDIOC_G_FMT, &fmt) == -1)
    {
        printf("Camera::initDev can't VIDIOC_G_FMT\n");
        return false;
    }
    this->width = fmt.fmt.pix.width;
    this->height = fmt.fmt.pix.height;
    this->bytesPerLine = fmt.fmt.pix.bytesperline;

    printf("fmt.fmt.pix.bytesperline:%d\n", fmt.fmt.pix.bytesperline);
    printf("format:%c%c%c%c\n", (fmt.fmt.pix.pixelformat & 0xff),
           ((fmt.fmt.pix.pixelformat >> 8) & 0xff),
           ((fmt.fmt.pix.pixelformat >> 16) & 0xff),
           ((fmt.fmt.pix.pixelformat >> 24) & 0xff));

    struct v4l2_requestbuffers req;
    memset(&req, 0, sizeof(struct v4l2_requestbuffers));

    req.count = bufferCount;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (ioctl(fd, VIDIOC_REQBUFS, &req) == -1)
    {
        printf("Camera::initDev can't VIDIOC_REQBUFS");
        return false;
    }

    buffers = (struct buffer *)calloc(bufferCount, sizeof(struct buffer));
    for (int i = 0; i < bufferCount; i++)
    {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(struct v4l2_buffer));

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        if (ioctl(fd, VIDIOC_QUERYBUF, &buf) == -1)
        {
            printf("Camera::initDev can't VIDIOC_QUERYBUF");
            return false;
        }
        buffers[i].len = buf.length;
        buffers[i].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
        if (MAP_FAILED == buffers[i].start)
        {
            printf("Camera::initDev mmap failed\n");
            return false;
        }
    }
	
    for (int i = 0; i < bufferCount; i++)
    {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        if (ioctl(fd, VIDIOC_QBUF, &buf) == -1) //放入缓存
        {
            printf("Camera::initDev can't VIDIOC_QBUF");
            return false;
        }
    }
    return true;
}

bool Camera::startStream()
{
    enum v4l2_buf_type type;
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(fd, VIDIOC_STREAMON, &type) == -1)
    {
        printf("Camera::startStream can't VIDIOC_STREAMON\n");
        return false;
    }
    return true;
}

bool Camera::stopStream()
{
    enum v4l2_buf_type type;
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == ioctl(fd, VIDIOC_STREAMOFF, &type))
    {
        perror("Camera::stopStream can't VIDIOC_STREAMOFF\n");
        return false;
    }
    return true;
}

int Camera::getWidth()
{
    return width;
}
int Camera::getHeight()
{
    return height;
}

Camera::~Camera()
{
    for (int i = 0; i < bufferCount; i++)
        munmap(buffers[i].start, buffers[i].len);
    free(buffers);
    close(fd);
}



int fill_iobuffer(void * buffer,uint8_t *iobuf, int bufsize){
    int i;
    for(i=0;i<bufsize;i++)
		iobuf[i] = ((unsigned char *) buffer)[i];
    return i;
 }

 bool Camera::readFrame(AVPicture &picDest, AVPixelFormat picFmt, int picWidth, int picHeight)
 {
    struct v4l2_buffer buf;
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    if (ioctl(fd, VIDIOC_DQBUF, &buf) == -1)
    {
        printf("Camera::readFrame can't VIDIOC_DQBUF\n");
        return false;
    }

    AVFormatContext *pFormatCtx = avformat_alloc_context();;
    unsigned char *ioBuffer = (unsigned char *)malloc(sizeof(unsigned char)*buffers[buf.index].len);
    AVIOContext *pb = avio_alloc_context(ioBuffer, buffers[buf.index].len, 0, (unsigned char *)buffers[buf.index].start, fill_iobuffer, NULL, NULL);
    
    pFormatCtx->pb =  pb;
    avformat_open_input(&pFormatCtx, NULL, NULL, NULL);
    avformat_find_stream_info(pFormatCtx,NULL);
    int videoindex = -1;
    for (int i = 0; i < pFormatCtx->nb_streams; i++) 
    if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
    {                  
        videoindex = i;
        break;
    }
    AVCodecContext *pCodecCtx = pFormatCtx->streams[videoindex]->codec;
    printf("picture width   =  %d \n", pCodecCtx->width);
    printf("picture height  =  %d \n", pCodecCtx->height);
    printf("Pixel   Format  =  %d \n", pCodecCtx->pix_fmt);

    AVCodec *pCodec = avcodec_find_decoder(pCodecCtx->codec_id);

    	
    AVFrame *pFrame = av_frame_alloc();
    AVPacket *packet = av_packet_alloc();
   


    SwsContext *img_convert_ctx;
    img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, picWidth, picHeight, picFmt, SWS_BICUBIC, NULL, NULL, NULL);   
    av_read_frame(pFormatCtx, packet);
  
    int ret,got_picture;
    ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);  
    if(ret < 0)
    {
        printf("Camera::readFrame decode error\n");
        return false;
    }
    if(got_picture)
    {
        ret = sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height,  picDest.data, picDest.linesize);  
        if(ret == -1)
        {
            printf("Camera::readFrame can't not open to change to dest image\n");
            return false;
        }
    }


    av_packet_free (&packet);
    av_frame_free (&pFrame);
    free(ioBuffer);
    avformat_free_context(pFormatCtx);
    


    if (ioctl(fd, VIDIOC_QBUF, &buf) == -1) //放回缓存
    {
        printf("Camera::readFrame can't VIDIOC_QBUF\n");
        return false;
    }
    return true;
 }