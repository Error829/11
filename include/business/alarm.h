#ifndef BUSINESS_ALARM_H
#define BUSINESS_ALARM_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

// 报警类型定义
typedef enum {
    ALARM_TYPE_TEMPERATURE_HIGH,    // 温度过高报警
    ALARM_TYPE_TEMPERATURE_LOW,     // 温度过低报警
    ALARM_TYPE_HUMIDITY_HIGH,       // 湿度过高报警
    ALARM_TYPE_HUMIDITY_LOW,        // 湿度过低报警
    ALARM_TYPE_SMOKE,              // 烟雾报警
    ALARM_TYPE_LIGHT_HIGH,         // 光照过强报警
    ALARM_TYPE_LIGHT_LOW,          // 光照不足报警
    ALARM_TYPE_SYSTEM_ERROR        // 系统错误报警
} AlarmType;

// 报警级别定义
typedef enum {
    ALARM_LEVEL_INFO,      // 信息
    ALARM_LEVEL_WARNING,   // 警告
    ALARM_LEVEL_CRITICAL   // 严重
} AlarmLevel;

// 报警规则结构体
typedef struct {
    AlarmType type;            // 报警类型
    AlarmLevel level;          // 报警级别
    float thresholdHigh;       // 上限阈值
    float thresholdLow;        // 下限阈值
    bool isEnabled;            // 是否启用
    uint32_t delaySeconds;     // 触发���迟(秒)
} AlarmRule;

// 报警记录结构体
typedef struct {
    AlarmType type;           // 报警类型
    AlarmLevel level;         // 报警级别
    time_t timestamp;         // 触发时间
    float value;              // 触发值
    char description[128];    // 报警描述
} AlarmRecord;

// 初始化报警管理模块
int AlarmInit(void);

// 设置报警规则
int AlarmSetRule(const AlarmRule* rule);

// 获取报警规则
int AlarmGetRule(AlarmType type, AlarmRule* rule);

// 启用报警规则
int AlarmEnableRule(AlarmType type);

// 禁用报警规则
int AlarmDisableRule(AlarmType type);

// 检查报警条件
int AlarmCheck(void);

// 获取最近的报警记录
int AlarmGetLatestRecords(AlarmRecord* records, uint32_t maxCount, uint32_t* actualCount);

// 清除报警记录
int AlarmClearRecords(void);

// 注册报警回调函数
typedef void (*AlarmCallback)(const AlarmRecord* record);
int AlarmRegisterCallback(AlarmCallback callback);

// 反初始化报警管理模块
int AlarmDeinit(void);

#ifdef __cplusplus
}
#endif

#endif // BUSINESS_ALARM_H
