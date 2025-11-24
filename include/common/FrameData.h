#ifndef FRAME_DATA_H
#define FRAME_DATA_H
extern "C" {
#include <libavutil/frame.h>
}
#include <memory>
#include <string>
struct FrameData {
    std::shared_ptr<AVFrame> frame;
    std::string sourceCamera = "NULL";
    int64_t timestamp = 0;
};

#endif