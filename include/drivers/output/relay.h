#ifndef DRIVERS_RELAY_H
#define DRIVERS_RELAY_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// 继电器和风扇状态
typedef enum {
    RELAY_STATE_OFF = 0,    // 关闭
    RELAY_STATE_ON = 1      // 开启
} RelayState;

// 风扇速度级别
typedef enum {
    FAN_SPEED_OFF = 0,      // 关闭
    FAN_SPEED_LOW = 1,      // 低速
    FAN_SPEED_MEDIUM = 2,   // 中速
    FAN_SPEED_HIGH = 3      // 高速
} FanSpeed;

// 继电器和风扇错误码
typedef enum {
    RELAY_ERROR_NONE = 0,       // 无错误
    RELAY_ERROR_OVERLOAD = -1,  // 过载错误
    RELAY_ERROR_TIMEOUT = -2,   // 超时错误
    RELAY_ERROR_HARDWARE = -3   // 硬件错误
} RelayError;

// 初始化继电器和风扇
int RelayInit(void);

// 设置继电器状态
int RelaySetState(RelayState state);

// 获取继电器状态
RelayState RelayGetState(void);

// 设置风扇速度
int RelaySetFanSpeed(FanSpeed speed);

// 获取风扇速度
FanSpeed RelayGetFanSpeed(void);

// 获取错误状态
RelayError RelayGetError(void);

// 清除错误状态
int RelayClearError(void);

// 关闭继电器和风扇
int RelayDeinit(void);

#ifdef __cplusplus
}
#endif

#endif // DRIVERS_RELAY_H 