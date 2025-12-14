#include "task.h"
#include "mqtt_service.h"
#include "json.hpp"
#include "ImageProcessor.h"
#include "config_info.h"
#include "config_parser.h"
#include "mqtt_topics.h"

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

// =================== 传感器任务实现 ===================

void GetAllSensorStatusTask::run(TaskContext& ctx)
{
    AllSensorStatus status = ctx.devMgr->getAllSensorStatus();
    nlohmann::json j;
    j["timestamp"] = status.collect_timestamp;
    
    // 传感器状态数组
    nlohmann::json sensors = nlohmann::json::array();
    for (const auto& sensor : status.sensors) {
        nlohmann::json s;
        s["sensorId"] = sensor.sensor_id;
        s["type"] = sensor.sensor_type;
        s["isValid"] = sensor.is_valid;
        s["timestamp"] = sensor.timestamp;
        
        if (sensor.sensor_type == "temperature_humidity") {
            s["temperature"] = sensor.temperature;
            s["humidity"] = sensor.humidity;
        } else {
            s["triggered"] = sensor.triggered;
        }
        sensors.push_back(s);
    }
    j["sensors"] = sensors;
    
    // 门锁状态数组
    nlohmann::json doorLocks = nlohmann::json::array();
    for (const auto& lock : status.doorLocks) {
        nlohmann::json l;
        l["lockId"] = lock.lock_id;
        l["isLocked"] = lock.is_locked;
        l["isValid"] = lock.is_valid;
        l["timestamp"] = lock.timestamp;
        doorLocks.push_back(l);
    }
    j["doorLocks"] = doorLocks;
    
    ctx.publisher->publish(RESULT_GET_ALL_SENSOR_STATUS_TOPIC, j.dump());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void GetTemperatureHumidityTask::run(TaskContext& ctx)
{
    SensorStatusData data = ctx.devMgr->getTemperatureHumidity();
    nlohmann::json j;
    j["sensorId"] = data.sensor_id;
    j["type"] = data.sensor_type;
    j["isValid"] = data.is_valid;
    j["timestamp"] = data.timestamp;
    
    if (data.is_valid) {
        j["temperature"] = data.temperature;
        j["humidity"] = data.humidity;
    } else {
        j["code"] = "no data";
    }
    
    ctx.publisher->publish(RESULT_GET_TEMP_HUMIDITY_TOPIC, j.dump());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

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

void UploadSensorStatusTask::run(TaskContext& ctx)
{
    // 定时上传所有传感器状态
    AllSensorStatus status = ctx.devMgr->getAllSensorStatus();
    nlohmann::json j;
    j["timestamp"] = status.collect_timestamp;
    j["reportType"] = "periodic";  // 标记为定时上报
    
    // 传感器状态数组
    nlohmann::json sensors = nlohmann::json::array();
    for (const auto& sensor : status.sensors) {
        nlohmann::json s;
        s["sensorId"] = sensor.sensor_id;
        s["type"] = sensor.sensor_type;
        s["isValid"] = sensor.is_valid;
        s["timestamp"] = sensor.timestamp;
        
        if (sensor.sensor_type == "temperature_humidity") {
            s["temperature"] = sensor.temperature;
            s["humidity"] = sensor.humidity;
        } else {
            s["triggered"] = sensor.triggered;
        }
        sensors.push_back(s);
    }
    j["sensors"] = sensors;
    
    ctx.publisher->publish(SENSOR_STATUS_UPLOAD_TOPIC, j.dump());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void GetSensorDataTask::run(TaskContext& ctx)
{
    // 根据sensorId获取指定传感器数据
    AllSensorStatus allStatus = ctx.devMgr->getAllSensorStatus();
    nlohmann::json j;
    j["sensorId"] = sensorId_;
    
    bool found = false;
    for (const auto& sensor : allStatus.sensors) {
        if (sensor.sensor_id == sensorId_) {
            j["type"] = sensor.sensor_type;
            j["isValid"] = sensor.is_valid;
            j["timestamp"] = sensor.timestamp;
            
            if (sensor.sensor_type == "temperature_humidity") {
                j["temperature"] = sensor.temperature;
                j["humidity"] = sensor.humidity;
            } else {
                j["triggered"] = sensor.triggered;
            }
            found = true;
            break;
        }
    }
    
    if (!found) {
        j["code"] = "sensor not found";
        j["isValid"] = false;
    }
    
    ctx.publisher->publish(RESULT_GET_SENSOR_DATA_TOPIC, j.dump());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

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
        ctx.publisher->publish(RESULT_UPDATE_CONFIG, ack.dump());
    }
    UpdateConfigResult res = ctx.devMgr->configUpdate(JsonStr_);
    nlohmann::json j;
    if(!res.isSuccess) j["code"] = "update failed";
    j["message"] = res.message;
    ctx.publisher->publish(RESULT_UPDATE_CONFIG, j.dump());
    std::this_thread::sleep_for(std::chrono::seconds(1));
}