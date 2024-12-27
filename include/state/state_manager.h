#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include "business/alarm.h"

#ifdef __cplusplus
extern "C" {
#endif

// 系统状态定义
typedef enum {
    SYSTEM_STATE_INIT = 0,    // 初始化状态
    SYSTEM_STATE_RUNNING,     // 运行状态
    SYSTEM_STATE_ERROR,       // 错误状态
    SYSTEM_STATE_STOP,        // 停止状态
    SYSTEM_STATE_MAX
} SystemState;

// 设备状态定义
typedef enum {
    DEVICE_STATE_OFFLINE = 0, // 离线状态
    DEVICE_STATE_ONLINE,      // 在线状态
    DEVICE_STATE_ERROR,       // 错误状态
    DEVICE_STATE_MAX
} DeviceState;

// 网络状态定义
typedef enum {
    NETWORK_STATE_DISCONNECTED = 0,  // 断开连接
    NETWORK_STATE_CONNECTING,        // 正在连接
    NETWORK_STATE_CONNECTED,         // 已连接
    NETWORK_STATE_ERROR,            // 连接错误
    NETWORK_STATE_MAX
} NetworkState;

// 错误码定义
typedef enum {
    STATE_ERROR_NONE = 0,     // 无错误
    STATE_ERROR_PARAM,        // 参数错误
    STATE_ERROR_STORAGE,      // 存储错误
    STATE_ERROR_DEVICE,       // 设备错误
    STATE_ERROR_NETWORK,      // 网络错误
    STATE_ERROR_MAX
} StateError;

// 设备类型定义
typedef enum {
    DEVICE_TYPE_DHT11 = 0,   // DHT11温湿度传感器
    DEVICE_TYPE_MQ2,         // MQ2烟雾传感器
    DEVICE_TYPE_BH1750,      // BH1750光照传感器
    DEVICE_TYPE_LED,         // LED指示灯
    DEVICE_TYPE_BUZZER,      // 蜂鸣器
    DEVICE_TYPE_MAX
} DeviceType;

// 状态变化回调函数类型
typedef void (*StateCallback)(void* arg);

// 状态管理器配置
typedef struct {
    bool auto_recovery;          // 是否自动恢复
    uint32_t recovery_interval;  // 恢复检查间隔(ms)
    uint32_t max_retry_count;    // 最大重试次数
} StateManagerConfig;

// 初始化状态管理器
int StateManagerInit(const StateManagerConfig* config);

// 获取系统状态
SystemState StateManagerGetSystemState(void);

// 设置系统状态
int StateManagerSetSystemState(SystemState state);

// 获取设备状态
DeviceState StateManagerGetDeviceState(DeviceType device);

// 设置设备状态
int StateManagerSetDeviceState(DeviceType device, DeviceState state);

// 获取网络状态
NetworkState StateManagerGetNetworkState(void);

// 设置网络状态
int StateManagerSetNetworkState(NetworkState state);

// 获取错误码
StateError StateManagerGetError(void);

// 注���状态变化回调函数
int StateManagerRegisterCallback(DeviceType device, StateCallback callback, void* arg);

// 取消注册状态变化回调函数
int StateManagerUnregisterCallback(DeviceType device, StateCallback callback);

// 检查设备是否在线
bool StateManagerIsDeviceOnline(DeviceType device);

// 检查网络是否连接
bool StateManagerIsNetworkConnected(void);

// 获取设备错误计数
uint32_t StateManagerGetDeviceErrorCount(DeviceType device);

// 重置设备错误计数
int StateManagerResetDeviceErrorCount(DeviceType device);

// 诊断设备状态
int StateManagerDiagnoseDevice(DeviceType device);

// 保存状态
int StateManagerSave(void);

// 加载状态
int StateManagerLoad(void);

// 反初始化状态管理器
int StateManagerDeinit(void);

#ifdef __cplusplus
}
#endif

#endif // STATE_MANAGER_H 