#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cmsis_os2.h"
#include "control/smart_controller.h"
#include "drivers/output/relay.h"
#include "drivers/output/buzzer.h"
#include "drivers/output/led.h"

// 控制规则结构
typedef struct {
    ControlRule rule;            // 规则配置
    uint8_t used;               // 是否使用
} RuleEntry;

// 全局变量
static ControllerState g_state = CONTROLLER_STATE_IDLE;
static ControllerError g_error = CONTROLLER_ERROR_NONE;
static ControllerConfig g_config = {0};
static RuleEntry* g_rules = NULL;
static ControlHistory* g_history = NULL;
static uint32_t g_history_count = 0;
static uint32_t g_history_index = 0;

// 更新状态
static void UpdateState(ControllerState state, ControllerError error)
{
    g_state = state;
    g_error = error;
}

// 记录控制历史
static void RecordHistory(uint8_t device_id, uint8_t action, uint8_t result)
{
    if (g_history == NULL || g_config.history_size == 0) {
        return;
    }

    ControlHistory* history = &g_history[g_history_index];
    history->timestamp = osKernelGetTickCount();
    history->device_id = device_id;
    history->action = action;
    history->result = result;

    g_history_index = (g_history_index + 1) % g_config.history_size;
    if (g_history_count < g_config.history_size) {
        g_history_count++;
    }
}

// 执行设备控制
static int ExecuteControl(uint8_t device_id, uint8_t action)
{
    int ret = 0;

    switch (device_id) {
        case 0: // 继电器
            ret = RelayControl(action);
            break;
        case 1: // 蜂鸣器
            ret = BuzzerControl(action);
            break;
        case 2: // LED
            ret = LEDControl(action);
            break;
        default:
            ret = -1;
            break;
    }

    RecordHistory(device_id, action, ret == 0 ? 1 : 0);
    return ret;
}

// 检查阈值规则
static void CheckThresholdRule(const RuleEntry* rule, float value)
{
    if (!rule->used || !rule->rule.enabled) {
        return;
    }

    const ThresholdConfig* config = &rule->rule.config.threshold;
    uint8_t need_action = 0;

    if (config->is_upper_bound) {
        // 上限阈值
        if (value >= config->threshold + config->hysteresis) {
            need_action = 1;
        } else if (value <= config->threshold - config->hysteresis) {
            need_action = 0;
        }
    } else {
        // 下限阈值
        if (value <= config->threshold - config->hysteresis) {
            need_action = 1;
        } else if (value >= config->threshold + config->hysteresis) {
            need_action = 0;
        }
    }

    ExecuteControl(rule->rule.device_id, need_action);
}

// 检查定时规则
static void CheckTimingRule(const RuleEntry* rule)
{
    if (!rule->used || !rule->rule.enabled) {
        return;
    }

    const TimingConfig* config = &rule->rule.config.timing;
    uint32_t current_time = osKernelGetTickCount() / 1000; // 转换为秒
    uint32_t day_seconds = current_time % (24 * 3600);

    if (day_seconds >= config->start_time && day_seconds < config->end_time) {
        if (config->interval == 0 || (day_seconds - config->start_time) % config->interval == 0) {
            ExecuteControl(rule->rule.device_id, 1);
        }
    } else {
        ExecuteControl(rule->rule.device_id, 0);
    }
}

// 检查联动规则
static void CheckLinkageRule(const RuleEntry* rule, uint8_t trigger_device, uint8_t trigger_state)
{
    if (!rule->used || !rule->rule.enabled) {
        return;
    }

    const LinkageConfig* config = &rule->rule.config.linkage;
    if (config->trigger_device == trigger_device) {
        ExecuteControl(config->target_device, trigger_state ? config->action : !config->action);
    }
}

// 初始化控��器
int ControllerInit(const ControllerConfig* config)
{
    if (config == NULL || config->max_rules == 0) {
        UpdateState(CONTROLLER_STATE_ERROR, CONTROLLER_ERROR_PARAM);
        return -1;
    }

    // 分配规则存储空间
    g_rules = malloc(sizeof(RuleEntry) * config->max_rules);
    if (g_rules == NULL) {
        UpdateState(CONTROLLER_STATE_ERROR, CONTROLLER_ERROR_INIT);
        return -1;
    }
    memset(g_rules, 0, sizeof(RuleEntry) * config->max_rules);

    // 分配历史记录存储空间
    if (config->history_size > 0) {
        g_history = malloc(sizeof(ControlHistory) * config->history_size);
        if (g_history == NULL) {
            free(g_rules);
            g_rules = NULL;
            UpdateState(CONTROLLER_STATE_ERROR, CONTROLLER_ERROR_INIT);
            return -1;
        }
        memset(g_history, 0, sizeof(ControlHistory) * config->history_size);
    }

    // 保存配置
    memcpy(&g_config, config, sizeof(ControllerConfig));
    UpdateState(CONTROLLER_STATE_RUNNING, CONTROLLER_ERROR_NONE);
    return 0;
}

// 反初始化控制器
void ControllerDeinit(void)
{
    if (g_rules != NULL) {
        free(g_rules);
        g_rules = NULL;
    }

    if (g_history != NULL) {
        free(g_history);
        g_history = NULL;
    }

    g_history_count = 0;
    g_history_index = 0;
    UpdateState(CONTROLLER_STATE_IDLE, CONTROLLER_ERROR_NONE);
}

// 添加控制规则
int ControllerAddRule(const ControlRule* rule)
{
    if (rule == NULL || g_rules == NULL) {
        UpdateState(CONTROLLER_STATE_ERROR, CONTROLLER_ERROR_PARAM);
        return -1;
    }

    // 查找空闲位置
    uint32_t i;
    for (i = 0; i < g_config.max_rules; i++) {
        if (!g_rules[i].used) {
            memcpy(&g_rules[i].rule, rule, sizeof(ControlRule));
            g_rules[i].used = 1;
            return 0;
        }
    }

    UpdateState(CONTROLLER_STATE_ERROR, CONTROLLER_ERROR_RULE);
    return -1;
}

// 删除控制规则
int ControllerRemoveRule(uint8_t device_id, RuleType type)
{
    if (g_rules == NULL) {
        UpdateState(CONTROLLER_STATE_ERROR, CONTROLLER_ERROR_PARAM);
        return -1;
    }

    uint32_t i;
    for (i = 0; i < g_config.max_rules; i++) {
        if (g_rules[i].used && 
            g_rules[i].rule.device_id == device_id &&
            g_rules[i].rule.type == type) {
            g_rules[i].used = 0;
            return 0;
        }
    }

    return -1;
}

// 启用/禁用规则
int ControllerEnableRule(uint8_t device_id, RuleType type, uint8_t enabled)
{
    if (g_rules == NULL) {
        UpdateState(CONTROLLER_STATE_ERROR, CONTROLLER_ERROR_PARAM);
        return -1;
    }

    uint32_t i;
    for (i = 0; i < g_config.max_rules; i++) {
        if (g_rules[i].used && 
            g_rules[i].rule.device_id == device_id &&
            g_rules[i].rule.type == type) {
            g_rules[i].rule.enabled = enabled;
            return 0;
        }
    }

    return -1;
}

// 获取控制器状态
ControllerState ControllerGetState(void)
{
    return g_state;
}

// 获取控制器错误码
ControllerError ControllerGetError(void)
{
    return g_error;
}

// 获取控制历史记录
int ControllerGetHistory(ControlHistory* history, uint32_t* count)
{
    if (history == NULL || count == NULL || g_history == NULL) {
        return -1;
    }

    uint32_t copy_count = *count < g_history_count ? *count : g_history_count;
    uint32_t start = (g_history_index - g_history_count + g_config.history_size) % g_config.history_size;

    uint32_t i;
    for (i = 0; i < copy_count; i++) {
        uint32_t src_index = (start + i) % g_config.history_size;
        memcpy(&history[i], &g_history[src_index], sizeof(ControlHistory));
    }

    *count = copy_count;
    return 0;
}

// 清除控制历史记录
void ControllerClearHistory(void)
{
    g_history_count = 0;
    g_history_index = 0;
}

// 处理传感器数据
void ControllerHandleData(const SensorData* data)
{
    if (data == NULL || g_rules == NULL) {
        return;
    }

    float value = 0.0f;
    uint32_t i;

    // 根据传感器类型获取数值
    switch (data->type) {
        case SENSOR_TYPE_DHT11:
            value = data->data.dht11.temperature;
            break;
        case SENSOR_TYPE_MQ2:
            value = data->data.mq2.smoke;
            break;
        case SENSOR_TYPE_BH1750:
            value = data->data.bh1750.light;
            break;
        default:
            return;
    }

    // 检查所有规则
    for (i = 0; i < g_config.max_rules; i++) {
        if (!g_rules[i].used) {
            continue;
        }

        switch (g_rules[i].rule.type) {
            case RULE_TYPE_THRESHOLD:
                CheckThresholdRule(&g_rules[i], value);
                break;
            case RULE_TYPE_TIMING:
                CheckTimingRule(&g_rules[i]);
                break;
            case RULE_TYPE_LINKAGE:
                // 联动规则在设备状态变化时处理
                break;
            default:
                break;
        }
    }
} 