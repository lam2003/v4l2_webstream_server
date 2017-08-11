#include "Camera.h"

using namespace v4l2;

Camera::Camera(int bufferCount)
{
    fd = -1;
    buffers = NULL;
    width = 0;
    height = 0;
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
    fmt.fmt.pix.pixelformat = CAMERA_FMT;

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

    this->bufferCount = bufferCount;
    buffers = (struct buffer *)calloc(bufferCount, sizeof(struct buffer));
    for (int i = 0; i < bufferCount; i++)
    {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(struct v4l2_buffer));

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = bufferCount;
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
    for(int i = 0; i < bufferCount; i++)
        munmap(buffers[i].start, buffers[i].len);
    free(buffers);
    close(fd);
}


bool Camera::readFrame(AVPicture &picDest,AVPixelFormat picFmt,int picWidth,int picHeight)
{
    
    AVPicture picSrc;
    SwsContext *swsCtx;

    struct v4l2_buffer buf;

    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;

    if (ioctl(fd, VIDIOC_DQBUF, &buf) == -1)   //读取
	{
		printf("Camera::readFrame can't VIDIOC_DQBUF\n");
		return false;
	}
    picSrc.data[0] = (unsigned char *) buffers[buf.index].start;
	picSrc.data[1] = picSrc.data[2] = picSrc.data[3] = NULL;
    picSrc.linesize[0] = bytesPerLine;
    for(int i = 1; i < 8; i++)
        picSrc.linesize[i] = 0;
    swsCtx = sws_getContext(width,height,(AVPixelFormat)CAMERA_FMT,picWidth,picHeight,picFmt,SWS_BICUBIC,0,0,0);
    int rs = sws_scale(swsCtx, picSrc.data, picSrc.linesize, 0,
			height, picDest.data, picDest.linesize);
    if(rs == -1)
    {
        printf("Camera::readFrame can't not open to change to dest Image\n");
        return false;
    }
    sws_freeContext(swsCtx);
    if (ioctl(fd, VIDIOC_QBUF, &buf) == -1)    //放回缓存
	{
		printf("Camera::readFrame can't VIDIOC_QBUF\n");
		return false;
	}
    return true;
}