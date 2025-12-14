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

// 定时上传传感器状态任务（用于定时任务）
class UploadSensorStatusTask : public ITask
{
public:
    UploadSensorStatusTask() {}
    std::string name() const override { return "UploadSensorStatus"; }
    void run(TaskContext& ctx) override;
};

class GetSensorDataTask : public ITask
{
public:
    GetSensorDataTask(std::string sensorId)
        : sensorId_(sensorId) {}
    std::string name() const override { return "GetSensorData"; }
    void run(TaskContext& ctx) override;
private:
    std::string sensorId_;
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
#endif