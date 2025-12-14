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
#endif