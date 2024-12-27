#include <stdio.h>
#include <string.h>
#include "state/state_manager.h"
#include "utils/kv_store/kv_store.h"
#include "cmsis_os2.h"

// 状态存储的键名
#define STATE_KEY "spacestation_state"

// 回调函数最大数量
#define MAX_CALLBACKS 8

// 设备状态结构体
typedef struct {
    DeviceState state;           // 当前状态
    uint32_t error_count;        // 错误计数
    uint32_t last_error_time;    // 最后一次错误时间
    uint32_t retry_count;        // 重试计数
    StateCallback callbacks[MAX_CALLBACKS];  // 回调函数数组
    void* callback_args[MAX_CALLBACKS];      // 回调函数参数
    uint32_t callback_count;     // 回调函数数量
} DeviceStatus;

// 状态管理器结构体
typedef struct {
    SystemState system_state;    // 系统状态
    NetworkState network_state;  // 网络状态
    DeviceStatus devices[DEVICE_TYPE_MAX];  // 设备状态数组
    StateError error;           // 错误码
    StateManagerConfig config;  // 配置
    osTimerId_t recovery_timer; // 恢复定时器
} StateManager;

// 全局状态管理器实例
static StateManager g_state_manager;

// 定时器回调函数
static void RecoveryTimerCallback(void* arg)
{
    (void)arg;
    
    // 检查所有设备状态
    for (DeviceType device = 0; device < DEVICE_TYPE_MAX; device++) {
        DeviceStatus* status = &g_state_manager.devices[device];
        
        // 如果设备处于错误状态且未超过最大重试次数
        if (status->state == DEVICE_STATE_ERROR && 
            status->retry_count < g_state_manager.config.max_retry_count) {
            // 尝试诊断和恢复设备
            if (StateManagerDiagnoseDevice(device) == 0) {
                // 恢复成功，重置错误计数
                status->error_count = 0;
                status->retry_count = 0;
                status->state = DEVICE_STATE_ONLINE;
                
                // 通知状态变化
                for (uint32_t i = 0; i < status->callback_count; i++) {
                    if (status->callbacks[i] != NULL) {
                        status->callbacks[i](status->callback_args[i]);
                    }
                }
            } else {
                // 恢复失败，增加重试计数
                status->retry_count++;
            }
        }
    }
}

// 初始化状态管理器
int StateManagerInit(const StateManagerConfig* config)
{
    if (config == NULL) {
        return -1;
    }
    
    // 初始化状态
    memset(&g_state_manager, 0, sizeof(StateManager));
    memcpy(&g_state_manager.config, config, sizeof(StateManagerConfig));
    
    // 设置初始状态
    g_state_manager.system_state = SYSTEM_STATE_INIT;
    g_state_manager.network_state = NETWORK_STATE_DISCONNECTED;
    
    for (DeviceType device = 0; device < DEVICE_TYPE_MAX; device++) {
        g_state_manager.devices[device].state = DEVICE_STATE_OFFLINE;
    }
    
    // 如果启用自动恢复，创建恢复定时器
    if (config->auto_recovery) {
        osTimerAttr_t timer_attr = {
            .name = "RecoveryTimer",
            .attr_bits = 0,
            .cb_mem = NULL,
            .cb_size = 0
        };
        g_state_manager.recovery_timer = osTimerNew(RecoveryTimerCallback, osTimerPeriodic, NULL, &timer_attr);
        if (g_state_manager.recovery_timer == NULL) {
            return -1;
        }
        
        // 启动定时器
        osTimerStart(g_state_manager.recovery_timer, config->recovery_interval);
    }
    
    // 加载保存的状态
    StateManagerLoad();
    
    return 0;
}

// 获取系统状态
SystemState StateManagerGetSystemState(void)
{
    return g_state_manager.system_state;
}

// 设置系统状态
int StateManagerSetSystemState(SystemState state)
{
    if (state >= SYSTEM_STATE_MAX) {
        g_state_manager.error = STATE_ERROR_PARAM;
        return -1;
    }
    
    g_state_manager.system_state = state;
    return StateManagerSave();
}

// 获取设备状态
DeviceState StateManagerGetDeviceState(DeviceType device)
{
    if (device >= DEVICE_TYPE_MAX) {
        return DEVICE_STATE_ERROR;
    }
    
    return g_state_manager.devices[device].state;
}

// 设置设备状态
int StateManagerSetDeviceState(DeviceType device, DeviceState state)
{
    if (device >= DEVICE_TYPE_MAX || state >= DEVICE_STATE_MAX) {
        g_state_manager.error = STATE_ERROR_PARAM;
        return -1;
    }
    
    DeviceStatus* status = &g_state_manager.devices[device];
    
    // 如果状态发生变化
    if (status->state != state) {
        status->state = state;
        
        // 如果进入错误状态，记录错误信息
        if (state == DEVICE_STATE_ERROR) {
            status->error_count++;
            status->last_error_time = osKernelGetTickCount();
        }
        
        // 通知所有回调函数
        for (uint32_t i = 0; i < status->callback_count; i++) {
            if (status->callbacks[i] != NULL) {
                status->callbacks[i](status->callback_args[i]);
            }
        }
        
        // 保存状态
        StateManagerSave();
    }
    
    return 0;
}

// 获取网络状态
NetworkState StateManagerGetNetworkState(void)
{
    return g_state_manager.network_state;
}

// 设置网络状态
int StateManagerSetNetworkState(NetworkState state)
{
    if (state >= NETWORK_STATE_MAX) {
        g_state_manager.error = STATE_ERROR_PARAM;
        return -1;
    }
    
    g_state_manager.network_state = state;
    return StateManagerSave();
}

// 获取错误码
StateError StateManagerGetError(void)
{
    return g_state_manager.error;
}

// 注册状态变化回调函数
int StateManagerRegisterCallback(DeviceType device, StateCallback callback, void* arg)
{
    if (device >= DEVICE_TYPE_MAX || callback == NULL) {
        g_state_manager.error = STATE_ERROR_PARAM;
        return -1;
    }
    
    DeviceStatus* status = &g_state_manager.devices[device];
    
    // 检查是否已达到最大回调数量
    if (status->callback_count >= MAX_CALLBACKS) {
        g_state_manager.error = STATE_ERROR_PARAM;
        return -1;
    }
    
    // 添加回调函数
    status->callbacks[status->callback_count] = callback;
    status->callback_args[status->callback_count] = arg;
    status->callback_count++;
    
    return 0;
}

// 取消注册状态变化回调函数
int StateManagerUnregisterCallback(DeviceType device, StateCallback callback)
{
    if (device >= DEVICE_TYPE_MAX || callback == NULL) {
        g_state_manager.error = STATE_ERROR_PARAM;
        return -1;
    }
    
    DeviceStatus* status = &g_state_manager.devices[device];
    
    // 查找并移除回调函数
    for (uint32_t i = 0; i < status->callback_count; i++) {
        if (status->callbacks[i] == callback) {
            // 移动后面的回调函数
            for (uint32_t j = i; j < status->callback_count - 1; j++) {
                status->callbacks[j] = status->callbacks[j + 1];
                status->callback_args[j] = status->callback_args[j + 1];
            }
            status->callback_count--;
            return 0;
        }
    }
    
    return -1;
}

// 检查设备是否在线
bool StateManagerIsDeviceOnline(DeviceType device)
{
    if (device >= DEVICE_TYPE_MAX) {
        return false;
    }
    
    return g_state_manager.devices[device].state == DEVICE_STATE_ONLINE;
}

// 检查网络是否连接
bool StateManagerIsNetworkConnected(void)
{
    return g_state_manager.network_state == NETWORK_STATE_CONNECTED;
}

// 获取设备错误计数
uint32_t StateManagerGetDeviceErrorCount(DeviceType device)
{
    if (device >= DEVICE_TYPE_MAX) {
        return 0;
    }
    
    return g_state_manager.devices[device].error_count;
}

// 重置设备错误计数
int StateManagerResetDeviceErrorCount(DeviceType device)
{
    if (device >= DEVICE_TYPE_MAX) {
        g_state_manager.error = STATE_ERROR_PARAM;
        return -1;
    }
    
    g_state_manager.devices[device].error_count = 0;
    g_state_manager.devices[device].retry_count = 0;
    return 0;
}

// 诊断设备状态
int StateManagerDiagnoseDevice(DeviceType device)
{
    if (device >= DEVICE_TYPE_MAX) {
        g_state_manager.error = STATE_ERROR_PARAM;
        return -1;
    }
    
    // 根据设备类型执行不同的诊断
    switch (device) {
        case DEVICE_TYPE_DHT11:
            // 尝试读取DHT11数据
            {
                float temperature, humidity;
                if (DHT11GetData(&temperature, &humidity) == 0) {
                    return 0;
                }
            }
            break;
            
        case DEVICE_TYPE_MQ2:
            // 尝试读取MQ2数据
            {
                float smoke;
                if (MQ2GetData(&smoke) == 0) {
                    return 0;
                }
            }
            break;
            
        case DEVICE_TYPE_BH1750:
            // 尝试读取BH1750数据
            {
                float light;
                if (BH1750GetData(&light) == 0) {
                    return 0;
                }
            }
            break;
            
        case DEVICE_TYPE_LED:
            // 尝试控制LED
            if (LEDSetColor(LED_COLOR_OFF) == 0) {
                return 0;
            }
            break;
            
        case DEVICE_TYPE_BUZZER:
            // 尝试控制蜂鸣器
            if (BuzzerSetVolume(0) == 0) {
                return 0;
            }
            break;
            
        default:
            break;
    }
    
    return -1;
}

// 保存状态
int StateManagerSave(void)
{
    // 保存到KV存储
    int ret = UtilsKvStoreSet(STATE_KEY, (const unsigned char*)&g_state_manager, sizeof(StateManager), 0);
    if (ret != 0) {
        g_state_manager.error = STATE_ERROR_STORAGE;
        return -1;
    }
    
    return 0;
}

// 加载状态
int StateManagerLoad(void)
{
    // 从KV存储中读取状态
    StateManager temp_manager;
    size_t len = sizeof(StateManager);
    
    int ret = UtilsKvStoreGet(STATE_KEY, (unsigned char*)&temp_manager, &len);
    if (ret != 0) {
        g_state_manager.error = STATE_ERROR_STORAGE;
        return -1;
    }
    
    // 更新状态
    memcpy(&g_state_manager, &temp_manager, sizeof(StateManager));
    return 0;
}

// 反初始化状态管理器
int StateManagerDeinit(void)
{
    // 停止恢复定时器
    if (g_state_manager.recovery_timer != NULL) {
        osTimerDelete(g_state_manager.recovery_timer);
        g_state_manager.recovery_timer = NULL;
    }
    
    // 保存当前状态
    StateManagerSave();
    
    return 0;
} 