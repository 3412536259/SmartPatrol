#include "business_layer/sensor_service.h"
#include "common/logger/logger.h"
#include <chrono>
#include <map>
#include <mutex>
#include <fstream>

// GPIO 传感器信息结构体
struct GPIOSensor {
    std::string sensor_id;
    std::string sensor_type;
    int gpio_pin;
    bool last_state;
    long last_trigger_time;
    
    GPIOSensor(const std::string& id, const std::string& type, int pin)
        : sensor_id(id), sensor_type(type), gpio_pin(pin), 
          last_state(false), last_trigger_time(0) {}
};

// 传感器服务内部实现
class SensorServiceImpl {
public:
    std::vector<GPIOSensor> sensors;
    mutable std::mutex sensors_mutex;
    
    void addSensor(const std::string& sensor_id, const std::string& sensor_type, int gpio_pin) {
        std::lock_guard<std::mutex> lock(sensors_mutex);
        
        // 检查是否已存在
        for (const auto& sensor : sensors) {
            if (sensor.sensor_id == sensor_id) {
                Logger::getInstance().log(LogLevel::WARNING, 
                    std::string("传感器已存在: ") + sensor_id);
                return;
            }
        }
        
        sensors.emplace_back(sensor_id, sensor_type, gpio_pin);
        Logger::getInstance().log(LogLevel::INFO, 
            std::string("已添加传感器: ") + sensor_id + 
            " (类型: " + sensor_type + ", GPIO: " + std::to_string(gpio_pin) + ")");
    }
    
    std::vector<GPIOSensor> getSensors() const {
        std::lock_guard<std::mutex> lock(sensors_mutex);
        return sensors;
    }
    
    GPIOSensor* findSensor(const std::string& sensor_id) {
        std::lock_guard<std::mutex> lock(sensors_mutex);
        for (auto& sensor : sensors) {
            if (sensor.sensor_id == sensor_id) {
                return &sensor;
            }
        }
        return nullptr;
    }
    
    void updateSensorState(const std::string& sensor_id, bool state, long timestamp) {
        std::lock_guard<std::mutex> lock(sensors_mutex);
        for (auto& sensor : sensors) {
            if (sensor.sensor_id == sensor_id) {
                sensor.last_state = state;
                sensor.last_trigger_time = timestamp;
                break;
            }
        }
    }
};

// 静态辅助函数：读取 GPIO 引脚状态
static bool readGPIOPin(int gpio_pin) {
    std::string gpio_path = "/sys/class/gpio/gpio" + std::to_string(gpio_pin) + "/value";
    std::ifstream gpio_file(gpio_path);
    
    if (gpio_file.is_open()) {
        int value = 0;
        gpio_file >> value;
        gpio_file.close();
        return (value == 1);
    }
    
    // 模拟环境返回 false
    return false;
}

// 静态辅助函数：初始化 GPIO 引脚
static bool initGPIOPin(int gpio_pin) {
    // Export GPIO
    std::ofstream export_file("/sys/class/gpio/export");
    if (export_file.is_open()) {
        export_file << gpio_pin;
        export_file.close();
    }
    
    // 设置为输入模式
    std::string direction_path = "/sys/class/gpio/gpio" + std::to_string(gpio_pin) + "/direction";
    std::ofstream direction_file(direction_path);
    if (direction_file.is_open()) {
        direction_file << "in";
        direction_file.close();
        return true;
    }
    
    return true; // 模拟环境也返回成功
}

// 静态辅助函数：获取当前时间戳
static long getCurrentTimestamp() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}

// 静态实现指针
static std::unique_ptr<SensorServiceImpl> s_impl;

// ===================== SensorService 实现 =====================

SensorService::SensorService() 
    : running_{false} {
    s_impl = std::make_unique<SensorServiceImpl>();
    Logger::getInstance().log(LogLevel::INFO, "传感器服务已创建");
}

SensorService::~SensorService() {
    stop();
    s_impl.reset();
    Logger::getInstance().log(LogLevel::INFO, "传感器服务已销毁");
}

bool SensorService::initialize() {
    Logger::getInstance().log(LogLevel::INFO, "正在初始化传感器服务...");
    
    try {
        // 初始化所有已添加的传感器的 GPIO 引脚
        auto sensors = s_impl->getSensors();
        for (const auto& sensor : sensors) {
            if (!initGPIOPin(sensor.gpio_pin)) {
                Logger::getInstance().log(LogLevel::WARNING, 
                    std::string("GPIO 初始化失败: ") + std::to_string(sensor.gpio_pin));
            }
        }
        
        Logger::getInstance().log(LogLevel::INFO, "传感器服务初始化成功");
        return true;
    } catch (const std::exception& e) {
        Logger::getInstance().log(LogLevel::ERROR, 
            std::string("传感器服务初始化失败: ") + e.what());
        return false;
    }
}

void SensorService::start() {
    if (running_) {
        Logger::getInstance().log(LogLevel::WARNING, "传感器服务已在运行中");
        return;
    }
    
    running_ = true;
    monitoring_thread_ = std::thread(&SensorService::monitoringLoop, this);
    Logger::getInstance().log(LogLevel::INFO, "传感器服务已启动");
}

void SensorService::stop() {
    if (!running_) {
        return;
    }
    
    running_ = false;
    
    if (monitoring_thread_.joinable()) {
        monitoring_thread_.join();
    }
    
    Logger::getInstance().log(LogLevel::INFO, "传感器服务已停止");
}

bool SensorService::isRunning() const {
    return running_;
}

void SensorService::addInfraredSensor(const std::string& sensor_id, int gpio_pin) {
    Logger::getInstance().log(LogLevel::INFO, 
        std::string("添加红外传感器: ") + sensor_id + 
        " (GPIO引脚: " + std::to_string(gpio_pin) + ")");
    
    s_impl->addSensor(sensor_id, "infrared", gpio_pin);
    initGPIOPin(gpio_pin);
}

void SensorService::addWaterImmersionSensor(const std::string& sensor_id, int gpio_pin) {
    Logger::getInstance().log(LogLevel::INFO, 
        std::string("添加水浸传感器: ") + sensor_id + 
        " (GPIO引脚: " + std::to_string(gpio_pin) + ")");
    
    s_impl->addSensor(sensor_id, "water_immersion", gpio_pin);
    initGPIOPin(gpio_pin);
}

void SensorService::addSmokeSensor(const std::string& sensor_id, int gpio_pin) {
    Logger::getInstance().log(LogLevel::INFO, 
        std::string("添加烟雾传感器: ") + sensor_id + 
        " (GPIO引脚: " + std::to_string(gpio_pin) + ")");
    
    s_impl->addSensor(sensor_id, "smoke", gpio_pin);
    initGPIOPin(gpio_pin);
}

void SensorService::setAlarmCallback(
    std::function<void(const std::string& alarm_type, const std::string& reason)> callback) {
    alarm_callback_ = callback;
    Logger::getInstance().log(LogLevel::INFO, "已设置告警回调函数");
}

void SensorService::setDataCallback(
    std::function<void(const std::string& sensor_type, const std::string& sensor_id,
                      bool state, long timestamp)> callback) {
    data_callback_ = callback;
    Logger::getInstance().log(LogLevel::INFO, "已设置数据回调函数");
}

void SensorService::monitoringLoop() {
    Logger::getInstance().log(LogLevel::INFO, "传感器监控循环已启动");
    
    const int POLL_INTERVAL_MS = 50;       // 50ms 轮询间隔
    const long DEBOUNCE_TIME_MS = 100;     // 100ms 去抖时间
    
    while (running_) {
        try {
            auto sensors = s_impl->getSensors();
            long current_time = getCurrentTimestamp();
            
            for (auto& sensor : sensors) {
                // 读取 GPIO 状态
                bool current_state = readGPIOPin(sensor.gpio_pin);
                
                // 检测状态变化（带去抖）
                if (current_state != sensor.last_state) {
                    // 检查是否超过去抖时间
                    if (current_time - sensor.last_trigger_time >= DEBOUNCE_TIME_MS) {
                        // 更新状态
                        s_impl->updateSensorState(sensor.sensor_id, current_state, current_time);
                        
                        // 处理传感器事件
                        handleGPIOSensor(sensor.sensor_type, sensor.sensor_id, 
                                        sensor.gpio_pin, current_state);
                    }
                }
            }
            
            // 轮询间隔
            std::this_thread::sleep_for(std::chrono::milliseconds(POLL_INTERVAL_MS));
            
        } catch (const std::exception& e) {
            Logger::getInstance().log(LogLevel::ERROR, 
                std::string("传感器监控循环异常: ") + e.what());
        }
    }
    
    Logger::getInstance().log(LogLevel::INFO, "传感器监控循环已停止");
}

void SensorService::handleGPIOSensor(const std::string& sensor_type, 
                                    const std::string& sensor_id,
                                    int gpio_pin, bool triggered) {
    Logger::getInstance().log(LogLevel::INFO, 
        std::string("GPIO传感器事件: ") + sensor_type + 
        " (ID: " + sensor_id + ", 引脚: " + std::to_string(gpio_pin) + 
        ", 状态: " + (triggered ? "触发" : "恢复") + ")");
    
    long timestamp = getCurrentTimestamp();
    
    // 调用数据回调
    if (data_callback_) {
        try {
            data_callback_(sensor_type, sensor_id, triggered, timestamp);
        } catch (const std::exception& e) {
            Logger::getInstance().log(LogLevel::ERROR, 
                std::string("数据回调异常: ") + e.what());
        }
    }
    
    // 触发时生成告警
    if (triggered && alarm_callback_) {
        std::string alarm_type;
        std::string reason;
        
        if (sensor_type == "infrared") {
            alarm_type = "入侵检测";
            reason = "红外传感器检测到运动: " + sensor_id;
        } else if (sensor_type == "water_immersion") {
            alarm_type = "水浸告警";
            reason = "水浸传感器检测到漏水: " + sensor_id;
        } else if (sensor_type == "smoke") {
            alarm_type = "烟雾告警";
            reason = "烟雾传感器检测到烟雾: " + sensor_id;
        }
        
        if (!alarm_type.empty()) {
            try {
                alarm_callback_(alarm_type, reason);
            } catch (const std::exception& e) {
                Logger::getInstance().log(LogLevel::ERROR, 
                    std::string("告警回调异常: ") + e.what());
            }
        }
    }
}
