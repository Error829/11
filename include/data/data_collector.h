#ifndef DATA_COLLECTOR_H
#define DATA_COLLECTOR_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// 采集器状态定义
typedef enum {
    COLLECTOR_STATE_IDLE = 0,  // 空闲状态
    COLLECTOR_STATE_RUNNING,   // 运行状态
    COLLECTOR_STATE_ERROR,     // 错误状态
    COLLECTOR_STATE_MAX
} CollectorState;

// 采集器错误码定义
typedef enum {
    COLLECTOR_ERROR_NONE = 0,  // 无错误
    COLLECTOR_ERROR_PARAM,     // 参数错误
    COLLECTOR_ERROR_MEMORY,    // 内存错误
    COLLECTOR_ERROR_SENSOR,    // 传感器错误
    COLLECTOR_ERROR_MAX
} CollectorError;

// 传感器类型定义
typedef enum {
    SENSOR_TYPE_DHT11 = 0,  // DHT11温湿度传感器
    SENSOR_TYPE_MQ2,        // MQ2烟雾传感器
    SENSOR_TYPE_BH1750,     // BH1750光照传感器
    SENSOR_TYPE_MAX,
    SENSOR_TYPE_ALL = 0xFF  // 所有传感器
} SensorType;

// DHT11传感器数据
typedef struct {
    float temperature;  // 温度值(℃)
    float humidity;     // 湿度值(%)
} DHT11Data;

// MQ2传感器数据
typedef struct {
    float smoke;  // 烟雾浓度
} MQ2Data;

// BH1750传感器数据
typedef struct {
    float light;  // 光照强度(lux)
} BH1750Data;

// 传感器数据联合体
typedef union {
    DHT11Data dht11;    // DHT11数据
    MQ2Data mq2;        // MQ2数据
    BH1750Data bh1750;  // BH1750数据
} SensorDataUnion;

// 传感器数据结构
typedef struct {
    SensorType type;        // 传感器类型
    SensorDataUnion data;   // 传感器数据
    uint32_t timestamp;     // 数据采集时间戳
} SensorData;

// 采集器配置结构
typedef struct {
    uint32_t collect_interval;  // 采集间隔(ms)
    uint32_t cache_size;        // 缓存大小
} CollectorConfig;

// 数据回调函数类型
typedef void (*DataCallback)(const SensorData* data);

// 初始化数据采集模块
int CollectorInit(const CollectorConfig* config);

// 启动数据采集
int CollectorStart(void);

// 停止数据采集
int CollectorStop(void);

// 获取采集器状态
CollectorState CollectorGetState(void);

// 获取采集器错误码
CollectorError CollectorGetError(void);

// 手动触发数据采集
int CollectorTrigger(SensorType type);

// 获取最新的传感器数据
int CollectorGetLatestData(SensorType type, SensorData* data);

// 注册数据回调函数
int CollectorRegisterCallback(DataCallback callback);

// 反初始化数据采集模块
int CollectorDeinit(void);

#ifdef __cplusplus
}
#endif

#endif // DATA_COLLECTOR_H 