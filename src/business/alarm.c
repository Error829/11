#include "business/alarm.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "drivers/output/led.h"
#include "drivers/output/buzzer.h"
#include "data/data_collector.h"
#include "cmsis_os2.h"

// 定义报警类型数量
#define ALARM_TYPE_COUNT 8

// 报警规则数组
static AlarmRule g_alarm_rules[ALARM_TYPE_COUNT] = {0};

// 报警记录缓存
static AlarmRecord* g_alarm_records = NULL;
static uint32_t g_record_capacity = 0;
static uint32_t g_record_count = 0;
static uint32_t g_record_index = 0;

// 报警回调函数
static AlarmCallback g_alarm_callback = NULL;

// 获取报警级别对应的LED颜色
static LEDColor GetAlarmLevelColor(AlarmLevel level)
{
    switch (level) {
        case ALARM_LEVEL_INFO:
            return LED_COLOR_GREEN;
        case ALARM_LEVEL_WARNING:
            return LED_COLOR_BLUE;  // 使用蓝色代替黄色
        case ALARM_LEVEL_CRITICAL:
            return LED_COLOR_RED;
        default:
            return LED_COLOR_OFF;
    }
}

// 获取传感器类型
static SensorType GetSensorType(AlarmType type)
{
    switch (type) {
        case ALARM_TYPE_TEMPERATURE_HIGH:
        case ALARM_TYPE_TEMPERATURE_LOW:
            return SENSOR_TYPE_DHT11;
        case ALARM_TYPE_HUMIDITY_HIGH:
        case ALARM_TYPE_HUMIDITY_LOW:
            return SENSOR_TYPE_DHT11;
        case ALARM_TYPE_SMOKE:
            return SENSOR_TYPE_MQ2;
        case ALARM_TYPE_LIGHT_HIGH:
        case ALARM_TYPE_LIGHT_LOW:
            return SENSOR_TYPE_BH1750;
        default:
            return SENSOR_TYPE_MAX;
    }
}

// 获取传感器数据值
static float GetSensorValue(AlarmType type, const SensorData* data)
{
    switch (type) {
        case ALARM_TYPE_TEMPERATURE_HIGH:
        case ALARM_TYPE_TEMPERATURE_LOW:
            return data->data.dht11.temperature;
        case ALARM_TYPE_HUMIDITY_HIGH:
        case ALARM_TYPE_HUMIDITY_LOW:
            return data->data.dht11.humidity;
        case ALARM_TYPE_SMOKE:
            return data->data.mq2.smoke;
        case ALARM_TYPE_LIGHT_HIGH:
        case ALARM_TYPE_LIGHT_LOW:
            return data->data.bh1750.light;
        default:
            return 0.0f;
    }
}

// 检查是否超过阈值
static bool CheckThreshold(AlarmType type, float value, const AlarmRule* rule)
{
    switch (type) {
        case ALARM_TYPE_TEMPERATURE_HIGH:
        case ALARM_TYPE_HUMIDITY_HIGH:
        case ALARM_TYPE_LIGHT_HIGH:
            return value >= rule->thresholdHigh;
        case ALARM_TYPE_TEMPERATURE_LOW:
        case ALARM_TYPE_HUMIDITY_LOW:
        case ALARM_TYPE_LIGHT_LOW:
            return value <= rule->thresholdLow;
        case ALARM_TYPE_SMOKE:
            return value >= rule->thresholdHigh;
        default:
            return false;
    }
}

// 生成报警描述
static void GenerateDescription(char* desc, size_t size, AlarmType type, float value)
{
    const char* type_str = NULL;
    switch (type) {
        case ALARM_TYPE_TEMPERATURE_HIGH:
            type_str = "温度过高";
            break;
        case ALARM_TYPE_TEMPERATURE_LOW:
            type_str = "温度过低";
            break;
        case ALARM_TYPE_HUMIDITY_HIGH:
            type_str = "湿度过高";
            break;
        case ALARM_TYPE_HUMIDITY_LOW:
            type_str = "湿度过低";
            break;
        case ALARM_TYPE_SMOKE:
            type_str = "烟雾";
            break;
        case ALARM_TYPE_LIGHT_HIGH:
            type_str = "光照过强";
            break;
        case ALARM_TYPE_LIGHT_LOW:
            type_str = "光照不足";
            break;
        case ALARM_TYPE_SYSTEM_ERROR:
            type_str = "系统错误";
            break;
        default:
            type_str = "未知";
            break;
    }
    
    snprintf(desc, size, "%s报警, 当前值: %.2f", type_str, value);
}

// 触发报警
static void TriggerAlarm(const AlarmRule* rule, float value)
{
    // 创建报警记录
    AlarmRecord record = {
        .type = rule->type,
        .level = rule->level,
        .value = value,
        .timestamp = time(NULL)
    };
    
    // 生成报警描述
    GenerateDescription(record.description, sizeof(record.description), 
        rule->type, value);
    
    // 保存报警记录
    if (g_alarm_records != NULL) {
        memcpy(&g_alarm_records[g_record_index], &record, sizeof(AlarmRecord));
        g_record_index = (g_record_index + 1) % g_record_capacity;
        if (g_record_count < g_record_capacity) {
            g_record_count++;
        }
    }
    
    // 控制LED指示
    LEDSetColor(GetAlarmLevelColor(rule->level));
    if (rule->level >= ALARM_LEVEL_WARNING) {
        LEDSetBlink(true, 500);  // 警告及以上级别报警LED闪烁
    }
    
    // 控制蜂鸣器
    if (rule->level >= ALARM_LEVEL_CRITICAL) {
        BuzzerStart(1000, 500);  // 严重级别报警蜂鸣器报警
    }
    
    // 调用回调函数
    if (g_alarm_callback != NULL) {
        g_alarm_callback(&record);
    }
}

// 检查报警条件
int AlarmCheck(void)
{
    SensorData data = {0};
    
    // 检查每个报警规则
    for (int i = 0; i < ALARM_TYPE_COUNT; i++) {
        AlarmRule* rule = &g_alarm_rules[i];
        if (!rule->isEnabled) {
            continue;
        }
        
        // 获取对应的传感器数据
        SensorType sensor_type = GetSensorType(rule->type);
        if (sensor_type >= SENSOR_TYPE_MAX) {
            continue;
        }
        
        if (CollectorGetLatestData(sensor_type, &data) != 0) {
            continue;
        }
        
        // 获取传感器值并检查阈值
        float value = GetSensorValue(rule->type, &data);
        if (CheckThreshold(rule->type, value, rule)) {
            TriggerAlarm(rule, value);
        }
    }
    
    return 0;
}

// 初始化报警管理模块
int AlarmInit(void)
{
    // 默认分配100条记录的缓存
    const uint32_t DEFAULT_RECORD_CAPACITY = 100;
    
    // 分配报警记录缓存
    g_alarm_records = malloc(sizeof(AlarmRecord) * DEFAULT_RECORD_CAPACITY);
    if (g_alarm_records == NULL) {
        return -1;
    }
    
    g_record_capacity = DEFAULT_RECORD_CAPACITY;
    g_record_count = 0;
    g_record_index = 0;
    
    // 初始化LED和蜂鸣器
    LEDInit();
    BuzzerInit();
    
    return 0;
}

// 设置报警规则
int AlarmSetRule(const AlarmRule* rule)
{
    if (rule == NULL || rule->type >= ALARM_TYPE_COUNT) {
        return -1;
    }
    
    memcpy(&g_alarm_rules[rule->type], rule, sizeof(AlarmRule));
    return 0;
}

// 获取报警规则
int AlarmGetRule(AlarmType type, AlarmRule* rule)
{
    if (type >= ALARM_TYPE_COUNT || rule == NULL) {
        return -1;
    }
    
    memcpy(rule, &g_alarm_rules[type], sizeof(AlarmRule));
    return 0;
}

// 启用报警规则
int AlarmEnableRule(AlarmType type)
{
    if (type >= ALARM_TYPE_COUNT) {
        return -1;
    }
    
    g_alarm_rules[type].isEnabled = true;
    return 0;
}

// 禁用报警规则
int AlarmDisableRule(AlarmType type)
{
    if (type >= ALARM_TYPE_COUNT) {
        return -1;
    }
    
    g_alarm_rules[type].isEnabled = false;
    return 0;
}

// 获取最近的报警记录
int AlarmGetLatestRecords(AlarmRecord* records, uint32_t maxCount, uint32_t* actualCount)
{
    if (records == NULL || actualCount == NULL) {
        return -1;
    }
    
    // 计算实际返回的记录数
    uint32_t return_count = maxCount > g_record_count ? g_record_count : maxCount;
    
    // 从最新的记录开始复制
    uint32_t start = (g_record_index - return_count + g_record_capacity) % g_record_capacity;
    for (uint32_t i = 0; i < return_count; i++) {
        memcpy(&records[i], &g_alarm_records[(start + i) % g_record_capacity], 
            sizeof(AlarmRecord));
    }
    
    *actualCount = return_count;
    return 0;
}

// 清除报警记录
int AlarmClearRecords(void)
{
    g_record_count = 0;
    g_record_index = 0;
    return 0;
}

// 注册报警回调函数
int AlarmRegisterCallback(AlarmCallback callback)
{
    g_alarm_callback = callback;
    return 0;
}

// 反初始化报警管理模块
int AlarmDeinit(void)
{
    // 释放报警记录缓存
    if (g_alarm_records != NULL) {
        free(g_alarm_records);
        g_alarm_records = NULL;
    }
    
    g_record_capacity = 0;
    g_record_count = 0;
    g_record_index = 0;
    
    // 关闭LED和蜂鸣器
    LEDOff();
    LEDDeinit();
    BuzzerStop();
    BuzzerDeinit();
    
    return 0;
}

// 处理定时器回调
void AlarmTimerCallback(void)
{
    AlarmCheck();
}
