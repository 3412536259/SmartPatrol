#ifndef I_DEVICE_MANAGER_H
#define I_DEVICE_MANAGER_H
#include <string>
#include "camera_info.h"
#include "device_info.h"
#include "sensor_service.h"

class IDeviceManager{
public:
    virtual ~IDeviceManager() = default;

    virtual DeviceStatus getStatus() = 0; //设备状态获取（含传感器状态）

    virtual RealImage getRealImage(const std::string& camId) = 0; //获取对应摄像头实时图片
    virtual RealImageList getAllRealImage() = 0; //获取所有摄像头实时图片
    virtual void getHistoryImage(const std::string& camId) = 0;
    virtual void getAllHistoryImage() = 0;
    
    virtual UpdateConfigResult configUpdate(const std::string& JsonStr) = 0;

    // =================== 传感器相关接口 ===================
    
    // 读取温湿度数据
    virtual SensorStatusData getTemperatureHumidity() = 0;
    
    // 门锁控制（仅开锁）
    virtual DoorLockOperationResult openDoorLock(const std::string& lockId) = 0;
};

#endif