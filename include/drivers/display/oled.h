#ifndef DRIVERS_DISPLAY_OLED_H
#define DRIVERS_DISPLAY_OLED_H

#include <stdint.h>

// OLED显示屏参数
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_PAGE_NUM 8  // 每页8行,总共8页

// OLED操作接口结构体
typedef struct {
    // 初始化函数
    int32_t (*init)(void);
    // 显示控制
    int32_t (*display_on)(void);
    int32_t (*display_off)(void);
    int32_t (*clear)(void);
    // 显示功能
    int32_t (*show_char)(uint8_t x, uint8_t y, char chr);
    int32_t (*show_string)(uint8_t x, uint8_t y, const char* str);
    int32_t (*show_num)(uint8_t x, uint8_t y, uint32_t num);
    // 反初始化
    int32_t (*deinit)(void);
} oled_ops;

// 获取OLED操作接口
const oled_ops* get_oled_ops(void);

#endif /* DRIVERS_DISPLAY_OLED_H */ 