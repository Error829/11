#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <stdbool.h>
#include "business/alarm.h"

#ifdef __cplusplus
extern "C" {
#endif

// 配置版本号
#define CONFIG_VERSION 1

// 配置错误码定义
typedef enum {
    CONFIG_ERROR_NONE = 0,    // 无错误
    CONFIG_ERROR_PARAM,       // 参数错误
    CONFIG_ERROR_STORAGE,     // 存储错误
    CONFIG_ERROR_VERSION,     // 版本错误
    CONFIG_ERROR_VALIDATE,    // 验证失败
    CONFIG_ERROR_MAX
} ConfigError;

// 系统基本配置
typedef struct {
    uint32_t collect_interval;    // 数据采集间隔(ms)
    uint32_t check_interval;      // 报警检查间隔(ms)
    uint32_t record_capacity;     // 报警记录容量
    bool log_enabled;             // 是否启用日志
} SystemConfig;

// 传感器配置
typedef struct {
    bool dht11_enabled;           // 是否启用DHT11
    bool mq2_enabled;             // 是否启用MQ2
    bool bh1750_enabled;          // 是否启用BH1750
    uint32_t cache_size;          // 数据缓存大小
} SensorConfig;

// LED配置
typedef struct {
    bool enabled;                 // 是否��用LED
    uint32_t brightness;          // 亮度(0-100)
    uint32_t blink_interval;      // 闪烁间隔(ms)
} LEDConfig;

// 蜂鸣器配置
typedef struct {
    bool enabled;                 // 是否启用蜂鸣器
    uint32_t volume;             // 音量(0-100)
    uint32_t duration;           // 报警持续时间(ms)
} BuzzerConfig;

// 配置结构体
typedef struct {
    uint32_t version;            // 配置版本号
    SystemConfig system;         // 系统配置
    SensorConfig sensor;         // 传感器配置
    LEDConfig led;              // LED配置
    BuzzerConfig buzzer;        // 蜂鸣器配置
    AlarmRule rules[32];        // 报警规则(最多32条)
    uint32_t rule_count;        // 实际的报警规则数量
} Config;

// 初始化配置管理模块
int ConfigInit(void);

// 加载配置
int ConfigLoad(void);

// 保存配置
int ConfigSave(void);

// 获取当前配置
const Config* ConfigGet(void);

// 设置系统配置
int ConfigSetSystem(const SystemConfig* config);

// 设置传感器配置
int ConfigSetSensor(const SensorConfig* config);

// 设置LED配置
int ConfigSetLED(const LEDConfig* config);

// 设置蜂鸣器配置
int ConfigSetBuzzer(const BuzzerConfig* config);

// 添加报警规则
int ConfigAddAlarmRule(const AlarmRule* rule);

// 删除报警规���
int ConfigDeleteAlarmRule(uint32_t index);

// 修改报警规则
int ConfigUpdateAlarmRule(uint32_t index, const AlarmRule* rule);

// 获取配置错误码
ConfigError ConfigGetError(void);

// 恢复默认配置
int ConfigRestoreDefaults(void);

// 反初始化配置管理模块
int ConfigDeinit(void);

#ifdef __cplusplus
}
#endif

#endif // CONFIG_H 