#include "config_parser.h"
#include <fstream>
#include <iostream>

using json = nlohmann::json;

ConfigParser& ConfigParser::getInstance()
{
    static ConfigParser instance;
    return instance;
}

bool ConfigParser::loadFromFile(const std::string& path)
{
    // std::cout << "[ConfigParser] Loading config from: " << path << "\n";
    if(isLoaded_) {
        return true; // 已加载，直接返回
    }
    std::ifstream ifs(path);
    if (!ifs.is_open()) {
        std::cerr << "[ConfigParser] Failed to open file: " << path << "\n";
        return false;
    }

    json j;
    try {
        ifs >> j;
        // std::cout << "[ConfigParser] JSON loaded successfully.\n";
    } catch (std::exception& e) {
        std::cerr << "[ConfigParser] JSON parse error: " << e.what() << "\n";
        return false;
    }

    auto& root = j["device_config"];

    config_.version = root.value("version", "");
    config_.description = root.value("description", "");
    config_.boxId = root.value("box", "");

    auto& devs = root["devices"];

    parseCameras(devs);
    parsePLCList(devs);        // ⭐ 新增
    parsePLCDevices(devs);     // 改写后的
    parseSensors(devs);
    parseGPIOSensors(devs);           // GPIO传感器（红外、水浸、烟感）
    parseTempHumiditySensor(devs);    // 温湿度传感器
    parseDoorLocks(devs);             // 门锁
    parseCarControls(devs);
    parseGateways(devs);

    isLoaded_ = true;
    return true;
}

// ---------------- Cameras ----------------
void ConfigParser::parseCameras(const json& j)
{
    if (!j.contains("camera")) return;

    for (auto& item : j["camera"]) {
        CameraConfig c;
        c.id = item.value("id", "");
        c.name = item.value("name", "");
        c.url = item.value("url", "");
        config_.cameras.push_back(c);
    }
}

// ---------------- PLC 本体：plc_list ----------------
void ConfigParser::parsePLCList(const json& j)
{
    if (!j.contains("plc_list")) return;

    for (auto& item : j["plc_list"]) {
        PLCConfig p;

        p.plcId = item.value("plc_id", "");
        p.name = item.value("name", "");
        std::string sid = item.value("slave_id", "1");
        if(sid.rfind("0x", 0) == 0 || sid.rfind("0X", 0) == 0){
            p.slaveId = static_cast<uint8_t>(std::stoi(sid, nullptr, 16));
        }
        else{
            p.slaveId = static_cast<uint8_t>(std::stoi(sid));
        }
        p.connectionType = item.value("connection_type", "");

        // direct
        if (p.connectionType == "direct" && item.contains("serial")) {
            p.hasSerial = true;

            auto& s = item["serial"];
            p.serialConfig.serial.port = s.value("port", "");
            p.serialConfig.serial.baudRate = s.value("baud_rate", 0);
            p.serialConfig.serial.parity = s.value("parity", "");
            p.serialConfig.serial.stopBits = s.value("stop_bits", 1);
        }

        // gateway
        if (p.connectionType == "gateway" && item.contains("gateway")) {
            p.hasGateway = true;

            auto& g = item["gateway"];
            p.gatewayConfig.gatewayId = g.value("gateway_id", "");
            p.gatewayConfig.gatewayIp = g.value("gateway_ip", "");
            p.gatewayConfig.gatewayPort = g.value("gateway_port", 0);
        }

        config_.plcs.push_back(p);
    }
}

// ---------------- PLC 下挂设备：plc_device ----------------
void ConfigParser::parsePLCDevices(const json& j)
{
    if (!j.contains("plc_device")) return;

    for (auto& item : j["plc_device"]) {
        PLCDeviceConfig d;

        d.id = item.value("id", "");
        d.plcId = item.value("plc_id", "");   
        d.name = item.value("name", "");
        d.deviceType = item.value("type", "");
        d.registerAddress = item.value("register", "");  

        config_.plcDevices.push_back(d);
    }
}

// ---------------- Sensors ----------------
void ConfigParser::parseSensors(const json& j)
{
    if (!j.contains("sensor")) return;

    for (auto& item : j["sensor"]) {
        SensorConfig s;
        s.id = item.value("id", "");
        s.name = item.value("name", "");
        s.type = item.value("type", "");

        auto& sc = item["serial_config"];
        s.serial.port = sc.value("port", "");
        s.serial.baudRate = sc.value("baud_rate", 0);
        s.serial.parity = sc.value("parity", "");
        s.serial.stopBits = sc.value("stop_bits", 1);

        if (s.type == "modbus") {
            s.modbusAddr = item.value("modbus_addr", 1);
            s.regStart   = item.value("reg_start", 0);
            s.regCount   = item.value("reg_count", 2);
        }

        config_.sensors.push_back(s);
    }
}

    // ---------------- CarControl ----------------
    void ConfigParser::parseCarControls(const json& j)
    {
        if (!j.contains("carcontrol")) return;

        for (auto& item : j["carcontrol"]) {
            CarControlConfig c;
            c.id = item.value("id", "");
            c.name = item.value("name", "");
            if (item.contains("serial_config")) {
                auto& sc = item["serial_config"];
                c.serial.port = sc.value("port", "");
                c.serial.baudRate = sc.value("baud_rate", 0);
                c.serial.parity = sc.value("parity", "");
                c.serial.stopBits = sc.value("stop_bits", 1);
            }
                // timing params (optional)
                c.sendWindowMs = item.value("send_window_ms", c.sendWindowMs);
                c.sendIntervalMs = item.value("send_interval_ms", c.sendIntervalMs);
                c.operateTimeoutMs = item.value("operate_timeout_ms", c.operateTimeoutMs);
            config_.carControls.push_back(c);
        }
    }

// ---------------- GPIO Sensors (红外、水浸、烟感) ----------------
void ConfigParser::parseGPIOSensors(const json& j)
{
    if (!j.contains("gpio_sensor")) return;

    for (auto& item : j["gpio_sensor"]) {
        GPIOSensorConfig s;
        s.id = item.value("id", "");
        s.name = item.value("name", "");
        s.type = item.value("type", "");
        s.gpioPin = item.value("gpio_pin", 0);
        s.description = item.value("description", "");

        config_.gpioSensors.push_back(s);
    }
}

// ---------------- 温湿度传感器 ----------------
void ConfigParser::parseTempHumiditySensor(const json& j)
{
    if (!j.contains("temp_humidity_sensor")) return;

    auto& ths = j["temp_humidity_sensor"];
    
    // 解析串口配置
    if (ths.contains("serial_config")) {
        auto& sc = ths["serial_config"];
        config_.tempHumiditySensor.serial.port = sc.value("port", "");
        config_.tempHumiditySensor.serial.baudRate = sc.value("baud_rate", 9600);
        config_.tempHumiditySensor.serial.parity = sc.value("parity", "none");
        config_.tempHumiditySensor.serial.stopBits = sc.value("stop_bits", 1);
    }
    
    // 解析传感器列表
    if (ths.contains("sensors")) {
        for (auto& item : ths["sensors"]) {
            TempHumiditySensorConfig s;
            s.id = item.value("id", "");
            s.name = item.value("name", "");
            s.modbusAddr = item.value("modbus_addr", 1);
            s.tempRegister = item.value("temp_register", 0);
            s.humidityRegister = item.value("humidity_register", 1);
            s.regCount = item.value("reg_count", 2);

            config_.tempHumiditySensor.sensors.push_back(s);
        }
    }
}

// ---------------- 门锁配置 ----------------
void ConfigParser::parseDoorLocks(const json& j)
{
    if (!j.contains("door_lock")) return;

    for (auto& item : j["door_lock"]) {
        DoorLockConfig d;
        d.id = item.value("id", "");
        d.name = item.value("name", "");
        d.type = item.value("type", "");
        d.gpioPin = item.value("gpio_pin", 0);
        d.description = item.value("description", "");

        config_.doorLocks.push_back(d);
    }
}

// ---------------- Gateways ----------------
void ConfigParser::parseGateways(const json& j)
{
    if (!j.contains("gateway")) return;

    for (auto& item : j["gateway"]) {
        GatewayConfig g;
        g.id = item.value("id", "");
        g.name = item.value("name", "");
        g.ip = item.value("ip", "");
        g.protocol = item.value("protocol", "");
        g.status = item.value("status", "");

        config_.gateways.push_back(g);
    }
}
