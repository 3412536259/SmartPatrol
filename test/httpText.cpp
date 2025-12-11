// #include "http_service_tcp.h"
// #include "itask.h"
// #include "itask_result_publisher.h"
// #include "job_scheduler.h"
// #include "icommand_dispatcher.h"
// #include "device_manager.h"
// #include "task.h"

// #include <iostream>
// #include <thread>
// #include <chrono>
// #include <cstring>
// #include <sys/socket.h>
// #include <arpa/inet.h>
// #include <unistd.h>

// // 工具函数：发送HTTP JSON请求（复用无需修改）
// std::string sendHttpJsonRequest(const std::string& ip, uint16_t port, const std::string& jsonRequest) {
//     int sock = socket(AF_INET, SOCK_STREAM, 0);
//     if (sock < 0) {
//         std::cerr << "创建客户端套接字失败: " << strerror(errno) << std::endl;
//         return "";
//     }

//     sockaddr_in server_addr{};
//     server_addr.sin_family = AF_INET;
//     server_addr.sin_port = htons(port);
//     if (inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr) <= 0) {
//         std::cerr << "无效的IP地址: " << ip << std::endl;
//         close(sock);
//         return "";
//     }

//     // 连接服务器
//     if (connect(sock, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
//         std::cerr << "连接服务器失败: " << strerror(errno) << std::endl;
//         close(sock);
//         return "";
//     }

//     // 构建HTTP POST请求
//     std::string httpRequest = 
//         "POST /api HTTP/1.1\r\n"
//         "Host: " + ip + ":" + std::to_string(port) + "\r\n"
//         "Content-Type: application/json\r\n"
//         "Content-Length: " + std::to_string(jsonRequest.size()) + "\r\n"
//         "Connection: close\r\n"
//         "\r\n" + jsonRequest;

//     // 发送请求
//     ssize_t send_len = send(sock, httpRequest.c_str(), httpRequest.size(), 0);
//     if (send_len < 0) {
//         std::cerr << "发送请求失败: " << strerror(errno) << std::endl;
//         close(sock);
//         return "";
//     }

//     // 接收响应
//     char buffer[4096] = {0};
//     ssize_t recv_len = recv(sock, buffer, sizeof(buffer) - 1, 0);
//     std::string response;
//     if (recv_len > 0) {
//         response = buffer;
//     }

//     close(sock);
//     return response;
// }

// // 主测试函数（核心逻辑）
// int main() {
//     // ========== 步骤1：初始化核心依赖实例 ==========
//     std::cout << "===== 初始化核心组件 =====" << std::endl;
//     // 1.1 初始化设备管理器（你已实现的IDeviceManager子类）
//     IDeviceManager* devMgr = new DeviceManager(); // 替换为你实际的DeviceManager类
//     if (!devMgr->init()) {
//         std::cerr << "设备管理器初始化失败" << std::endl;
//         return -1;
//     }

//     // 1.2 初始化任务结果发布器（你已实现的ITaskResultPublisher子类）
//     ITaskResultPublisher* publisher = new TaskResultPublisher(); // 替换为实际类

//     // 1.3 初始化任务调度器（4个工作线程）
//     JobScheduler scheduler(4, devMgr, publisher);
//     std::cout << "JobScheduler 初始化完成" << std::endl;

//     // ========== 步骤2：初始化Web服务相关实例 ==========
//     // 2.1 初始化HTTP控制器（绑定任务调度器）
//     HTTPCommandController controller(scheduler);
//     std::cout << "HTTPCommandController 初始化完成" << std::endl;

//     // 2.2 初始化并启动WebService（绑定路径/api，关联控制器）
//     WebService webService("/api", &controller);
//     if (!webService.start()) {
//         std::cerr << "WebService 启动失败（端口8080可能被占用）" << std::endl;
//         delete devMgr;
//         delete publisher;
//         return -1;
//     }
//     std::cout << "WebService 启动成功（监听端口：8080）" << std::endl;

//     // ========== 步骤3：测试1 - 直接调用JobScheduler提交任务 ==========
//     std::cout << "\n===== 测试1：直接提交任务到JobScheduler =====" << std::endl;
//     // 3.1 创建测试任务（你已实现的ITask子类）
//     std::shared_ptr<ITask> task1 = std::make_shared<RealImageTask>(); // 实际任务类1
//     std::shared_ptr<ITask> task2 = std::make_shared<PlcOperateTask>(); // 实际任务类2

//     // 3.2 提交任务并获取任务ID
//     int taskId1 = scheduler.submit(task1);
//     int taskId2 = scheduler.submit(task2);
//     std::cout << "提交任务1，ID：" << taskId1 << std::endl;
//     std::cout << "提交任务2，ID：" << taskId2 << std::endl;

//     // 3.3 等待任务执行并查询状态
//     std::this_thread::sleep_for(std::chrono::seconds(1));
//     std::cout << "任务1状态：" << static_cast<int>(scheduler.getTaskStatus(taskId1)) << std::endl;
//     std::cout << "任务2状态：" << static_cast<int>(scheduler.getTaskStatus(taskId2)) << std::endl;

//     // ========== 步骤4：测试2 - 发送HTTP JSON请求测试WebService ==========
//     std::cout << "\n===== 测试2：发送HTTP JSON请求到WebService =====" << std::endl;
//     std::this_thread::sleep_for(std::chrono::seconds(1)); // 等待服务完全就绪

//     // 4.1 测试：获取传感器数据
//     std::string sensorReq = R"({"topic":"GetSensorData", "payload":{"sensor_id":1001}})";
//     std::string sensorResp = sendHttpJsonRequest("127.0.0.1", 8080, sensorReq);
//     std::cout << "【获取传感器数据】请求：" << sensorReq << std::endl;
//     std::cout << "【获取传感器数据】响应：" << sensorResp << "\n" << std::endl;

//     // 4.2 测试：操作PLC设备
//     std::string plcReq = R"({"topic":"OperatePlc", "payload":{"plc_id":2001, "cmd":"start_production"}})";
//     std::string plcResp = sendHttpJsonRequest("127.0.0.1", 8080, plcReq);
//     std::cout << "【操作PLC】请求：" << plcReq << std::endl;
//     std::cout << "【操作PLC】响应：" << plcResp << "\n" << std::endl;

//     // 4.3 测试：获取所有设备状态
//     std::string statusReq = R"({"topic":"GetAllDeviceStatus", "payload":{}})";
//     std::string statusResp = sendHttpJsonRequest("127.0.0.1", 8080, statusReq);
//     std::cout << "【获取所有设备状态】请求：" << statusReq << std::endl;
//     std::cout << "【获取所有设备状态】响应：" << statusResp << "\n" << std::endl;

//     // ========== 步骤5：等待用户输入后停止服务 ==========
//     std::cout << "\n===== 测试完成，按Enter键停止服务 =====" << std::endl;
//     std::cin.get();

//     // ========== 步骤6：资源清理 ==========
//     std::cout << "===== 停止服务并释放资源 =====" << std::endl;
//     // 6.1 停止Web服务
//     webService.stop();
//     // 6.2 释放动态分配的资源（JobScheduler析构会自动清理内部资源）
//     delete devMgr;
//     delete publisher;

//     std::cout << "所有资源已释放，程序正常退出" << std::endl;
//     return 0;
// }