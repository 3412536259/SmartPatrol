#include "task.h"
#include "mqtt_service.h"
#include "json.hpp"
#include "ImageProcessor.h"
#include "config_info.h"
#include "config_parser.h"
#include "mqtt_topics.h"
//摄像头---------------------------
void GetCameraRealImageTask::run(TaskContext& ctx)
{
    RealImage image = ctx.devMgr->getRealImage(camId_);
    image_buffer_t out_image;
    std::vector<unsigned char> outJpeg;
    if(image.integrity)
    {
        ImageProcessor::avframeToRGB(image.frame.frame.get(),640,640,&out_image);
        ImageProcessor::compressToJpeg(&out_image,outJpeg);
        std::string imageBase64 = ImageProcessor::jpegToBase64(outJpeg);
        nlohmann::json j;
        j["cameraId"] =  camId_;
        j["image"] = imageBase64;
        ctx.publisher->publish(RESULT_GET_REAL_IMAGE_TOPIC, j.dump());
    }
    else{
        nlohmann::json j;
        j["code"] = "no image";
        ctx.publisher->publish(RESULT_GET_REAL_IMAGE_TOPIC, j.dump());
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

//摄像头---------------------------

// =================== 传感器任务实现 ===================

void OpenDoorLockTask::run(TaskContext& ctx)
{
    DoorLockOperationResult result = ctx.devMgr->openDoorLock(lockId_);
    
    nlohmann::json j;
    j["lockId"] = lockId_;
    j["success"] = result.success;
    j["message"] = result.message;
    
    ctx.publisher->publish(RESULT_DOOR_LOCK_CONTROL_TOPIC, j.dump());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

// =================== 传感器任务实现 ===================

void GetDeviceStatusTask::run(TaskContext& ctx)
{
    DeviceStatus status = ctx.devMgr->getStatus();
    nlohmann::json j;
    auto& device = j["device"];

    // 摄像头状态
    for(const auto& camStatus : status.cameraStatusList.cameraStatus){
        device["cameras"].push_back({
            {"cameraId", camStatus.camera_id},
            {"onlineStatus", camStatus.online_status == CameraOnlineStatus::ONLINE ? "ONLINE" : "OFFLINE"}
        });
    }

    // 传感器状态
    nlohmann::json sensors = nlohmann::json::array();
    for (const auto& sensor : status.sensorStatus.sensors) {
        nlohmann::json s;
        s["sensorId"] = sensor.sensor_id;
        s["type"] = sensor.sensor_type;
        s["isValid"] = sensor.is_valid;
        
        if (sensor.sensor_type == "temperature_humidity") {
            s["temperature"] = sensor.temperature;
            s["humidity"] = sensor.humidity;
        } else {
            s["triggered"] = sensor.triggered;
        }
        sensors.push_back(s);
    }
    device["sensors"] = sensors;

    ctx.publisher->publish(RESULT_GET_ALL_DEVICE_STATUS_TOPIC, j.dump());
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

void UpdateConfigTask::run(TaskContext& ctx){
    {
        nlohmann::json ack;
        ack["success"] = true;
        ctx.publisher->publish(RESULT_UPDATE_CONFIG_TOPIC, ack.dump());
    }
    UpdateConfigResult res = ctx.devMgr->configUpdate(JsonStr_);
    nlohmann::json j;
    if(!res.isSuccess) j["code"] = "update failed";
    j["message"] = res.message;
    ctx.publisher->publish(RESULT_UPDATE_CONFIG_TOPIC, j.dump());
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

// =================== 传感器报警任务实现 ===================
void SensorAlarmTask::run(TaskContext& ctx)
{
    // 获取当前时间戳
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
    
    // 构建报警消息JSON
    nlohmann::json alarmMsg;
    alarmMsg["alarmType"] = alarmType_;       // 报警类型
    alarmMsg["sensorId"] = sensorId_;         // 传感器ID
    alarmMsg["reason"] = alarmReason_;        // 报警原因
    alarmMsg["timestamp"] = timestamp;        // 报警时间戳
    alarmMsg["status"] = "ALARM";             // 状态：报警中
    
    // 添加传感器数据（如果有）
    if (!sensorData_.empty()) {
        alarmMsg["sensorData"] = sensorData_;
    }
    
    // 发布报警消息到MQTT
    ctx.publisher->publish(SENSOR_ALARM_TOPIC, alarmMsg.dump());
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}