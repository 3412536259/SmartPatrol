#ifndef SENSOR_SERVICE_H
#define SENSOR_SERVICE_H

#include <memory>
#include <thread>
#include <atomic>
#include <functional>
#include <vector>
#include <string>
#include <mutex>

// 传感器数据结构（用于状态上报）
struct SensorStatusData {
    std::string sensor_id;
    std::string sensor_type;    // "infrared", "water_immersion", "smoke", "temperature_humidity"
    bool triggered = false;     // GPIO传感器触发状态
    float temperature = 0.0f;   // 温湿度传感器温度
    float humidity = 0.0f;      // 温湿度传感器湿度
    long timestamp = 0;
    bool is_valid = false;      // 数据是否有效
};

// 所有传感器状态汇总（仅包含传感器，门锁在控制器中管理）
struct AllSensorStatus {
    std::vector<SensorStatusData> sensors;
    long collect_timestamp = 0;
};

// 传感器服务接口
// 红外、水浸、烟感：后台监听，触发告警
// 温湿度：主动读取数据
class ISensorService {
public:
    virtual ~ISensorService() = default;
    virtual bool initialize() = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual bool isRunning() const = 0;
    
    // 读取所有传感器状态（用于定时上报）
    virtual AllSensorStatus getAllSensorStatus() = 0;
    
    // 读取温湿度（主动读取）
    virtual SensorStatusData readTemperatureHumidity() = 0;
    
    // 告警回调（红外、水浸、烟感触发时调用）
    virtual void setAlarmCallback(std::function<void(const std::string& alarm_type, 
                                                   const std::string& reason)> callback) = 0;
};

// 传感器服务实现类
// 红外、水浸、烟感：后台监听，触发告警
// 温湿度：主动读取数据
class SensorService : public ISensorService {
public:
    SensorService();
    ~SensorService();
    
    bool initialize() override;
    void start() override;
    void stop() override;
    bool isRunning() const override;
    
    // 读取所有传感器状态（用于定时上报）
    AllSensorStatus getAllSensorStatus() override;
    
    // 读取温湿度（主动读取）
    SensorStatusData readTemperatureHumidity() override;
    
    // 告警回调
    void setAlarmCallback(std::function<void(const std::string& alarm_type, 
                                           const std::string& reason)> callback) override;
    
private:
    void monitoringLoop();
    void checkAndTriggerAlarm(const SensorStatusData& data);
    long getCurrentTimestamp();
    
    // GPIO操作（模拟/实际）
    bool readGPIOPin(int gpio_pin);
    bool initGPIOInput(int gpio_pin);
    
    std::thread monitoring_thread_;
    std::atomic<bool> running_{false};
    std::function<void(const std::string&, const std::string&)> alarm_callback_;
    
    mutable std::mutex status_mutex_;
    
    // 固定配置的GPIO引脚（从配置文件读取）
    int infrared_gpio_pin_ = 17;
    int water_immersion_gpio_pin_ = 18;
    int smoke_gpio_pin_ = 27;
    
    // 当前传感器状态缓存
    SensorStatusData infrared_status_;
    SensorStatusData water_status_;
    SensorStatusData smoke_status_;
    SensorStatusData temp_humidity_status_;
};

#endif