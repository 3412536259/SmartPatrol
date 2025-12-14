#ifndef DEVICE_MANAGER_H
#define DEVICE_MANAGER_H

#include "icamera_manager.h"
// #include "isensor_manager.h"
#include "idevice_manager.h"
#include "update_config.h"

class DeviceManager : public IDeviceManager{
public:
    DeviceManager();
    ~DeviceManager();
    DeviceStatus getStatus() override;

    RealImageList getAllRealImage() override;
    RealImage getRealImage(const std::string& camId) override;
    void getAllHistoryImage() override;
    void getHistoryImage(const std::string& camId) override;

    UpdateConfigResult configUpdate(const std::string& JsonStr) override;

private:
    std::shared_ptr<ICameraManager> cameraManager_;
    // std::shared_ptr<IPLCManager> plcManager_;
    // std::shared_ptr<ISensorManager> sensorManager_;
    std::shared_ptr<ConfigUpdater> configUpdater_;
};

#endif