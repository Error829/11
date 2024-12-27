#ifndef NETWORK_WIFI_MANAGER_H
#define NETWORK_WIFI_MANAGER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Wi-Fi连接状态
typedef enum {
    WIFI_STATE_IDLE = 0,        // 空闲状态
    WIFI_STATE_CONNECTING,      // 正在连接
    WIFI_STATE_CONNECTED,       // 已连接
    WIFI_STATE_DISCONNECTED,    // 已断开
    WIFI_STATE_ERROR           // 错误状态
} WifiState;

// Wi-Fi错误码
typedef enum {
    WIFI_ERROR_NONE = 0,           // 无错误
    WIFI_ERROR_CONFIG = -1,        // 配置错误
    WIFI_ERROR_CONNECT = -2,       // 连接错误
    WIFI_ERROR_AUTH = -3,          // 认证错误
    WIFI_ERROR_NETWORK = -4,       // 网络错误
    WIFI_ERROR_HARDWARE = -5       // 硬件错误
} WifiError;

// Wi-Fi配置结构体
typedef struct {
    char ssid[33];             // SSID
    char password[65];         // 密码
    uint8_t security;          // 安全类型
    uint8_t channel;           // 信道
    uint8_t auto_reconnect;    // 自动重连
} WifiConfig;

// Wi-Fi连接信息
typedef struct {
    char ssid[33];            // SSID
    uint8_t bssid[6];         // BSSID
    int8_t rssi;              // 信号强度
    uint8_t channel;          // 信道
    uint32_t ip_address;      // IP地址
} WifiInfo;

// Wi-Fi事件回调函数类型
typedef void (*WifiEventCallback)(WifiState state, WifiError error);

// 初始化Wi-Fi管理模块
int WifiInit(void);

// 配置Wi-Fi参数
int WifiSetConfig(const WifiConfig* config);

// 获取当前Wi-Fi配置
int WifiGetConfig(WifiConfig* config);

// 连接Wi-Fi
int WifiConnect(void);

// 断开Wi-Fi连接
int WifiDisconnect(void);

// 获取Wi-Fi状态
WifiState WifiGetState(void);

// 获取错误码
WifiError WifiGetError(void);

// 获取连接信息
int WifiGetInfo(WifiInfo* info);

// 注册事件回调函数
int WifiRegisterCallback(WifiEventCallback callback);

// 取消注册事件回调函数
int WifiUnregisterCallback(void);

// 启动自动重连
int WifiEnableAutoReconnect(void);

// 停止自动重连
int WifiDisableAutoReconnect(void);

// 反初始化Wi-Fi管理模块
int WifiDeinit(void);

#ifdef __cplusplus
}
#endif

#endif // NETWORK_WIFI_MANAGER_H
