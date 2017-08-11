#include "Main.h"
using namespace v4l2;
int main()
{
    Camera c(4);
    c.initDev("/dev/video0",480,320);
    c.startStream();
    sleep(2);
    c.stopStream();
    return 0;
}