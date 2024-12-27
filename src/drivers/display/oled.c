#include "drivers/display/oled.h"
#include "iot_gpio.h"
#include "iot_i2c.h"
#include "hi_io.h"

// OLED I2C地址
#define OLED_I2C_ADDR 0x78

// OLED控制命令
#define OLED_CMD_MODE 0x00
#define OLED_DATA_MODE 0x40

// OLED初始化命令序列
static const uint8_t OLED_INIT_CMD[] = {
    0xAE, // 关闭显示
    0xD5, // 设置显示时钟分频比/振荡器频率
    0x80,
    0xA8, // 设置多路复用率
    0x3F,
    0xD3, // 设置显示偏移
    0x00,
    0x40, // 设置显示开始行
    0x8D, // 充电泵设置
    0x14,
    0x20, // 设置内存地址模式
    0x02,
    0xA1, // 段重定义
    0xC8, // COM扫描方向
    0xDA, // COM硬件配置
    0x12,
    0x81, // 对比度设置
    0xCF,
    0xD9, // 预充电周期
    0xF1,
    0xDB, // VCOMH取消选择级别
    0x30,
    0xA4, // 全局显示开启
    0xA6, // 设置正常显示
    0xAF  // 开启显示
};

// ASCII字体库 6x8点阵
static const uint8_t ASCII_FONT_6X8[][6] = {
    {0x00,0x00,0x00,0x00,0x00,0x00}, // 空格
    // ... 其他字符的点阵数据
};

// I2C写命令
static int32_t oled_write_cmd(uint8_t cmd)
{
    uint8_t data[2] = {OLED_CMD_MODE, cmd};
    return IoTI2cWrite(0, OLED_I2C_ADDR, data, 2);
}

// I2C写数据
static int32_t oled_write_data(uint8_t data)
{
    uint8_t buf[2] = {OLED_DATA_MODE, data};
    return IoTI2cWrite(0, OLED_I2C_ADDR, buf, 2);
}

// 设置光标位置
static int32_t oled_set_cursor(uint8_t x, uint8_t y)
{
    oled_write_cmd(0xB0 + y);
    oled_write_cmd(((x & 0xF0) >> 4) | 0x10);
    oled_write_cmd(x & 0x0F);
    return 0;
}

// 初始化OLED
static int32_t oled_init(void)
{
    // 配置I2C引脚
    IoTGpioInit(0);
    IoTGpioInit(1);
    hi_io_set_func(0, 6);  // I2C_SDA
    hi_io_set_func(1, 6);  // I2C_SCL
    
    // 初始化I2C
    IoTI2cInit(0, 400000); // 400KHz
    
    // 发送初始化命令序列
    for (uint32_t i = 0; i < sizeof(OLED_INIT_CMD); i++) {
        if (oled_write_cmd(OLED_INIT_CMD[i]) != 0) {
            return -1;
        }
    }
    
    return 0;
}

// 开启显示
static int32_t oled_display_on(void)
{
    return oled_write_cmd(0xAF);
}

// 关闭显示
static int32_t oled_display_off(void)
{
    return oled_write_cmd(0xAE);
}

// 清屏
static int32_t oled_clear(void)
{
    for (uint8_t i = 0; i < OLED_PAGE_NUM; i++) {
        oled_set_cursor(0, i);
        for (uint8_t j = 0; j < OLED_WIDTH; j++) {
            oled_write_data(0x00);
        }
    }
    return 0;
}

// 显示一个字符
static int32_t oled_show_char(uint8_t x, uint8_t y, char chr)
{
    if (x > OLED_WIDTH - 6 || y > OLED_PAGE_NUM - 1) {
        return -1;
    }
    
    oled_set_cursor(x, y);
    
    for (uint8_t i = 0; i < 6; i++) {
        oled_write_data(ASCII_FONT_6X8[chr - ' '][i]);
    }
    
    return 0;
}

// 显示字符串
static int32_t oled_show_string(uint8_t x, uint8_t y, const char* str)
{
    while (*str != '\0') {
        if (oled_show_char(x, y, *str) != 0) {
            return -1;
        }
        x += 6;
        if (x > OLED_WIDTH - 6) {
            x = 0;
            y++;
            if (y > OLED_PAGE_NUM - 1) {
                break;
            }
        }
        str++;
    }
    return 0;
}

// 显示数字
static int32_t oled_show_num(uint8_t x, uint8_t y, uint32_t num)
{
    char str[11];
    sprintf(str, "%u", num);
    return oled_show_string(x, y, str);
}

// 反初始化
static int32_t oled_deinit(void)
{
    oled_display_off();
    IoTI2cDeinit(0);
    IoTGpioDeinit(0);
    IoTGpioDeinit(1);
    return 0;
}

// OLED操作接口实例
static const oled_ops g_oled_ops = {
    .init = oled_init,
    .display_on = oled_display_on,
    .display_off = oled_display_off,
    .clear = oled_clear,
    .show_char = oled_show_char,
    .show_string = oled_show_string,
    .show_num = oled_show_num,
    .deinit = oled_deinit,
};

// 获取OLED操作接口
const oled_ops* get_oled_ops(void)
{
    return &g_oled_ops;
} 