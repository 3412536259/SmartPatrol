#include "common/logger/logger.h"
#include "common/config.h"
#include "business_layer/sensor_service.h"
#include "business_layer/controller_service.h"

#include <iostream>
#include <csignal>
#include <atomic>
#include <thread>
#include <chrono>

/*
 * 设备测试程序
 * 
 * 功能：逐个测试传感器和控制器设备
 * 测试模式：
 *   1 - 测试红外传感器
 *   2 - 测试门锁控制器
 *   0 - 退出程序
 */

std::atomic<bool> running{true};

void signal_handler(int /*signal*/) {
    Logger::getInstance().log(LogLevel::INFO, "收到退出信号...");
    running = false;
}

// 打印菜单
void printMenu() {
    std::cout << "\n========== 设备测试菜单 ==========" << std::endl;
    std::cout << "  1. 测试红外传感器" << std::endl;
    std::cout << "  2. 测试门锁控制器" << std::endl;
    std::cout << "  0. 退出程序" << std::endl;
    std::cout << "===================================" << std::endl;
    std::cout << "请选择: ";
}

// 测试红外传感器
void testInfraredSensor(SensorService& sensor_service, int gpio_pin) {
    std::cout << "\n--- 红外传感器测试 ---" << std::endl;
    std::cout << "GPIO 引脚: " << gpio_pin << std::endl;
    
    // 添加红外传感器
    sensor_service.addInfraredSensor("infrared_test", gpio_pin);
    
    // 设置数据回调
    sensor_service.setDataCallback([](const std::string& sensor_type, 
                                      const std::string& sensor_id,
                                      bool state, long timestamp) {
        std::cout << "[传感器数据] 类型: " << sensor_type 
                  << ", ID: " << sensor_id
                  << ", 状态: " << (state ? "触发" : "未触发")
                  << ", 时间戳: " << timestamp << std::endl;
    });
    
    // 设置告警回调
    sensor_service.setAlarmCallback([](const std::string& alarm_type, 
                                       const std::string& reason) {
        std::cout << "[告警] 类型: " << alarm_type 
                  << ", 原因: " << reason << std::endl;
    });
    
    // 启动传感器服务
    sensor_service.start();
    
    std::cout << "传感器监控已启动，按 Enter 键停止测试..." << std::endl;
    std::cin.ignore();
    std::cin.get();
    
    // 停止服务
    sensor_service.stop();
    std::cout << "红外传感器测试结束\n" << std::endl;
}

// 测试门锁控制器
void testDoorLock(ControllerService& controller_service, int gpio_pin) {
    std::cout << "\n--- 门锁控制器测试 ---" << std::endl;
    std::cout << "GPIO 引脚: " << gpio_pin << std::endl;
    
    const std::string lock_id = "door_lock_test";
    
    // 添加门锁
    controller_service.addDoorLock(lock_id, gpio_pin);
    
    // 设置状态回调
    controller_service.setStateCallback([](const std::string& device_id,
                                           DeviceType type,
                                           DeviceState state, 
                                           long timestamp) {
        std::cout << "[设备状态] ID: " << device_id 
                  << ", 类型: " << deviceTypeToString(type)
                  << ", 状态: " << deviceStateToString(state)
                  << ", 时间戳: " << timestamp << std::endl;
    });
    
    // 启动控制器服务
    controller_service.start();
    
    bool test_running = true;
    while (test_running && running) {
        std::cout << "\n门锁操作: [1]解锁  [2]上锁  [3]查询状态  [0]返回" << std::endl;
        std::cout << "请选择: ";
        
        std::string input;
        std::getline(std::cin, input);
        
        if (input.empty()) continue;
        
        switch (input[0]) {
            case '1':
                std::cout << "执行解锁..." << std::endl;
                controller_service.turnOn(lock_id);
                break;
            case '2':
                std::cout << "执行上锁..." << std::endl;
                controller_service.turnOff(lock_id);
                break;
            case '3': {
                DeviceState state = controller_service.getDeviceState(lock_id);
                std::cout << "当前状态: " << deviceStateToString(state) << std::endl;
                break;
            }
            case '0':
                test_running = false;
                break;
            default:
                std::cout << "无效选择" << std::endl;
        }
    }
    
    // 停止服务
    controller_service.stop();
    std::cout << "门锁控制器测试结束\n" << std::endl;
}

int main(int argc, char* argv[]) {
    // 设置信号处理
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    // 初始化配置
    auto& config = Config::getInstance();
    if (!config.load()) {
        std::cerr << "加载配置文件失败，使用默认配置" << std::endl;
    }

    // 初始化日志
    Logger::getInstance().log(LogLevel::INFO, "设备测试程序启动...");

    // 从配置读取 GPIO 引脚（或使用默认值）
    int infrared_gpio = config.getInt("sensors.infrared_gpio_pin", 17);
    int doorlock_gpio = config.getInt("controllers.doorlock_gpio_pin", 18);

    std::cout << "\n========== 设备测试程序 ==========" << std::endl;
    std::cout << "红外传感器 GPIO: " << infrared_gpio << std::endl;
    std::cout << "门锁控制器 GPIO: " << doorlock_gpio << std::endl;

    // 创建服务（每次测试单独使用）
    while (running) {
        printMenu();
        
        std::string input;
        std::getline(std::cin, input);
        
        if (input.empty()) continue;
        
        switch (input[0]) {
            case '1': {
                SensorService sensor_service;
                sensor_service.initialize();
                testInfraredSensor(sensor_service, infrared_gpio);
                break;
            }
            case '2': {
                ControllerService controller_service;
                controller_service.initialize();
                testDoorLock(controller_service, doorlock_gpio);
                break;
            }
            case '0':
                running = false;
                std::cout << "退出程序..." << std::endl;
                break;
            default:
                std::cout << "无效选择，请重新输入" << std::endl;
        }
    }

    Logger::getInstance().log(LogLevel::INFO, "设备测试程序退出");
    return 0;
}