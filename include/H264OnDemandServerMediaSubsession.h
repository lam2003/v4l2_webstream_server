#ifndef H264ONDEMANDSERVERMEDIASUBSESSION_H
#define H264ONDEMANDSERVERMEDIASUBSESSION_H

#include <liveMedia/OnDemandServerMediaSubsession.hh>
#include <liveMedia/H264VideoStreamFramer.hh>
#include <liveMedia/H264VideoRTPSink.hh>

#include "Camera.h"

class H264OnDemandServerMediaSubsession : public OnDemandServerMediaSubsession
{
  public:
    H264OnDemandServerMediaSubsession(UsageEnvironment &env, FramedSource *source);
    virtual ~H264OnDemandServerMediaSubsession();

  protected:
    virtual const char *getAuxSDPLine(RTPSink *sink, FramedSource *source);
    virtual RTPSink *createNewRTPSink(Groupsock *rtpsock, unsigned char type, FramedSource *source);
    virtual FramedSource *createNewStreamSource(unsigned sid, unsigned &bitrate);
    virtual char const *sdpLines();

  private:
    static void afterPlayingDummy(void *ptr)
    {
        ((H264OnDemandServerMediaSubsession *)ptr)->m_done = 0xff;
    }
    static void chkForAuxSDPLine(void *ptr)
    {
        ((H264OnDemandServerMediaSubsession *)ptr)->chkForAuxSDPLine1();
    }
    void chkForAuxSDPLine1();

    FramedSource *mp_source;
    char *mp_sdp_line;
    RTPSink *mp_dummy_rtpsink;
    char m_done;
};

#endif