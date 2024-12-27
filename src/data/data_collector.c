#include "data/data_collector.h"
#include <stdio.h>
#include <string.h>
#include "cmsis_os2.h"
#include "iot_gpio.h"
#include "iot_adc.h"

// 数据缓存结构
typedef struct {
    SensorData* data;     // 数据缓存数组
    uint32_t size;        // 缓存大小
    uint32_t count;       // 当前数据数量
    uint32_t index;       // 当前写入位置
} DataCache;

// 全局变量
static CollectorState g_state = COLLECTOR_STATE_IDLE;
static CollectorError g_error = COLLECTOR_ERROR_NONE;
static CollectorConfig g_config = {0};
static DataCallback g_callback = NULL;
static osTimerId_t g_timer = NULL;
static DataCache g_cache[SENSOR_TYPE_MAX] = {0};
static SensorData g_latest[SENSOR_TYPE_MAX] = {0};

// 更新状态
static void UpdateState(CollectorState state, CollectorError error)
{
    g_state = state;
    g_error = error;
}

// 采集DHT11数据
static int CollectDHT11Data(SensorData* data)
{
    float temperature = 0.0f;
    float humidity = 0.0f;
    
    if (DHT11GetData(&temperature, &humidity) != 0) {
        return -1;
    }
    
    data->type = SENSOR_TYPE_DHT11;
    data->timestamp = osKernelGetTickCount();
    data->data.dht11.temperature = temperature;
    data->data.dht11.humidity = humidity;
    
    return 0;
}

// 采集MQ2数据
static int CollectMQ2Data(SensorData* data)
{
    float smoke = 0.0f;
    
    if (MQ2GetData(&smoke) != 0) {
        return -1;
    }
    
    data->type = SENSOR_TYPE_MQ2;
    data->timestamp = osKernelGetTickCount();
    data->data.mq2.smoke = smoke;
    
    return 0;
}

// 采集BH1750数据
static int CollectBH1750Data(SensorData* data)
{
    float light = 0.0f;
    
    if (BH1750GetData(&light) != 0) {
        return -1;
    }
    
    data->type = SENSOR_TYPE_BH1750;
    data->timestamp = osKernelGetTickCount();
    data->data.bh1750.light = light;
    
    return 0;
}

// 缓存数据
static int CacheData(const SensorData* data)
{
    if (data == NULL || data->type >= SENSOR_TYPE_MAX) {
        return -1;
    }
    
    DataCache* cache = &g_cache[data->type];
    if (cache->data == NULL || cache->size == 0) {
        return -1;
    }
    
    // 写入数据
    memcpy(&cache->data[cache->index], data, sizeof(SensorData));
    
    // 更新索引
    cache->index = (cache->index + 1) % cache->size;
    if (cache->count < cache->size) {
        cache->count++;
    }
    
    // 更新最新数据
    memcpy(&g_latest[data->type], data, sizeof(SensorData));
    
    // 调用回调函数
    if (g_callback != NULL) {
        g_callback(data);
    }
    
    return 0;
}

// 采集数据
static int CollectData(SensorType type)
{
    SensorData data = {0};
    int ret = 0;
    
    switch (type) {
        case SENSOR_TYPE_DHT11:
            ret = CollectDHT11Data(&data);
            break;
            
        case SENSOR_TYPE_MQ2:
            ret = CollectMQ2Data(&data);
            break;
            
        case SENSOR_TYPE_BH1750:
            ret = CollectBH1750Data(&data);
            break;
            
        default:
            return -1;
    }
    
    if (ret == 0) {
        CacheData(&data);
    }
    
    return ret;
}

// 定时器回调函数
static void TimerCallback(void* arg)
{
    (void)arg;
    
    if (g_state != COLLECTOR_STATE_RUNNING) {
        return;
    }
    
    // 采集所有传感器数据
    for (SensorType type = SENSOR_TYPE_DHT11; type < SENSOR_TYPE_MAX; type++) {
        if (CollectData(type) != 0) {
            UpdateState(COLLECTOR_STATE_ERROR, COLLECTOR_ERROR_SENSOR);
            return;
        }
    }
}

// 初始化数据采集模块
int CollectorInit(const CollectorConfig* config)
{
    if (config == NULL || config->collect_interval == 0 || config->cache_size == 0) {
        UpdateState(COLLECTOR_STATE_ERROR, COLLECTOR_ERROR_PARAM);
        return -1;
    }
    
    // 保存配置
    memcpy(&g_config, config, sizeof(CollectorConfig));
    
    // 初始化缓存
    for (SensorType type = SENSOR_TYPE_DHT11; type < SENSOR_TYPE_MAX; type++) {
        g_cache[type].data = malloc(sizeof(SensorData) * config->cache_size);
        if (g_cache[type].data == NULL) {
            UpdateState(COLLECTOR_STATE_ERROR, COLLECTOR_ERROR_MEMORY);
            return -1;
        }
        g_cache[type].size = config->cache_size;
        g_cache[type].count = 0;
        g_cache[type].index = 0;
    }
    
    // 创建定时器
    osTimerAttr_t attr = {0};
    attr.name = "CollectorTimer";
    g_timer = osTimerNew(TimerCallback, osTimerPeriodic, NULL, &attr);
    if (g_timer == NULL) {
        UpdateState(COLLECTOR_STATE_ERROR, COLLECTOR_ERROR_PARAM);
        return -1;
    }
    
    UpdateState(COLLECTOR_STATE_IDLE, COLLECTOR_ERROR_NONE);
    return 0;
}

// 启动数据采集
int CollectorStart(void)
{
    if (g_state == COLLECTOR_STATE_RUNNING) {
        return 0;
    }
    
    if (g_timer == NULL) {
        UpdateState(COLLECTOR_STATE_ERROR, COLLECTOR_ERROR_PARAM);
        return -1;
    }
    
    // 启动定时器
    if (osTimerStart(g_timer, g_config.collect_interval) != osOK) {
        UpdateState(COLLECTOR_STATE_ERROR, COLLECTOR_ERROR_PARAM);
        return -1;
    }
    
    UpdateState(COLLECTOR_STATE_RUNNING, COLLECTOR_ERROR_NONE);
    return 0;
}

// 停止数据采集
int CollectorStop(void)
{
    if (g_timer == NULL) {
        UpdateState(COLLECTOR_STATE_ERROR, COLLECTOR_ERROR_PARAM);
        return -1;
    }
    
    // 停止定时器
    if (osTimerStop(g_timer) != osOK) {
        UpdateState(COLLECTOR_STATE_ERROR, COLLECTOR_ERROR_PARAM);
        return -1;
    }
    
    UpdateState(COLLECTOR_STATE_IDLE, COLLECTOR_ERROR_NONE);
    return 0;
}

// 获取采集器状态
CollectorState CollectorGetState(void)
{
    return g_state;
}

// 获取采集器错误码
CollectorError CollectorGetError(void)
{
    return g_error;
}

// 手动触发数据采集
int CollectorTrigger(SensorType type)
{
    if (type >= SENSOR_TYPE_MAX) {
        UpdateState(COLLECTOR_STATE_ERROR, COLLECTOR_ERROR_PARAM);
        return -1;
    }
    
    if (type == SENSOR_TYPE_ALL) {
        // 采集所有传感器数据
        for (SensorType t = SENSOR_TYPE_DHT11; t < SENSOR_TYPE_MAX; t++) {
            if (CollectData(t) != 0) {
                return -1;
            }
        }
    } else {
        // 采集指定传感器数据
        if (CollectData(type) != 0) {
            return -1;
        }
    }
    
    return 0;
}

// 获取最新的传感器数据
int CollectorGetLatestData(SensorType type, SensorData* data)
{
    if (type >= SENSOR_TYPE_MAX || data == NULL) {
        return -1;
    }
    
    memcpy(data, &g_latest[type], sizeof(SensorData));
    return 0;
}

// 注册数据回调函数
int CollectorRegisterCallback(DataCallback callback)
{
    g_callback = callback;
    return 0;
}

// 反初始化数据采集模块
int CollectorDeinit(void)
{
    // 停止采集
    CollectorStop();
    
    // 删除定时器
    if (g_timer != NULL) {
        osTimerDelete(g_timer);
        g_timer = NULL;
    }
    
    // 释放缓存
    for (SensorType type = SENSOR_TYPE_DHT11; type < SENSOR_TYPE_MAX; type++) {
        if (g_cache[type].data != NULL) {
            free(g_cache[type].data);
            g_cache[type].data = NULL;
        }
    }
    
    UpdateState(COLLECTOR_STATE_IDLE, COLLECTOR_ERROR_NONE);
    return 0;
} 