#include "video_recorder.h"
#include "SegmentManager.h"
#include "TimestampAdjuster.h"


VideoRecorder::VideoRecorder(
                            std::shared_ptr<CameraInfo> camera_info,
                            std::shared_ptr<ThreadSafeQueue<FrameData>> queue,
                            const std::string& videoFormat 
                            )
    : camera_info_(camera_info), frameQueue_(queue),videoFormat_(videoFormat),  isRunning_(false),durationSec_(60){
        videoDir_ = "/home/ztl/workspace/SmartPatrol/SmartPatrol/videos/"+camera_info_->camera_name;
        capture_ = std::make_unique<VideoCapture>(camera_info_->camera_url);
    }


bool VideoRecorder::start() {
    if (isRunning_) {
        LOG_WARNING("Recorder is already running");
        return false;
    }

    isRunning_ = true;
    recordThread_ = std::thread(&VideoRecorder::recordLoop, this);
    //recordThread_.detach();
    return true;
}
/*
bool VideoRecorder::start(thread_pool& pool,const std::string& rtspUrl, int durationSec){
    if (isRunning_) {
        LOG_WARNING("Recorder is already running");
        return false;
    }

    isRunning_ = true;
    pool.submit([this,rtspUrl,durationSec]{
        this->recordLoop(rtspUrl,durationSec);
    });
    return true;
}*/
void VideoRecorder::stop() {
    if (!isRunning_) return;
    isRunning_ = false;
    LOG_INFO("Stop signal sent to recorder");
    wait();
}
void VideoRecorder::wait() {
    if (recordThread_.joinable()) {
        recordThread_.join();
    }
}
void VideoRecorder::setDurationSec(int durationSec)
{
    durationSec_ = durationSec;
}

void VideoRecorder::recordLoop() {
    LOG_INFO("=== VideoRecorder Started ===");

    if (!capture_->openStream()) {
        LOG_ERROR("Failed to open RTSP stream");
        isRunning_ = false;
        return;
    }
    AVFrame* frame = av_frame_alloc();
    AVPacket* packet = av_packet_alloc();
    if (!packet) {
        LOG_ERROR("Failed to allocate AVPacket");
        capture_->closeStream();
        isRunning_ = false;
        return;
    }

    SegmentManager segMgr(videoDir_, videoFormat_, durationSec_);
    TimestampAdjuster tsAdjuster;

    auto storage = segMgr.createSegment(capture_->getFormatContext());
    if (!storage) {
        LOG_ERROR("Failed to create first segment");
        av_packet_free(&packet);
        capture_->closeStream();
        isRunning_ = false;
        return;
    }

    int totalPackets = 0;

    while (isRunning_) {
        if (segMgr.needNewSegment()) {
            storage->closeStorage();
            storage = segMgr.createSegment(capture_->getFormatContext());
            tsAdjuster.reset();
            if (!storage) break;
        }

        if (!capture_->readPacket(packet)) {
            LOG_WARNING("Failed to read packet, retrying...");
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }

        tsAdjuster.adjust(packet);

        if (!storage->writePacket(packet)) {
            LOG_WARNING("Failed to write packet");
        } else {
            totalPackets++;
            if (totalPackets % 200 == 0)
                LOG_INFO("Written " + std::to_string(totalPackets) + " packets total");
        }

        //对关键帧入队
        if (packet->flags & AV_PKT_FLAG_KEY) {
            if (capture_->decodePacket(packet, frame) >= 0) {
                FrameData data;
                data.frame = std::shared_ptr<AVFrame>(
                av_frame_clone(frame), [](AVFrame* f) { av_frame_free(&f); });
                data.timestamp = av_gettime_relative();
                data.sourceCamera = camera_info_->camera_name;
                frameQueue_->try_push(data);
            }
        }
       
        av_packet_unref(packet);
    }

    if (storage) storage->closeStorage();
    av_packet_free(&packet);
    av_frame_free(&frame);
    capture_->closeStream();
    isRunning_ = false;
    LOG_INFO("=== VideoRecorder Finished ===");
}