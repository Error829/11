#ifndef SMART_CONTROLLER_H
#define SMART_CONTROLLER_H

#include <stdint.h>
#include "data/data_collector.h"

#ifdef __cplusplus
extern "C" {
#endif

// 控制器状态
typedef enum {
    CONTROLLER_STATE_IDLE = 0,    // 空闲状态
    CONTROLLER_STATE_RUNNING,     // 运行状态
    CONTROLLER_STATE_ERROR        // 错误状态
} ControllerState;

// 控制器错误码
typedef enum {
    CONTROLLER_ERROR_NONE = 0,    // 无错误
    CONTROLLER_ERROR_INIT,        // 初始化错误
    CONTROLLER_ERROR_PARAM,       // 参数错误
    CONTROLLER_ERROR_RULE,        // 规则错误
    CONTROLLER_ERROR_DEVICE       // 设备错误
} ControllerError;

// 控制规则类型
typedef enum {
    RULE_TYPE_THRESHOLD = 0,      // 阈值规则
    RULE_TYPE_TIMING,            // 定时规则
    RULE_TYPE_LINKAGE            // 联动规则
} RuleType;

// 阈值规则配置
typedef struct {
    float threshold;              // 阈值
    float hysteresis;            // 迟滞值
    uint8_t is_upper_bound;      // 是否为上限阈值
} ThresholdConfig;

// 定时规则配置
typedef struct {
    uint32_t start_time;         // 开始时间(秒)
    uint32_t end_time;           // 结束时间(秒)
    uint32_t interval;           // 间隔时间(秒)
} TimingConfig;

// 联动规则配置
typedef struct {
    uint8_t trigger_device;      // 触发设备
    uint8_t target_device;       // 目标设备
    uint8_t action;              // 联动动作
} LinkageConfig;

// 控制规则
typedef struct {
    RuleType type;               // 规则类型
    uint8_t enabled;             // 是否启用
    uint8_t device_id;           // 设备ID
    union {
        ThresholdConfig threshold;
        TimingConfig timing;
        LinkageConfig linkage;
    } config;                    // 规则配置
} ControlRule;

// 控制历史记录
typedef struct {
    uint32_t timestamp;          // 时间戳
    uint8_t device_id;           // 设备ID
    uint8_t action;              // 控制动作
    uint8_t result;              // 执行结果
} ControlHistory;

// 控制器配置
typedef struct {
    uint32_t max_rules;          // 最大规则数
    uint32_t history_size;       // 历史记录大小
} ControllerConfig;

// 初始化控制器
int ControllerInit(const ControllerConfig* config);

// 反初始化控制器
void ControllerDeinit(void);

// 添加控制规则
int ControllerAddRule(const ControlRule* rule);

// 删除控��规则
int ControllerRemoveRule(uint8_t device_id, RuleType type);

// 启用/禁用规则
int ControllerEnableRule(uint8_t device_id, RuleType type, uint8_t enabled);

// 获取控制器状态
ControllerState ControllerGetState(void);

// 获取控制器错误码
ControllerError ControllerGetError(void);

// 获取控制历史记录
int ControllerGetHistory(ControlHistory* history, uint32_t* count);

// 清除控制历史记录
void ControllerClearHistory(void);

// 处理传感器数据
void ControllerHandleData(const SensorData* data);

#ifdef __cplusplus
}
#endif

#endif // SMART_CONTROLLER_H 