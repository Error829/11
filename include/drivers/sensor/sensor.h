#ifndef SENSOR_H
#define SENSOR_H

#include <stdint.h>

// 传感器类型定义
typedef enum {
    SENSOR_TYPE_TEMP_HUMID = 0,  // 温湿度传感器
    SENSOR_TYPE_SMOKE,           // 烟雾传感器
    SENSOR_TYPE_LIGHT,           // 光照传感器
} sensor_type_t;

// 传感器数据结构
typedef struct {
    float temperature;           // 温度值
    float humidity;             // 湿度值
    float smoke;                // 烟雾值
    float light;                // 光照值
} sensor_data_t;

// 传感器状态
typedef enum {
    SENSOR_OK = 0,              // 正常
    SENSOR_ERROR_TIMEOUT,       // 超时错误
    SENSOR_ERROR_CHECKSUM,      // 校验错误
    SENSOR_ERROR_GPIO,          // GPIO错误
    SENSOR_ERROR_DATA           // 数据错误
} sensor_status_t;

// 传感器操作接口
typedef struct {
    sensor_type_t type;                     // 传感器类型
    sensor_status_t (*init)(void);          // 初始化函数
    sensor_status_t (*read)(sensor_data_t*);// 读取数据函数
    sensor_status_t (*deinit)(void);        // 反初始化函数
} sensor_ops_t;

#endif /* SENSOR_H */
