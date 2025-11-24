#include "camera_service.h"

CameraService::CameraService(std::shared_ptr<CameraInfo> camera_info,std::shared_ptr<ThreadSafeQueue<FrameData>>frame_queue)
    :camera_info_(camera_info),frameQueue_(frame_queue),recording_(false)
{
    startRecording();
}
CameraService::~CameraService()
{
    if (recorder_) {
        recorder_->stop();
        recorder_.reset();   // 提前释放
    }
}
    
void CameraService::startRecording()
{
    if(isRecording())
    {
        LOG_INFO("this camera is already recording.");
        return;
    }
    recorder_ = std::make_shared<VideoRecorder>(camera_info_,frameQueue_,"mp4");
    recorder_->start();
    recording_ = true;
}
void CameraService::stopRecording()
{
    if(!isRecording())
    {
        LOG_INFO("this camera is already close recording.");
        return;
    }
    recorder_->stop();
    recording_ = false;
}
bool CameraService::isRecording() const
{
    return recording_;
}
bool CameraService::isOnline() const
{
    return online_;
}