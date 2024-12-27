#include "main.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "cmsis_os2.h"
#include "data/data_collector.h"
#include "business/alarm.h"
#include "drivers/sensor/dht11.h"
#include "drivers/sensor/mq2.h"
#include "drivers/sensor/bh1750.h"
#include "drivers/output/led.h"
#include "drivers/output/buzzer.h"

// 系统状态
static SystemState g_system_state = SYSTEM_STATE_INIT;
static SystemError g_system_error = SYSTEM_ERROR_NONE;
static uint32_t g_system_start_time = 0;

// 定时器
static osTimerId_t g_alarm_timer = NULL;

// 默认报警规则配置
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
    },
    {
        .type = ALARM_TYPE_HUMIDITY_HIGH,
        .level = ALARM_LEVEL_WARNING,
        .thresholdHigh = 80.0f,
        .thresholdLow = -999.0f,
        .isEnabled = true,
        .delaySeconds = 5
    },
    {
        .type = ALARM_TYPE_HUMIDITY_LOW,
        .level = ALARM_LEVEL_WARNING,
        .thresholdHigh = 999.0f,
        .thresholdLow = 20.0f,
        .isEnabled = true,
        .delaySeconds = 5
    },
    {
        .type = ALARM_TYPE_SMOKE,
        .level = ALARM_LEVEL_CRITICAL,
        .thresholdHigh = 100.0f,
        .thresholdLow = -999.0f,
        .isEnabled = true,
        .delaySeconds = 0
    },
    {
        .type = ALARM_TYPE_LIGHT_HIGH,
        .level = ALARM_LEVEL_WARNING,
        .thresholdHigh = 1000.0f,
        .thresholdLow = -999.0f,
        .isEnabled = true,
        .delaySeconds = 3
    },
    {
        .type = ALARM_TYPE_LIGHT_LOW,
        .level = ALARM_LEVEL_WARNING,
        .thresholdHigh = 999.0f,
        .thresholdLow = 10.0f,
        .isEnabled = true,
        .delaySeconds = 3
    }
};

// 更新系统状态
static void UpdateSystemState(SystemState state, SystemError error)
{
    g_system_state = state;
    g_system_error = error;
}

// 报警回调函数
static void HandleAlarm(const AlarmRecord* record)
{
    printf("[ALARM] Type: %d, Level: %d, Value: %.2f, Description: %s\n",
        record->type, record->level, record->value, record->description);
}

// 定时器回调函数
static void AlarmTimerCallback(void* arg)
{
    (void)arg;
    AlarmCheck();
}

// 初始化传感器
static int InitSensors(void)
{
    int ret = 0;
    
    // 初始化DHT11
    ret = DHT11Init();
    if (ret != 0) {
        printf("DHT11 init failed: %d\n", ret);
        return -1;
    }
    
    // 初始化MQ2
    ret = MQ2Init();
    if (ret != 0) {
        printf("MQ2 init failed: %d\n", ret);
        return -1;
    }
    
    // 初始化BH1750
    ret = BH1750Init();
    if (ret != 0) {
        printf("BH1750 init failed: %d\n", ret);
        return -1;
    }
    
    return 0;
}

// 初始化报警规则
static int InitAlarmRules(void)
{
    // 设置默认报警规则
    for (size_t i = 0; i < sizeof(g_default_rules) / sizeof(AlarmRule); i++) {
        if (AlarmSetRule(&g_default_rules[i]) != 0) {
            printf("Set alarm rule failed: %zu\n", i);
            return -1;
        }
    }
    
    return 0;
}

// 获取系统状态
SystemState GetSystemState(void)
{
    return g_system_state;
}

// 重命名为SpaceStationGetSystemError避免冲突
static int SpaceStationGetSystemError(void)
{
    return g_system_error;
}

// 获取系统运行时间
uint32_t GetSystemUptime(void)
{
    if (g_system_start_time == 0) {
        return 0;
    }
    return osKernelGetTickCount() - g_system_start_time;
}

// 系统初始化
int SystemInit(const SystemConfig* config)
{
    if (config == NULL) {
        UpdateSystemState(SYSTEM_STATE_ERROR, SYSTEM_ERROR_INIT_FAILED);
        return -1;
    }
    
    int ret = 0;
    
    // 初始化传感器
    ret = InitSensors();
    if (ret != 0) {
        UpdateSystemState(SYSTEM_STATE_ERROR, SYSTEM_ERROR_SENSOR);
        return -1;
    }
    
    // 初始化数据采集模块
    CollectorConfig collector_config = {
        .collect_interval = config->collect_interval,
        .cache_size = 10  // 每个传感器缓存10条数据
    };
    ret = CollectorInit(&collector_config);
    if (ret != 0) {
        UpdateSystemState(SYSTEM_STATE_ERROR, SYSTEM_ERROR_COLLECTOR);
        return -1;
    }
    
    // 初始化报警管理模块
    ret = AlarmInit();
    if (ret != 0) {
        UpdateSystemState(SYSTEM_STATE_ERROR, SYSTEM_ERROR_ALARM);
        return -1;
    }
    
    // 注册报警回调函数
    ret = AlarmRegisterCallback(HandleAlarm);
    if (ret != 0) {
        UpdateSystemState(SYSTEM_STATE_ERROR, SYSTEM_ERROR_ALARM);
        return -1;
    }
    
    // 初始化报警规则
    ret = InitAlarmRules();
    if (ret != 0) {
        UpdateSystemState(SYSTEM_STATE_ERROR, SYSTEM_ERROR_ALARM);
        return -1;
    }
    
    // 创建报警检查定时器
    osTimerAttr_t timer_attr = {
        .name = "AlarmTimer",
        .attr_bits = 0,
        .cb_mem = NULL,
        .cb_size = 0
    };
    g_alarm_timer = osTimerNew(AlarmTimerCallback, osTimerPeriodic, NULL, &timer_attr);
    if (g_alarm_timer == NULL) {
        UpdateSystemState(SYSTEM_STATE_ERROR, SYSTEM_ERROR_TIMER);
        return -1;
    }
    
    UpdateSystemState(SYSTEM_STATE_INIT, SYSTEM_ERROR_NONE);
    return 0;
}

// 系统启动
int SystemStart(void)
{
    if (g_system_state != SYSTEM_STATE_INIT) {
        return -1;
    }
    
    // 启动数据采集
    int ret = CollectorStart();
    if (ret != 0) {
        UpdateSystemState(SYSTEM_STATE_ERROR, SYSTEM_ERROR_COLLECTOR);
        return -1;
    }
    
    // 启动报警检查定时器
    ret = osTimerStart(g_alarm_timer, 1000);  // 1秒检查一次
    if (ret != osOK) {
        UpdateSystemState(SYSTEM_STATE_ERROR, SYSTEM_ERROR_TIMER);
        return -1;
    }
    
    // 记录启动时间
    g_system_start_time = osKernelGetTickCount();
    
    UpdateSystemState(SYSTEM_STATE_RUNNING, SYSTEM_ERROR_NONE);
    return 0;
}

// 系统停止
int SystemStop(void)
{
    if (g_system_state != SYSTEM_STATE_RUNNING) {
        return -1;
    }
    
    // 停止报警检查定时器
    if (g_alarm_timer != NULL) {
        osTimerStop(g_alarm_timer);
    }
    
    // 停止数据采集
    CollectorStop();
    
    UpdateSystemState(SYSTEM_STATE_STOP, SYSTEM_ERROR_NONE);
    return 0;
}

// 系统反初始化
int SystemDeinit(void)
{
    // 删除报警检查定时器
    if (g_alarm_timer != NULL) {
        osTimerDelete(g_alarm_timer);
        g_alarm_timer = NULL;
    }
    
    // 反初始化各个模块
    AlarmDeinit();
    CollectorDeinit();
    DHT11Deinit();
    MQ2Deinit();
    BH1750Deinit();
    
    UpdateSystemState(SYSTEM_STATE_INIT, SYSTEM_ERROR_NONE);
    return 0;
}

// 处理系统事件
static void ProcessSystemEvents(void)
{
    // 处理传感器数据
    SensorData data;
    if (CollectorGetLatestData(SENSOR_TYPE_ALL, &data) == 0) {
        // 更新LED状态
        if (data.data.dht11.temperature > 30.0f) {
            LEDSetColor(LED_COLOR_RED);
        } else if (data.data.dht11.temperature < 10.0f) {
            LEDSetColor(LED_COLOR_BLUE);
        } else {
            LEDSetColor(LED_COLOR_GREEN);
        }
        
        // 打印传感器数据
        printf("Temperature: %.1f°C, Humidity: %.1f%%, Smoke: %.1fppm, Light: %.1flx\n",
            data.data.dht11.temperature, 
            data.data.dht11.humidity,
            data.data.mq2.smoke,
            data.data.bh1750.light);
    }
}

// 重命名main函数为SpaceStationMain
int SpaceStationMain(void)
{
    // 系统配置
    SystemConfig config = {
        .collect_interval = 1000,  // 1秒采集一次数据
        .check_interval = 1000,    // 1秒检查一次报警
        .record_capacity = 100     // 最多保存100条报警记录
    };
    
    // 初始化系统
    if (SystemInit(&config) != 0) {
        printf("System init failed\n");
        return -1;
    }
    
    // 启动系统
    if (SystemStart() != 0) {
        printf("System start failed\n");
        return -1;
    }
    
    // 主循环
    while (1) {
        // 处理系统事件
        ProcessSystemEvents();
        
        // 检查系统错误
        if (SpaceStationGetSystemError() != 0) {
            printf("System error occurred\n");
            break;
        }
        
        // 休眠100ms
        usleep(100000);
    }
    
    // 停止系统
    SystemStop();
    
    // 反初始化系统
    SystemDeinit();
    
    return 0;
}
