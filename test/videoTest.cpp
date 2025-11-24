#include "camera_service.h"
#include "logger.h"
#include <string>
#include <iostream>
int main()
{
    auto info1 = std::make_shared<CameraInfo>("camera01","rtsp://krdn:wm6tnn@192.168.1.110:554/Streaming/Channels/101");
    auto info2 = std::make_shared<CameraInfo>("camera02","rtsp://krdn:wm6tnn@192.168.1.110:554/Streaming/Channels/101");
    auto queue = std::make_shared<ThreadSafeQueue<FrameData>>(); 
    CameraService camera1(info1,queue);
    CameraService camera2(info2,queue);
    std::this_thread::sleep_for(std::chrono::seconds(30));
    camera1.stopRecording();
    std::this_thread::sleep_for(std::chrono::minutes(2));
}