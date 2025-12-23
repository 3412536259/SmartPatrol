#ifndef TASK_H
#define TASK_H

#include "itask.h"
#include <iostream>
#include <thread>
#include "json.hpp"

class GetCameraRealImageTask : public ITask
{
public:
    GetCameraRealImageTask(std::string camId)
        :camId_(camId){}
    std::string name() const override{return "GetCameraRealImage";}
    void run(TaskContext& ctx)override;

private:
    std::string camId_;
}; 

// 门锁控制任务（仅开锁）
class OpenDoorLockTask : public ITask
{
public:
    OpenDoorLockTask(const std::string& lockId)
        : lockId_(lockId) {}
    std::string name() const override { return "OpenDoorLock"; }
    void run(TaskContext& ctx) override;
private:
    std::string lockId_;
};


class GetDeviceStatusTask : public ITask
{
public:
    GetDeviceStatusTask() {}
    std::string name() const override { return "GetDeviceStatus";}
    void run(TaskContext& ctx) override;
};

class UpdateConfigTask : public ITask
{
public:
    UpdateConfigTask(const std::string& JsonStr) : JsonStr_(JsonStr) {}
    std::string name() const override { return "UpdateConfig"; }
    void run(TaskContext& ctx) override;
private:
    std::string JsonStr_;
};

// 传感器报警任务（当传感器异常时上传数据和状态）
class SensorAlarmTask : public ITask
{
public:
    // alarmType: 报警类型 (如 "infrared", "water_immersion", "smoke", "temperature_humidity")
    // sensorId: 传感器ID
    // alarmReason: 报警原因描述
    // sensorData: 传感器相关数据（可选，如温度、湿度等）
    SensorAlarmTask(const std::string& alarmType, 
                    const std::string& sensorId,
                    const std::string& alarmReason,
                    const nlohmann::json& sensorData = nlohmann::json::object())
        : alarmType_(alarmType)
        , sensorId_(sensorId)
        , alarmReason_(alarmReason)
        , sensorData_(sensorData) {}
    
    std::string name() const override { return "SensorAlarm"; }
    void run(TaskContext& ctx) override;

private:
    std::string alarmType_;      // 报警类型
    std::string sensorId_;       // 传感器ID
    std::string alarmReason_;    // 报警原因
    nlohmann::json sensorData_;  // 传感器数据（温度、湿度、触发状态等）
};

#endif