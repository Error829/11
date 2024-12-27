#ifndef DRIVERS_LED_H
#define DRIVERS_LED_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// LED颜色定义
typedef enum {
    LED_COLOR_OFF = 0,
    LED_COLOR_RED,
    LED_COLOR_GREEN,
    LED_COLOR_BLUE,
    LED_COLOR_MAX
} LEDColor;

// 初始化LED
int LEDInit(void);

// 设置LED颜色
int LEDSetColor(LEDColor color);

// 设置LED闪烁
// enable: 是否启用闪烁
// interval_ms: 闪烁间隔(毫秒)
int LEDSetBlink(bool enable, uint32_t interval_ms);

// 关闭LED
int LEDOff(void);

// 反初始化LED
int LEDDeinit(void);

#ifdef __cplusplus
}
#endif

#endif // DRIVERS_LED_H 