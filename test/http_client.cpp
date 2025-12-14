#include  "http_client.h"
#include  <iostream>
// 主函数示例
int main() {
    try {
        // ==================== 示例1: GET请求 ====================
        std::cout << "===== 测试GET请求 =====" << std::endl;
        // 替换为你实际的HTTP服务端地址（比如之前写的8080端口服务）
        std::string getUrl = "http://localhost:8080/api/hello";
        std::string getResp = HttpClient::get(getUrl);
        std::cout << "GET请求URL: " << getUrl << std::endl;
        std::cout << "GET响应结果: " << getResp << "\n\n";

        // ==================== 示例2: POST JSON请求 ====================
        std::cout << "===== 测试POST JSON请求 =====" << std::endl;
        // 替换为你实际的POST接口地址
        std::string postUrl = "http://localhost:8080/api/post";
        // 构造JSON请求体
        nlohmann::json postBody;
        postBody["name"] = "测试客户端";
        postBody["age"] = 20;
        postBody["timestamp"] = (long long)time(NULL);

        std::string postResp = HttpClient::postJson(postUrl, postBody);
        std::cout << "POST请求URL: " << postUrl << std::endl;
        std::cout << "POST请求体: " << postBody.dump(2) << std::endl;
        std::cout << "POST响应结果: " << postResp << std::endl;

    } catch (const std::exception& e) {
        // 异常处理
        std::cerr << "请求失败: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
