#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

// 系统头文件
#include "los_typedef.h"
#include "los_task.h"
#include "los_sem.h"
#include "cmsis_os2.h"

// Wi-Fi相关头文件
#include "wifi_device.h"
#include "wifi_error_code.h"
#include "lwip/ip_addr.h"
#include "lwip/netifapi.h"

// 应用头文件
#include "network/wifi_manager.h"

// 重连参数定义
#define WIFI_RECONNECT_INTERVAL    5000   // 重连间隔(ms)
#define WIFI_RECONNECT_MAX_TIMES   5      // 最大重连次数

// 全局变量
static WifiState g_wifi_state = WIFI_STATE_IDLE;
static WifiError g_wifi_error = WIFI_ERROR_NONE;
static WifiConfig g_wifi_config = {0};
static WifiEventCallback g_wifi_callback = NULL;
static uint8_t g_auto_reconnect = 0;
static uint8_t g_reconnect_times = 0;
static osTimerId_t g_reconnect_timer = NULL;

// 函数前向声明
static void UpdateState(WifiState state, WifiError error);
static void ReconnectTimerCallback(void *arg);
static int WaitScanResult(void);
static int DisconnectWithAp(void);

// 等待扫描结果
static int WaitScanResult(void)
{
    return 0; // 暂时返回成功
}

// 断开AP连接
static int DisconnectWithAp(void)
{
    return Disconnect(); // 返回SDK的Disconnect函数结果
}

// 状态更新函数
static void UpdateState(WifiState state, WifiError error)
{
    g_wifi_state = state;
    g_wifi_error = error;
    
    if (g_wifi_callback != NULL) {
        g_wifi_callback(state, error);
    }
}

// 重连定时器回调
static void ReconnectTimerCallback(void *arg)
{
    (void)arg;
    if (g_auto_reconnect && g_wifi_state == WIFI_STATE_DISCONNECTED) {
        if (g_reconnect_times < WIFI_RECONNECT_MAX_TIMES) {
            g_reconnect_times++;
            WifiConnect();
        } else {
            WifiDisableAutoReconnect();
            UpdateState(WIFI_STATE_ERROR, WIFI_ERROR_CONNECT);
        }
    }
}

// 初始化Wi-Fi管理模块
int WifiInit(void)
{
    // 初始化Wi-Fi设备
    if (EnableWifi() != WIFI_SUCCESS) {
        UpdateState(WIFI_STATE_ERROR, WIFI_ERROR_HARDWARE);
        return -1;
    }
    
    // 创建重连定时器
    osTimerAttr_t timer_attr = {
        .name = "WifiReconnectTimer",
        .attr_bits = 0,
        .cb_mem = NULL,
        .cb_size = 0
    };
    g_reconnect_timer = osTimerNew(ReconnectTimerCallback, osTimerPeriodic, NULL, &timer_attr);
    if (g_reconnect_timer == NULL) {
        DisableWifi();
        UpdateState(WIFI_STATE_ERROR, WIFI_ERROR_HARDWARE);
        return -1;
    }
    
    UpdateState(WIFI_STATE_IDLE, WIFI_ERROR_NONE);
    return 0;
}

// 配置Wi-Fi参数
int WifiSetConfig(const WifiConfig* config)
{
    if (config == NULL) {
        UpdateState(WIFI_STATE_ERROR, WIFI_ERROR_CONFIG);
        return -1;
    }
    
    // 保存配置
    memcpy(&g_wifi_config, config, sizeof(WifiConfig));
    
    // 配置Wi-Fi设备
    WifiDeviceConfig wifi_config = {0};
    strncpy(wifi_config.ssid, config->ssid, sizeof(wifi_config.ssid) - 1);
    strncpy(wifi_config.preSharedKey, config->password, sizeof(wifi_config.preSharedKey) - 1);
    wifi_config.securityType = config->security;
    
    int netId = AddDeviceConfig(&wifi_config, 0);
    if (netId < 0) {
        UpdateState(WIFI_STATE_ERROR, WIFI_ERROR_CONFIG);
        return -1;
    }
    
    return 0;
}

// 获取当前Wi-Fi配置
int WifiGetConfig(WifiConfig* config)
{
    if (config == NULL) {
        return -1;
    }
    
    memcpy(config, &g_wifi_config, sizeof(WifiConfig));
    return 0;
}

// 连接Wi-Fi
int WifiConnect(void)
{
    if (g_wifi_state == WIFI_STATE_CONNECTED) {
        return 0;
    }
    
    UpdateState(WIFI_STATE_CONNECTING, WIFI_ERROR_NONE);
    
    // 连接Wi-Fi
    int ret = ConnectTo(0);
    if (ret != WIFI_SUCCESS) {
        if (g_auto_reconnect) {
            osTimerStart(g_reconnect_timer, WIFI_RECONNECT_INTERVAL);
        }
        UpdateState(WIFI_STATE_DISCONNECTED, WIFI_ERROR_CONNECT);
        return -1;
    }
    
    // 等待获取IP地址
    ret = WaitScanResult();
    if (ret != WIFI_SUCCESS) {
        DisconnectWithAp();
        if (g_auto_reconnect) {
            osTimerStart(g_reconnect_timer, WIFI_RECONNECT_INTERVAL);
        }
        UpdateState(WIFI_STATE_DISCONNECTED, WIFI_ERROR_NETWORK);
        return -1;
    }
    
    g_reconnect_times = 0;
    UpdateState(WIFI_STATE_CONNECTED, WIFI_ERROR_NONE);
    return 0;
}

// 断开Wi-Fi连接
int WifiDisconnect(void)
{
    if (g_wifi_state == WIFI_STATE_DISCONNECTED) {
        return 0;
    }
    
    // 停止重连定时器
    osTimerStop(g_reconnect_timer);
    
    // 断开连接
    int ret = DisconnectWithAp();
    if (ret != WIFI_SUCCESS) {
        UpdateState(WIFI_STATE_ERROR, WIFI_ERROR_HARDWARE);
        return -1;
    }
    
    UpdateState(WIFI_STATE_DISCONNECTED, WIFI_ERROR_NONE);
    return 0;
}

// 获取Wi-Fi状态
WifiState WifiGetState(void)
{
    return g_wifi_state;
}

// 获取错误码
WifiError WifiGetError(void)
{
    return g_wifi_error;
}

// 获取连接信息
int WifiGetInfo(WifiInfo* info)
{
    if (info == NULL) {
        return -1;
    }

    WifiLinkedInfo linked_info = {0};
    if (GetLinkedInfo(&linked_info) != WIFI_SUCCESS) {
        return -1;
    }

    // 复制连接信息
    strncpy(info->ssid, linked_info.ssid, sizeof(info->ssid) - 1);
    memcpy(info->bssid, linked_info.bssid, sizeof(info->bssid));
    info->rssi = linked_info.rssi;
    info->channel = 0; // 暂时固定为0,因为SDK不提供此信息
    info->ip_address = linked_info.ipAddress;

    return 0;
}

// 注册事件回调函数
int WifiRegisterCallback(WifiEventCallback callback)
{
    if (callback == NULL) {
        return -1;
    }
    
    g_wifi_callback = callback;
    return 0;
}

// 取消注册事件回调函数
int WifiUnregisterCallback(void)
{
    g_wifi_callback = NULL;
    return 0;
}

// 启动自动重连
int WifiEnableAutoReconnect(void)
{
    g_auto_reconnect = 1;
    g_reconnect_times = 0;
    
    if (g_wifi_state == WIFI_STATE_DISCONNECTED) {
        osTimerStart(g_reconnect_timer, WIFI_RECONNECT_INTERVAL);
    }
    
    return 0;
}

// 停止自动重连
int WifiDisableAutoReconnect(void)
{
    g_auto_reconnect = 0;
    osTimerStop(g_reconnect_timer);
    return 0;
}

// 反初始化Wi-Fi管理模块
int WifiDeinit(void)
{
    // 断开连接
    WifiDisconnect();
    
    // 删除重连定时器
    if (g_reconnect_timer != NULL) {
        osTimerDelete(g_reconnect_timer);
        g_reconnect_timer = NULL;
    }
    
    // 关闭Wi-Fi
    if (DisableWifi() != WIFI_SUCCESS) {
        return -1;
    }
    
    UpdateState(WIFI_STATE_IDLE, WIFI_ERROR_NONE);
    return 0;
}
