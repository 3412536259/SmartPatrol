#include "WebController.h"
#include <iostream>
#include "ImageProcessor.h"
#include "task.h"
#include "task_result_publisher.h"

WebController::WebController(const std::string& /*configPath*/, IDeviceManager* devMgr, JobScheduler* scheduler)
    : devMgr_(devMgr), scheduler_(scheduler)
{
}

json WebController::handleJson(const json& payload)
{
    json resp;
    resp["success"] = false;
    resp["error"] = "use handleHttp for routing";
    return resp;
}

json WebController::handleHttp(const std::string& path, const json& payload)
{
    json resp;

    if (!scheduler_) {
        resp["success"] = false;
        resp["error"] = "scheduler not available";
        return resp;
    }

    // -------------------------------
    // 1) Camera getRealImage  → 提交任务
    // -------------------------------
    if (path.find("/device/camera") != std::string::npos) {
        std::string camId = payload.value("cameraId", payload.value("deviceId", ""));
        if (camId.empty()) {
            resp["success"] = false;
            resp["error"] = "no camera id";
            return resp;
        }

        auto task = std::make_shared<GetCameraRealImageTask>(camId);
        int id = scheduler_->submit(task, "http");

        resp["success"] = true;
        resp["task_id"] = id;
        return resp;
    }

    // 获取全部状态
    if(path.find("/device/getAll") != std::string::npos) {
        auto task = std::make_shared<GetDeviceStatusTask>();
        int id = scheduler_->submit(task, "http");
        resp["success"] = true;
        resp["task_id"] = id;
        return resp;
    }

    //更新配置文件
    if(path.find("/device/config") != std::string::npos){
        std::string newConfigJson = payload.at("new_config_data").get<std::string>();
        auto task = std::make_shared<UpdateConfigTask>(newConfigJson);
        int id = scheduler_->submit(task, "http");
        resp["success"] = true;
        resp["task_id"] = id;
        return resp;
    }

    // -------------------------------
    // 4) 传感器报警任务
    // -------------------------------
    if(path.find("/device/alarm") != std::string::npos){
        std::string alarmType = payload.value("alarmType", "");
        std::string sensorId = payload.value("sensorId", "");
        std::string alarmReason = payload.value("reason", "");
        
        if(alarmType.empty() || sensorId.empty()) {
            resp["success"] = false;
            resp["error"] = "missing alarmType or sensorId";
            return resp;
        }
        
        // 可选的传感器数据
        nlohmann::json sensorData = payload.value("sensorData", nlohmann::json::object());
        
        auto task = std::make_shared<SensorAlarmTask>(alarmType, sensorId, alarmReason, sensorData);
        int id = scheduler_->submit(task, "http");
        resp["success"] = true;
        resp["task_id"] = id;
        return resp;
    }

    // -------------------------------
    // 5) 其它未知路径，统一接受但不执行任务
    // -------------------------------
    resp["success"] = true;
    resp["note"] = "accepted";
    return resp;
}
