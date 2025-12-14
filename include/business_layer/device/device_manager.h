#ifndef DEVICE_MANAGER_H
#define DEVICE_MANAGER_H

#include "icamera_manager.h"
#include "idevice_manager.h"
#include "update_config.h"
#include "sensor_service.h"
#include "controller_service.h"

class DeviceManager : public IDeviceManager{
public:
    DeviceManager();
    ~DeviceManager();
    
    // 初始化服务
    bool initializeSensorService();
    bool initializeControllerService();
    void startServices();
    void stopServices();
    
    DeviceStatus getStatus() override;

    RealImageList getAllRealImage() override;
    RealImage getRealImage(const std::string& camId) override;
    void getAllHistoryImage() override;
    void getHistoryImage(const std::string& camId) override;

    UpdateConfigResult configUpdate(const std::string& JsonStr) override;
    
    // =================== 传感器相关实现 ===================
    
    // 获取所有传感器状态（用于定时上报）
    AllSensorStatus getAllSensorStatus() override;
    
    // 读取温湿度（主动读取）
    SensorStatusData getTemperatureHumidity() override;
    
    // =================== 门锁控制（由控制器服务管理，仅开锁）===================
    DoorLockOperationResult openDoorLock(const std::string& lockId) override;
    
    // 设置告警回调
    void setAlarmCallback(std::function<void(const std::string&, const std::string&)> callback);

private:
    std::shared_ptr<ICameraManager> cameraManager_;
    std::shared_ptr<ISensorService> sensorService_;
    std::shared_ptr<IControllerService> controllerService_;
    std::shared_ptr<ConfigUpdater> configUpdater_;
};

#endif