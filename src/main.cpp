
#include <liveMedia/liveMedia.hh>
#include <liveMedia/BasicUsageEnvironment.hh>
#include <liveMedia/UsageEnvironment.hh>
#include "Main.h"

UsageEnvironment *env;
static void announceStream(RTSPServer *rtspServer, ServerMediaSession *sms, char const *streamName, char const *inputFileName = "Live");
int main()
{
    TaskScheduler *scheduler = BasicTaskScheduler::createNew();
    env = BasicUsageEnvironment::createNew(*scheduler);
    UserAuthenticationDatabase *authDB = NULL;

    RTSPServer *rtspServer = RTSPServer::createNew(*env, 8554, authDB);
    if (rtspServer == NULL)
    {
        *env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
        exit(1);
    }
    char const *descriptionString =
        "Session streamed by \"testOnDemandRTSPServer\"";
    char const *streamName = "cathy_video0";
    ServerMediaSession *sms = ServerMediaSession::createNew(*env, streamName, streamName, descriptionString);

    sms->addSubsession(new H264OnDemandServerMediaSubsession(*env, new CameraFramedSource(*env)));
    rtspServer->addServerMediaSession(sms);
    announceStream(rtspServer, sms, streamName);
    env->taskScheduler().doEventLoop();             //does not return

    return 0;
}
static void announceStream(RTSPServer *rtspServer, ServerMediaSession *sms,
                           char const *streamName, char const *inputFileName)
{
    char *url = rtspServer->rtspURL(sms);
    UsageEnvironment &env = rtspServer->envir();
    env << "\n\"" << streamName << "\" stream, from the file \""
        << inputFileName << "\"\n";
    env << "Play this stream using the URL \"" << url << "\"\n";
    delete[] url;
}