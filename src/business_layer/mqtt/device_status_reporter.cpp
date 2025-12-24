#include "device_status_reporter.h"
#include <chrono>
#include <iostream>
#include <json.hpp>
DeviceStatusReporter::DeviceStatusReporter(IDeviceManager* devMgr,
                                           ITaskResultPublisher* mqttPublisher,
                                           ITaskResultPublisher* httpPublisher)
    : devMgr_(devMgr), mqttPublisher_(mqttPublisher),httpPublisher_(httpPublisher) {}

DeviceStatusReporter::~DeviceStatusReporter() {
    stopAutoReport();
}

void DeviceStatusReporter::startAutoReport(const std::string& topic, int intervalSec) {
    if (running_) return;

    topic_ = topic;
    intervalSec_ = intervalSec;
    running_ = true;

    worker_ = std::thread(&DeviceStatusReporter::autoReportLoop, this);
}

void DeviceStatusReporter::stopAutoReport() {
    running_ = false;
    if (worker_.joinable()) worker_.join();
}

void DeviceStatusReporter::autoReportLoop() {
    while (running_) {
        reportStatus(topic_);
        std::this_thread::sleep_for(std::chrono::seconds(intervalSec_));
    }
}

void DeviceStatusReporter::reportStatus(const std::string& topic)
{
    DeviceStatus status = devMgr_->getStatus();
    nlohmann::json j;
    auto& device = j["device"];

    // Cameras
    for (const auto& cs : status.cameraStatusList.cameraStatus) {
        device["cameras"].push_back({
            {"cameraId", cs.camera_id},
            {"onlineStatus", cs.online_status == CameraOnlineStatus::ONLINE ? "ONLINE" : "OFFLINE"}
        });
    }

    // Sensors
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

    mqttPublisher_->publish(topic, j.dump());
    httpPublisher_->publish(topic, j.dump());

}
