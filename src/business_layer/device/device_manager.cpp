#include "device_manager.h"
#include "camera_manager.h"

#include <iostream>
#include <chrono>

DeviceManager::DeviceManager()
{
    cameraManager_ = std::make_shared<CameraManager>();
    sensorService_ = std::make_shared<SensorService>();
    controllerService_ = std::make_shared<ControllerService>();
    configUpdater_ = std::make_shared<ConfigUpdater>();
}

DeviceManager::~DeviceManager()
{
    stopServices();
}

bool DeviceManager::initializeSensorService()
{
    if (!sensorService_) {
        std::cerr << "DeviceManager: sensorService_ is null!" << std::endl;
        return false;
    }
    return sensorService_->initialize();
}

bool DeviceManager::initializeControllerService()
{
    if (!controllerService_) {
        std::cerr << "DeviceManager: controllerService_ is null!" << std::endl;
        return false;
    }
    // 添加默认门锁设备
    controllerService_->addDoorLock("door_lock_01", 22);
    return controllerService_->initialize();
}

void DeviceManager::startServices()
{
    if (sensorService_) {
        sensorService_->start();
    }
    if (controllerService_) {
        controllerService_->start();
    }
}

void DeviceManager::stopServices()
{
    if (sensorService_) {
        sensorService_->stop();
    }
    if (controllerService_) {
        controllerService_->stop();
    }
}

DeviceStatus DeviceManager::getStatus()
{
    DeviceStatus deviceStatus;
    deviceStatus.cameraStatusList = cameraManager_->getAllStatus();
    
    // 获取传感器状态
    if (sensorService_) {
        deviceStatus.sensorStatus = sensorService_->getAllSensorStatus();
    }

    return deviceStatus;
}

// =================== 传感器相关实现 ===================

AllSensorStatus DeviceManager::getAllSensorStatus()
{
    AllSensorStatus status;
    if (!sensorService_) {
        std::cerr << "DeviceManager: sensorService_ is null!" << std::endl;
        return status;
    }
    return sensorService_->getAllSensorStatus();
}

// =================== 门锁控制（由控制器服务管理，仅开锁）===================

DoorLockOperationResult DeviceManager::openDoorLock(const std::string& lockId)
{
    DoorLockOperationResult result;
    if (!controllerService_) {
        result.success = false;
        result.message = "controllerService_ is null";
        return result;
    }
    
    // turnOn 对于门锁表示解锁
    bool success = controllerService_->turnOn(lockId);
    result.success = success;
    result.message = success ? "Door lock opened successfully" : "Failed to open door lock";
    return result;
}

void DeviceManager::setAlarmCallback(std::function<void(const std::string&, const std::string&)> callback)
{
    if (sensorService_) {
        sensorService_->setAlarmCallback(callback);
    }
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

    
UpdateConfigResult DeviceManager::configUpdate(const std::string& JsonStr)
{
    UpdateConfigResult result;
    if(!configUpdater_) {
        result.message = "no configUpdater_";
        return result;
    } 

    result = configUpdater_->updateConfig(JsonStr);
    return result;
}
