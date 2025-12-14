#include "http_service_tcp.h"
#include "itask.h"
#include "task_result_publisher.h"
#include "job_scheduler.h"
#include "icommand_dispatcher.h"
#include "device_manager.h"
#include "task.h"

#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>


// 主测试函数（核心逻辑）
int main() {
    // ========== 步骤1：初始化核心组件（简化版） ==========
    std::cout << "===== 初始化核心组件 =====" << std::endl;
    
    // 1.1 初始化空壳JobScheduler（无实际逻辑）
    JobScheduler scheduler(4, nullptr, nullptr);
    std::cout << "JobScheduler 初始化完成" << std::endl;

    // 1.2 初始化HTTP控制器（绑定调度器）
    HTTPCommandController controller(scheduler);
    std::cout << "HTTPCommandController 初始化完成" << std::endl;

    // 1.3 初始化并启动WebService（绑定路径/api，关联控制器）
    WebService webService("127.0.0.1", &controller);
    if (!webService.start()) {
        std::cerr << "WebService 启动失败（端口8080可能被占用）" << std::endl;
        return -1;
    }
    std::cout << "WebService 启动成功（监听端口：8080）" << std::endl;

    // ========== 步骤2：测试提示 ==========
    std::cout << "\n===== 测试说明 =====" << std::endl;
    std::cout << "1. 可使用curl命令测试以下接口（示例）：" << std::endl;
    std::cout << "   curl -X POST http://127.0.0.1:8080/api/plc/operate -d '{\"plc_id\":1,\"action\":\"start\"}'" << std::endl;
    std::cout << "   curl -X POST http://127.0.0.1:8080/api/sensor/data -d '{\"sensor_id\":1001}'" << std::endl;
    std::cout << "2. 按Enter键停止服务" << std::endl;

    // ========== 步骤3：等待用户输入后停止服务 ==========
    std::cin.get();

    // ========== 步骤4：资源清理 ==========
    std::cout << "\n===== 停止服务并释放资源 =====" << std::endl;
    webService.stop();
    std::cout << "所有资源已释放，程序正常退出" << std::endl;

    return 0;
}