#ifndef DRIVERS_SENSOR_DHT11_H
#define DRIVERS_SENSOR_DHT11_H

#ifdef __cplusplus
extern "C" {
#endif

// 初始化DHT11传感器
int DHT11Init(void);

// 获取DHT11传感器数据
int DHT11GetData(float* temperature, float* humidity);

// 反初始化DHT11传感器
int DHT11Deinit(void);

#ifdef __cplusplus
}
#endif

#endif // DRIVERS_SENSOR_DHT11_H 