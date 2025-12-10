#include "device_manager.h"
#include "camera_manager.h"

#include <iostream>
DeviceManager::DeviceManager()
{
    cameraManager_ = std::make_shared<CameraManager>();
    // sensorManager_ = std::make_shared<SensorManager>();
}

DeviceManager::~DeviceManager()
{}

DeviceStatus DeviceManager::getStatus()
{
    DeviceStatus deviceStatus;
    deviceStatus.cameraStatus_ = cameraManager_->getAllStatus();
    for(auto& kv : deviceStatus.cameraStatus_)
    {
        std::cout << kv.camera_id << kv.online_status << std::endl;
    }
    return deviceStatus;
}

RealImageList DeviceManager::getAllRealImage()
{
    RealImageList list;
    RealImage image;
    if(!cameraManager_)
    {
        list.success = false;
        return list;
    }
    auto allFrames = cameraManager_->getAllLastKeyFrames();

    for(auto& kv : allFrames)
    {
        std::string id = kv.first;
        const FrameData& frame = kv.second;
        image.integrity = true;
        image.frame = frame;
        image.sourceCameraId = id;
        list.RealImages.push_back(image);
        // TODO: 上传云端或回调 UI
        // cloudUploader.uploadRealImage(id, frame);
 
    }
    return list;
}

RealImage DeviceManager::getRealImage(const std::string& camId)
{
    RealImage realImage;
    if (!cameraManager_) {
        std::cerr << "DeviceManager: cameraManager_ is null!" << std::endl;
        return realImage;
    }
    CameraStaticInfo info;
    info.camera_id = camId;
    FrameData frame;
    
    realImage.sourceCameraId = camId;
    // 调用 CameraManager 获取关键帧
    bool ok = cameraManager_->getCameraLastKeyFrame(info, frame);
    if (!ok) {
        std::cerr << "DeviceManager: failed to get real image for camera "<< info.camera_id << std::endl;
        return realImage;
    }
    realImage.frame = frame;
    realImage.integrity = true;
    // TODO：把 frame 传递到云端 或者回调给上层
    // 示例（你之后自己替换上传函数）：
    // cloudUploader_.uploadRealImage(id, frame);

    std::cout << "DeviceManager: Real image retrieved for camera " 
              << info.camera_id << ", timestamp=" << frame.timestamp << std::endl;

    return realImage;
}

void DeviceManager::getAllHistoryImage()
{

}

void DeviceManager::getHistoryImage(const std::string& camId)
{

}
void DeviceManager::operateCamera()
{

}
void DeviceManager::operatePlc(const std::string &deviceId, const std::string &cmd)
{
    // OperateResult res = plcManager_->operate(deviceId,cmd);
    // TODO：把 res 传递到云端 或者回调给上层
    // 示例（你之后自己替换上传函数）：
    // cloudUploader_.uploadRealImage(deviceId, res);
}
    
void DeviceManager::updateConfig()
{

}
