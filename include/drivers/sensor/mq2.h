#ifndef DRIVERS_SENSOR_MQ2_H
#define DRIVERS_SENSOR_MQ2_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// MQ2 ADC通道定义
#define MQ2_ADC_CHANNEL     3    // ADC通道3
#define MQ2_HEAT_GPIO       10   // 加热控制GPIO

// MQ2报警阈值(PPM)
#define MQ2_ALARM_THRESHOLD 100

// 初始化MQ2传感器
int MQ2Init(void);

// 获取MQ2传感器数据
int MQ2GetData(float* smoke);

// 反初始化MQ2传感器
int MQ2Deinit(void);

// 加热控制
int MQ2HeatOn(void);
int MQ2HeatOff(void);

#ifdef __cplusplus
}
#endif

#endif // DRIVERS_SENSOR_MQ2_H 