#include "mqtt_command_dispatcher.h"
#include "mqtt_topics.h"

MqttCommandDispatcher::MqttCommandDispatcher(JobScheduler& scheduler)
    :scheduler_(scheduler){}
void MqttCommandDispatcher::onMessage(const std::string& topic, const std::string& payload)
{
    nlohmann::json j;
    try{
        j = nlohmann::json::parse(payload);
    }catch(...){
        std::cerr << "Invalid JSON: " << payload << std::endl;
        return;
    }
    // 用 MQTT topic 决定任务类型
    if (topic == GET_REAL_IMAGE_TOPIC) {
        handleGetRealImage(j);  //获取实时图片
    }
    else if(topic == GET_ALL_DEVICE_STATUS_TOPIC){
        handleGetAllDeviceStatus(j); //获取所有设备状态
    }
    else if(topic == UPDATE_CONFIG_TOPIC){
        handleConfigUpdate(j); //更新配置文件
    }
    else if(topic == DOOR_LOCK_CONTROL_TOPIC){
        handleOpenDoorLock(j);
    }
    else {
        std::cout << "Unknown topic: " << topic << std::endl;
    }
}

void MqttCommandDispatcher::handleGetRealImage(const nlohmann::json& j)
{
    if (!j.contains("deviceId")) return;

    std::string camId = j["deviceId"];

    auto task = std::make_shared<GetCameraRealImageTask>(camId);
    int id = scheduler_.submit(task, "mqtt");

    std::cout << "Submitted GetRealImageTask id=" << id 
              << " for cam=" << camId << std::endl;
}


void MqttCommandDispatcher::handleGetAllDeviceStatus(const nlohmann::json& j)
{
    auto task = std::make_shared<GetDeviceStatusTask>();
    int id = scheduler_.submit(task, "mqtt");

    std::cout << "submitted GetDeviceStatusTask id=" << id << std::endl;
}

void MqttCommandDispatcher::handleConfigUpdate(const nlohmann::json& j){
    if(!j.contains("new_config_data")) return;
    std::string newConfigJson = j.at("new_config_data").get<std::string>();
    
    auto task = std::make_shared<UpdateConfigTask>(newConfigJson);
    int id = scheduler_.submit(task, "mqtt");

    std::cout<<"Submitted UpdateConfigTask id=" <<id<<std::endl;
}

void MqttCommandDispatcher::handleOpenDoorLock(const nlohmann::json& j)
{
    if (!j.contains("lockId")) return;

    std::string lockId = j["lockId"];

    auto task = std::make_shared<OpenDoorLockTask>(lockId);
    int id = scheduler_.submit(task, "mqtt");

    std::cout << "Submitted OpenDoorLockTask id=" << id 
              << " for lockId=" << lockId << std::endl;
}