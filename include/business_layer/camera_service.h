#ifndef CAMERA_SERVICE_H
#define CAMERA_SERVICE_H
#include <memory>
#include <thread>
#include <atomic>
#include <functional>
#include <vector>
#include <string>
#include "logger.h"
#include "video_capture.h"
#include "video_recorder.h"
#include "threadsafe_queue.h"
#include "camerainfo.h"

class CameraService {
public:
    CameraService(std::shared_ptr<CameraInfo> camera_info,std::shared_ptr<ThreadSafeQueue<FrameData>>frame_queue);
    ~CameraService();

    void startRecording();
    void stopRecording();
    bool isRecording() const;
    bool isOnline() const;
private:

    std::shared_ptr<CameraInfo> camera_info_;
    std::shared_ptr<VideoRecorder> recorder_;
    std::thread processing_thread_;
    std::shared_ptr<ThreadSafeQueue<FrameData>> frameQueue_;
    std::atomic<bool> recording_{false};
    std::atomic<bool> online_{false};
};

#endif