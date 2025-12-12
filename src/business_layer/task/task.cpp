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


void GetSensorDataTask::run(TaskContext& ctx)
{
    // RealSensorData rsd = ctx.devMgr->getSensorData(sensorId_);
    // const SensorData& data = rsd.data;
    // nlohmann::json j;
    // j["sensorId"] = sensorId_;
    // j["type"] = data.type;
    // j["status"] = to_string(data.status);

    // if(data.status == SensorStatus::NORMAL)
    // {
    //     if (data.type == "modbus") {
    //         j["temperature"] = data.temperature;
    //         j["humidity"] = data.humidity;
    //     }
    //     if (data.type == "gpio" || data.type == "custom") {
    //         j["value"] = data.value;
    //     }
    // }
    // else
    // {
    //     j["code"] = "no data";
    // }
    // ctx.publisher->publish(RESULT_GET_SENSOR_DATA_TOPIC, j.dump());
    // std::this_thread::sleep_for(std::chrono::seconds(1));
}

void GetDeviceStatusTask::run(TaskContext& ctx)
{
    DeviceStatus status = ctx.devMgr->getStatus();
    nlohmann::json j;
    auto& device = j["device"];

    for(const auto& camStatus : status.cameraStatusList.cameraStatus){
        device["cameras"].push_back({
            {"cameraId",camStatus.camera_id},
            {"onlineStatus",camStatus.online_status == CameraOnlineStatus::ONLINE ? "ONLINE" : "OFFLINE"}
        });
    }

    // for (const auto& sensor : status.sensorStatus_.sensors) {
    //     nlohmann::json s;
    //     s["id"] = sensor.id;
    //     s["type"] = sensor.type;
    //     s["status"] = to_string(sensor.status); 

    //     if (sensor.status == SensorStatus::NORMAL) {
    //         if (sensor.type == "modbus") {
    //             s["temperature"] = sensor.temperature;
    //             s["humidity"] = sensor.humidity;
    //         } else if (sensor.type == "gpio" || sensor.type == "custom") {
    //             s["value"] = sensor.value;
    //         }
    //     } else {
    //         s["code"] = "no data";
    //     }
    //     device["sensor"].push_back(s);
    // }
    ctx.publisher->publish(RESULT_GET_ALL_DEVICE_STATUS_TOPIC, j.dump());
    std::this_thread::sleep_for(std::chrono::seconds(1));
}
