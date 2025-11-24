#ifndef VIDEO_RECORDER_H
#define VIDEO_RECORDER_H

#include "video_capture.h"
#include "video_storage.h"
#include "logger.h"
// #include "thread_pool.h"
#include <atomic>
#include <thread>
#include <chrono>
#include <string>
#include <sstream>
#include <iomanip>
#include <memory>
#include <ctime>
#include "threadsafe_queue.h"
#include "FrameData.h"
#include "camerainfo.h"
class VideoRecorder {
public:
    VideoRecorder( 
                  std::shared_ptr<CameraInfo> camera_info,
                  std::shared_ptr<ThreadSafeQueue<FrameData>> queue = NULL,
                  const std::string& videoFormat = "mp4"
                );
    //自己管理线程
    bool start();
    //线程池实现
    // bool start(thread_pool& pool,const std::string& rtspUrl, int durationSec);
    void stop();
    bool isRunning() const { return isRunning_; }
    void wait();
    void setDurationSec(int durationSec);
private:
    void recordLoop();


private:
    std::atomic<int> durationSec_;
    std::string videoDir_;
    std::shared_ptr<CameraInfo> camera_info_;
    std::string videoFormat_;
    std::atomic<bool> isRunning_{false};
    std::thread recordThread_;
    std::shared_ptr<ThreadSafeQueue<FrameData>> frameQueue_;
    std::unique_ptr<VideoCapture> capture_;
};

#endif