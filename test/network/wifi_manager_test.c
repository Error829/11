#include <stdio.h>
#include <unistd.h>
#include "network/wifi_manager.h"

// Wi-Fi配置参数
#define TEST_WIFI_SSID      "YourWiFiSSID"
#define TEST_WIFI_PASSWORD  "YourWiFiPassword"
#define TEST_WIFI_SECURITY  3  // WPA2-PSK

// Wi-Fi事件回调函数
static void OnWifiEvent(WifiState state, WifiError error)
{
    printf("Wi-Fi Event - State: ");
    switch (state) {
        case WIFI_STATE_IDLE:
            printf("IDLE");
            break;
        case WIFI_STATE_CONNECTING:
            printf("CONNECTING");
            break;
        case WIFI_STATE_CONNECTED:
            printf("CONNECTED");
            break;
        case WIFI_STATE_DISCONNECTED:
            printf("DISCONNECTED");
            break;
        case WIFI_STATE_ERROR:
            printf("ERROR");
            break;
        default:
            printf("UNKNOWN");
            break;
    }
    
    printf(", Error: ");
    switch (error) {
        case WIFI_ERROR_NONE:
            printf("NONE");
            break;
        case WIFI_ERROR_CONFIG:
            printf("CONFIG");
            break;
        case WIFI_ERROR_CONNECT:
            printf("CONNECT");
            break;
        case WIFI_ERROR_AUTH:
            printf("AUTH");
            break;
        case WIFI_ERROR_NETWORK:
            printf("NETWORK");
            break;
        case WIFI_ERROR_HARDWARE:
            printf("HARDWARE");
            break;
        default:
            printf("UNKNOWN");
            break;
    }
    printf("\n");
}

// 测试基本连接功能
void TestBasicConnection(void)
{
    printf("\nTesting basic connection...\n");
    
    // 配置Wi-Fi参数
    WifiConfig config = {0};
    strncpy(config.ssid, TEST_WIFI_SSID, sizeof(config.ssid) - 1);
    strncpy(config.password, TEST_WIFI_PASSWORD, sizeof(config.password) - 1);
    config.security = TEST_WIFI_SECURITY;
    config.channel = 0;  // 自动选择信道
    config.auto_reconnect = 0;
    
    printf("Configuring Wi-Fi...\n");
    if (WifiSetConfig(&config) != 0) {
        printf("Failed to configure Wi-Fi!\n");
        return;
    }
    
    // 连接Wi-Fi
    printf("Connecting to Wi-Fi...\n");
    if (WifiConnect() != 0) {
        printf("Failed to connect to Wi-Fi!\n");
        return;
    }
    
    // 等待连接完成
    sleep(5);
    
    // 获取连接信息
    WifiInfo info = {0};
    if (WifiGetInfo(&info) == 0) {
        printf("Connected to: %s\n", info.ssid);
        printf("Signal strength: %d dBm\n", info.rssi);
        printf("Channel: %d\n", info.channel);
        printf("IP address: %u.%u.%u.%u\n",
               (info.ip_address >> 24) & 0xFF,
               (info.ip_address >> 16) & 0xFF,
               (info.ip_address >> 8) & 0xFF,
               info.ip_address & 0xFF);
    }
    
    // 断开连接
    printf("Disconnecting from Wi-Fi...\n");
    WifiDisconnect();
    sleep(2);
}

// 测试自动重连功能
void TestAutoReconnect(void)
{
    printf("\nTesting auto reconnect...\n");
    
    // 启用自动重连
    printf("Enabling auto reconnect...\n");
    WifiEnableAutoReconnect();
    
    // 连接Wi-Fi
    printf("Connecting to Wi-Fi...\n");
    WifiConnect();
    
    // 等待连接完成
    sleep(5);
    
    // 模拟断开连接
    printf("Simulating connection loss...\n");
    WifiDisconnect();
    
    // 等待自动重连
    printf("Waiting for auto reconnect...\n");
    sleep(15);  // 等待3次重连尝试
    
    // 禁用自动重连
    printf("Disabling auto reconnect...\n");
    WifiDisableAutoReconnect();
}

int main(void)
{
    printf("Wi-Fi Manager Test Program\n");
    
    // 初始化Wi-Fi管理模块
    if (WifiInit() != 0) {
        printf("Failed to initialize Wi-Fi manager!\n");
        return -1;
    }
    printf("Wi-Fi manager initialized successfully.\n");
    
    // 注册事件回调
    WifiRegisterCallback(OnWifiEvent);
    
    // 运行测试
    TestBasicConnection();
    TestAutoReconnect();
    
    // 清理
    printf("\nCleaning up...\n");
    WifiDeinit();
    printf("Test completed.\n");
    
    return 0;
} 