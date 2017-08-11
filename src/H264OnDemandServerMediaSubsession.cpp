#include "H264OnDemandServerMediaSubsession.h"

H264OnDemandServerMediaSubsession::H264OnDemandServerMediaSubsession(UsageEnvironment &env, FramedSource *source) : OnDemandServerMediaSubsession(env, true)
{
    mp_source = source;
    mp_sdp_line = NULL;
    mp_dummy_rtpsink = NULL;
    m_done = 0;
}

H264OnDemandServerMediaSubsession::~H264OnDemandServerMediaSubsession()
{


}

void H264OnDemandServerMediaSubsession::chkForAuxSDPLine1()
{
    if (mp_dummy_rtpsink->auxSDPLine())
        m_done = 0xff;
    else
    {
        int delay = 100 * 1000; //100ms
        nextTask() = envir().taskScheduler().scheduleDelayedTask(delay, chkForAuxSDPLine, this);
    }
}

const char *H264OnDemandServerMediaSubsession::getAuxSDPLine(RTPSink *sink, FramedSource *source)
{
    if (mp_sdp_line)
        return mp_sdp_line;

    mp_dummy_rtpsink = sink;
    mp_dummy_rtpsink->startPlaying(*source, 0, 0);
    chkForAuxSDPLine(this);
    m_done = 0;
    envir().taskScheduler().doEventLoop(&m_done);
    mp_sdp_line = strdup(mp_dummy_rtpsink->auxSDPLine());
    mp_dummy_rtpsink->stopPlaying();

    return mp_sdp_line;
}

RTPSink *H264OnDemandServerMediaSubsession::createNewRTPSink(Groupsock *rtpsock, unsigned char type, FramedSource *source)
{
    return H264VideoRTPSink::createNew(envir(), rtpsock, type);
}

FramedSource *H264OnDemandServerMediaSubsession::createNewStreamSource(unsigned sid, unsigned &bitrate)
{
    bitrate = 500;
    return H264VideoStreamFramer::createNew(envir(),
                                            mp_source);
}

char const *H264OnDemandServerMediaSubsession::sdpLines()
{
    return fSDPLines = (char *)"m=video 0 RTP/AVP 96\r\n"
                               "c=IN IP4 0.0.0.0\r\n"
                               "b=AS:96\r\n"
                               "a=rtpmap:96 H264/90000\r\n"
                               "a=fmtp:96 packetization-mode=1;profile-level-id=000000;sprop-parameter-sets=H264\r\n"
                               "a=control:track1\r\n";
}
