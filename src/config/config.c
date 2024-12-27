#include <stdio.h>
#include <string.h>
#include "config/config.h"
#include "utils/kv_store/kv_store.h"

// 配置存储的键名
#define CONFIG_KEY "spacestation_config"

// 当前配置
static Config g_config;
static ConfigError g_error = CONFIG_ERROR_NONE;

// 默认系统配置
static const SystemConfig g_default_system_config = {
    .collect_interval = 1000,    // 1秒采集一次
    .check_interval = 1000,      // 1秒检查一次报警
    .record_capacity = 100,      // 最多保存100条报警记录
    .log_enabled = true         // 默认启用日志
};

// 默认传感器配置
static const SensorConfig g_default_sensor_config = {
    .dht11_enabled = true,      // 默认启用DHT11
    .mq2_enabled = true,        // 默认启用MQ2
    .bh1750_enabled = true,     // 默认启用BH1750
    .cache_size = 10           // 默认缓存10条数据
};

// 默认LED配置
static const LEDConfig g_default_led_config = {
    .enabled = true,           // 默认启用LED
    .brightness = 100,         // 默认最大亮度
    .blink_interval = 500      // 默认闪烁间隔500ms
};

// 默认蜂鸣器配置
static const BuzzerConfig g_default_buzzer_config = {
    .enabled = true,           // 默认启用蜂鸣器
    .volume = 80,             // 默认音量80%
    .duration = 1000          // 默认报警持续1秒
};

// 默认报警规则
static const AlarmRule g_default_rules[] = {
    {
        .type = ALARM_TYPE_TEMPERATURE_HIGH,
        .level = ALARM_LEVEL_CRITICAL,
        .thresholdHigh = 30.0f,
        .thresholdLow = -999.0f,
        .isEnabled = true,
        .delaySeconds = 0
    },
    {
        .type = ALARM_TYPE_TEMPERATURE_LOW,
        .level = ALARM_LEVEL_CRITICAL,
        .thresholdHigh = 999.0f,
        .thresholdLow = 10.0f,
        .isEnabled = true,
        .delaySeconds = 0
    }
    // 其他默认规则...
};

// 验证配置有效性
static bool ValidateConfig(const Config* config)
{
    if (config == NULL) {
        return false;
    }
    
    // 验证版本号
    if (config->version != CONFIG_VERSION) {
        return false;
    }
    
    // 验证系统配置
    if (config->system.collect_interval < 100 ||
        config->system.check_interval < 100 ||
        config->system.record_capacity == 0) {
        return false;
    }
    
    // 验证传感器配置
    if (config->sensor.cache_size == 0) {
        return false;
    }
    
    // 验证LED配置
    if (config->led.brightness > 100) {
        return false;
    }
    
    // 验证蜂鸣器配置
    if (config->buzzer.volume > 100) {
        return false;
    }
    
    // 验证报警规则
    if (config->rule_count > 32) {
        return false;
    }
    
    return true;
}

// 加载默认配置
static void LoadDefaultConfig(void)
{
    // 设置版本号
    g_config.version = CONFIG_VERSION;
    
    // 加载系统配置
    memcpy(&g_config.system, &g_default_system_config, sizeof(SystemConfig));
    
    // 加载传感器配置
    memcpy(&g_config.sensor, &g_default_sensor_config, sizeof(SensorConfig));
    
    // 加载LED配置
    memcpy(&g_config.led, &g_default_led_config, sizeof(LEDConfig));
    
    // 加载蜂鸣器配置
    memcpy(&g_config.buzzer, &g_default_buzzer_config, sizeof(BuzzerConfig));
    
    // 加载默认报警规则
    g_config.rule_count = sizeof(g_default_rules) / sizeof(AlarmRule);
    memcpy(g_config.rules, g_default_rules, sizeof(g_default_rules));
}

// 初始化配置管理模块
int ConfigInit(void)
{
    // 初始化KV存储
    int ret = UtilsKvStoreInit();
    if (ret != 0) {
        g_error = CONFIG_ERROR_STORAGE;
        return -1;
    }
    
    // 加载配置
    ret = ConfigLoad();
    if (ret != 0) {
        // 加载失败时使用默认配置
        LoadDefaultConfig();
    }
    
    return 0;
}

// 加载配置
int ConfigLoad(void)
{
    // 从KV存储中读取配置
    Config temp_config;
    size_t len = sizeof(Config);
    
    int ret = UtilsKvStoreGet(CONFIG_KEY, (unsigned char*)&temp_config, &len);
    if (ret != 0) {
        g_error = CONFIG_ERROR_STORAGE;
        return -1;
    }
    
    // 验证配置有效性
    if (!ValidateConfig(&temp_config)) {
        g_error = CONFIG_ERROR_VALIDATE;
        return -1;
    }
    
    // 更新当前配置
    memcpy(&g_config, &temp_config, sizeof(Config));
    g_error = CONFIG_ERROR_NONE;
    
    return 0;
}

// 保存配置
int ConfigSave(void)
{
    // 验证当前配置
    if (!ValidateConfig(&g_config)) {
        g_error = CONFIG_ERROR_VALIDATE;
        return -1;
    }
    
    // 保存到KV存储
    int ret = UtilsKvStoreSet(CONFIG_KEY, (const unsigned char*)&g_config, sizeof(Config), 0);
    if (ret != 0) {
        g_error = CONFIG_ERROR_STORAGE;
        return -1;
    }
    
    g_error = CONFIG_ERROR_NONE;
    return 0;
}

// 获取当前配置
const Config* ConfigGet(void)
{
    return &g_config;
}

// 设置系统配置
int ConfigSetSystem(const SystemConfig* config)
{
    if (config == NULL) {
        g_error = CONFIG_ERROR_PARAM;
        return -1;
    }
    
    memcpy(&g_config.system, config, sizeof(SystemConfig));
    return ConfigSave();
}

// 设置传感器配置
int ConfigSetSensor(const SensorConfig* config)
{
    if (config == NULL) {
        g_error = CONFIG_ERROR_PARAM;
        return -1;
    }
    
    memcpy(&g_config.sensor, config, sizeof(SensorConfig));
    return ConfigSave();
}

// 设置LED配置
int ConfigSetLED(const LEDConfig* config)
{
    if (config == NULL) {
        g_error = CONFIG_ERROR_PARAM;
        return -1;
    }
    
    memcpy(&g_config.led, config, sizeof(LEDConfig));
    return ConfigSave();
}

// 设置蜂鸣器配置
int ConfigSetBuzzer(const BuzzerConfig* config)
{
    if (config == NULL) {
        g_error = CONFIG_ERROR_PARAM;
        return -1;
    }
    
    memcpy(&g_config.buzzer, config, sizeof(BuzzerConfig));
    return ConfigSave();
}

// 添加报警规则
int ConfigAddAlarmRule(const AlarmRule* rule)
{
    if (rule == NULL || g_config.rule_count >= 32) {
        g_error = CONFIG_ERROR_PARAM;
        return -1;
    }
    
    memcpy(&g_config.rules[g_config.rule_count], rule, sizeof(AlarmRule));
    g_config.rule_count++;
    
    return ConfigSave();
}

// 删除报警规则
int ConfigDeleteAlarmRule(uint32_t index)
{
    if (index >= g_config.rule_count) {
        g_error = CONFIG_ERROR_PARAM;
        return -1;
    }
    
    // 移动后面的规则
    for (uint32_t i = index; i < g_config.rule_count - 1; i++) {
        memcpy(&g_config.rules[i], &g_config.rules[i + 1], sizeof(AlarmRule));
    }
    g_config.rule_count--;
    
    return ConfigSave();
}

// 修改报警规则
int ConfigUpdateAlarmRule(uint32_t index, const AlarmRule* rule)
{
    if (rule == NULL || index >= g_config.rule_count) {
        g_error = CONFIG_ERROR_PARAM;
        return -1;
    }
    
    memcpy(&g_config.rules[index], rule, sizeof(AlarmRule));
    return ConfigSave();
}

// 获取配置错误码
ConfigError ConfigGetError(void)
{
    return g_error;
}

// 恢复默认配置
int ConfigRestoreDefaults(void)
{
    LoadDefaultConfig();
    return ConfigSave();
}

// 反初始化配置管理模块
int ConfigDeinit(void)
{
    // 保存当前配置
    ConfigSave();
    
    // 反初始化KV存储
    UtilsKvStoreDeInit();
    
    return 0;
} 