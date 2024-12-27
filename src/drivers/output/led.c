#include "drivers/output/led.h"
#include "iot_gpio.h"
#include "ohos_init.h"
#include "cmsis_os2.h"

// LED GPIO引脚定义
#define LED_PIN_RED     10  // GPIO10用于红色LED
#define LED_PIN_GREEN   11  // GPIO11用于绿色LED
#define LED_PIN_BLUE    12  // GPIO12用于蓝色LED

// LED控制相关参数
static struct {
    uint8_t gpio_pin;       // GPIO引脚号
    bool is_blinking;       // 是否正在闪烁
    uint32_t interval_ms;   // 闪烁间隔
    osThreadId_t thread;    // 闪烁控制线程
} g_led_ctrl[LED_COLOR_MAX] = {
    [LED_COLOR_RED]   = {LED_PIN_RED,   false, 0, NULL},
    [LED_COLOR_GREEN] = {LED_PIN_GREEN, false, 0, NULL},
    [LED_COLOR_BLUE]  = {LED_PIN_BLUE,  false, 0, NULL}
};

// 当前LED颜色
static LEDColor g_current_color = LED_COLOR_OFF;

// LED闪烁控制线程
static void LedBlinkThread(void* arg)
{
    LEDColor color = (LEDColor)(uintptr_t)arg;
    
    while (g_led_ctrl[color].is_blinking) {
        // 打开LED
        IoTGpioSetOutputVal(g_led_ctrl[color].gpio_pin, 1);
        osDelay(g_led_ctrl[color].interval_ms);
        
        // 关闭LED
        IoTGpioSetOutputVal(g_led_ctrl[color].gpio_pin, 0);
        osDelay(g_led_ctrl[color].interval_ms);
    }
    
    g_led_ctrl[color].thread = NULL;
    osThreadExit();
}

// 初始化LED
int LEDInit(void)
{
    // 初始化GPIO
    for (int i = 0; i < LED_COLOR_MAX; i++) {
        IoTGpioInit(g_led_ctrl[i].gpio_pin);
        IoTGpioSetDir(g_led_ctrl[i].gpio_pin, IOT_GPIO_DIR_OUT);
        IoTGpioSetOutputVal(g_led_ctrl[i].gpio_pin, 0);
    }
    
    g_current_color = LED_COLOR_OFF;
    return 0;
}

// 设置LED颜色
int LEDSetColor(LEDColor color)
{
    if (color >= LED_COLOR_MAX) {
        return -1;
    }
    
    // 关闭当前LED
    if (g_current_color != LED_COLOR_OFF) {
        LEDSetBlink(false, 0);
        IoTGpioSetOutputVal(g_led_ctrl[g_current_color].gpio_pin, 0);
    }
    
    // 设置新的颜色
    g_current_color = color;
    if (color != LED_COLOR_OFF) {
        IoTGpioSetOutputVal(g_led_ctrl[color].gpio_pin, 1);
    }
    
    return 0;
}

// 设置LED闪烁
int LEDSetBlink(bool enable, uint32_t interval_ms)
{
    if (g_current_color == LED_COLOR_OFF) {
        return -1;
    }
    
    // 停止当前闪烁
    if (g_led_ctrl[g_current_color].is_blinking) {
        g_led_ctrl[g_current_color].is_blinking = false;
        if (g_led_ctrl[g_current_color].thread != NULL) {
            osThreadTerminate(g_led_ctrl[g_current_color].thread);
            g_led_ctrl[g_current_color].thread = NULL;
        }
    }
    
    // 设置新的闪烁状态
    if (enable) {
        g_led_ctrl[g_current_color].interval_ms = interval_ms;
        g_led_ctrl[g_current_color].is_blinking = true;
        
        osThreadAttr_t attr = {0};
        attr.name = "LedBlink";
        attr.stack_size = 1024;
        attr.priority = osPriorityNormal;
        
        g_led_ctrl[g_current_color].thread = osThreadNew(LedBlinkThread, 
            (void*)(uintptr_t)g_current_color, &attr);
        
        if (g_led_ctrl[g_current_color].thread == NULL) {
            g_led_ctrl[g_current_color].is_blinking = false;
            return -1;
        }
    }
    
    return 0;
}

// 关闭LED
int LEDOff(void)
{
    return LEDSetColor(LED_COLOR_OFF);
}

// 反初始化LED
int LEDDeinit(void)
{
    // 停止所有LED
    LEDOff();
    
    // 反初始化GPIO
    for (int i = 0; i < LED_COLOR_MAX; i++) {
        IoTGpioDeinit(g_led_ctrl[i].gpio_pin);
    }
    
    return 0;
}
