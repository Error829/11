#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// 系统状态定义
typedef enum {
    SYSTEM_STATE_INIT,       // 初始化状态
    SYSTEM_STATE_RUNNING,    // 运行状态
    SYSTEM_STATE_ERROR,      // 错误状态
    SYSTEM_STATE_STOP        // 停止状态
} SystemState;

// 系统错误码定义
typedef enum {
    SYSTEM_ERROR_NONE = 0,      // 无错误
    SYSTEM_ERROR_INIT_FAILED,   // 初始化失败
    SYSTEM_ERROR_SENSOR,        // 传感器错误
    SYSTEM_ERROR_COLLECTOR,     // 数据采集错误
    SYSTEM_ERROR_ALARM,         // 报警管理错误
    SYSTEM_ERROR_MEMORY,        // 内存错误
    SYSTEM_ERROR_TIMER,         // 定时器错误
    SYSTEM_ERROR_MAX
} SystemError;

// 系统配置结构体
typedef struct {
    uint32_t collect_interval;   // 数据采集间隔(ms)
    uint32_t check_interval;     // 报警检查间隔(ms)
    uint32_t record_capacity;    // 报警记录容量
} SystemConfig;

// 获取系统状态
SystemState GetSystemState(void);

// 获取系统错误码
SystemError GetSystemError(void);

// 获取系统运行时间
uint32_t GetSystemUptime(void);

// 系统初始化
int SystemInit(const SystemConfig* config);

// 系统启动
int SystemStart(void);

// 系统停止
int SystemStop(void);

// 系统反初始化
int SystemDeinit(void);

// 空间站环境监控系统主函数
int SpaceStationMain(void);

#ifdef __cplusplus
}
#endif

#endif // MAIN_H 