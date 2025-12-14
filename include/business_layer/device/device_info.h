#ifndef DEVICE_INFO_H
#define DEVICE_INFO_H

#include "camera_info.h"
#include "sensor_service.h"
#include "controller_service.h"
#include <vector>

// 设备状态汇总
struct DeviceStatus
{
    CameraStatusList cameraStatusList;
    AllSensorStatus sensorStatus;              // 传感器状态
};

struct RealImage
{
    FrameData frame;
    std::string sourceCameraId;
    bool integrity = false;
};

struct RealImageList
{
    std::vector<RealImage> RealImages;
    bool success = true;
};

struct UpdateConfigResult{
    bool isSuccess = false;
    std::string message;
};

// 传感器操作结果
struct SensorOperationResult {
    bool success = false;
    std::string message;
    SensorStatusData data;
};

// 门锁操作结果（仅开锁）
struct DoorLockOperationResult {
    bool success = false;
    std::string message;
};

#endif