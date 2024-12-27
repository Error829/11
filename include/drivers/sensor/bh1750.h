#ifndef DRIVERS_SENSOR_BH1750_H
#define DRIVERS_SENSOR_BH1750_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// BH1750 I2C地址(ADDR引脚接地时)
#define BH1750_I2C_ADDR 0x23

// BH1750指令集
#define BH1750_POWER_DOWN    0x00  // 断电模式
#define BH1750_POWER_ON      0x01  // 开启模式
#define BH1750_RESET         0x07  // 重置数据寄存器
#define BH1750_CONT_H_MODE   0x10  // 连续高分辨率模式:1lx
#define BH1750_CONT_H_MODE2  0x11  // 连续高分辨率模式2:0.5lx
#define BH1750_CONT_L_MODE   0x13  // 连续低分辨率模式:4lx
#define BH1750_ONE_H_MODE    0x20  // 单次高分辨率模式
#define BH1750_ONE_H_MODE2   0x21  // 单次高分辨率模式2
#define BH1750_ONE_L_MODE    0x23  // 单次低分辨率模式

// 初始化BH1750传感器
int BH1750Init(void);

// 获取BH1750传感器数据
int BH1750GetData(float* light);

// 反初始化BH1750传感器
int BH1750Deinit(void);

#ifdef __cplusplus
}
#endif

#endif // DRIVERS_SENSOR_BH1750_H 