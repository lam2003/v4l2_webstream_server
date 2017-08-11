#include "Camera.h"

using namespace v4l2;

int CameraFramedSource::nalIndex = 0;

CameraFramedSource::CameraFramedSource(UsageEnvironment &env) : FramedSource(env)
{
    camera = new Camera(4);
    encoder = new H264Encoder();
    camera->initDev(CAMERA_DEV_NAME,CAMERA_WIDTH,CAMERA_WIDTH);
    avpicture_alloc(&picture,AV_PIX_FMT_YUV420P,camera->getWidth(),camera->getHeight());
    camera->startStream();
    encoder->x264Init(picture,camera->getWidth(),camera->getHeight());
}

void CameraFramedSource::doGetNextFrame()
{
    if(nalIndex == encoder->nnal)
    {
        camera->readFrame(picture, AV_PIX_FMT_YUV420P, camera->getWidth(),
				camera->getHeight());
        encoder->x264Encode();
        nalIndex = 0;
        gettimeofday(&fPresentationTime, NULL);
    }
    memmove(fTo, encoder->nals[nalIndex].p_payload,
			encoder->nals[nalIndex].i_payload);
    fFrameSize = encoder->nals[nalIndex].i_payload;
    nalIndex++;
    afterGetting(this);
}
CameraFramedSource::~CameraFramedSource()
{
    delete camera;
    delete encoder;
    avpicture_free(picture);
}
