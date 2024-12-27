#include <stdio.h>
#include <unistd.h>
#include "iot_gpio.h"
#include "hi_pwm.h"
#include "iot_errno.h"
#include "hi_io.h"
#include "hi_gpio.h"
#include "drivers/output/buzzer.h"

// PWM端口定义
#define BUZZER_PWM_PORT    HI_PWM_PORT_PWM0
#define BUZZER_PWM_PIN     9    // GPIO9
#define BUZZER_GPIO_FUNC   HI_IO_FUNC_GPIO_9_PWM0_OUT    // GPIO9的PWM功能

// 音效参数定义
#define DEFAULT_FREQUENCY   2000  // 默认频率2000Hz
#define BEEP_DURATION      200   // 基本音效持续时间(ms)
#define BEEP_INTERVAL      100   // 音效间隔时间(ms)

// 全局变量
static uint32_t g_current_frequency = DEFAULT_FREQUENCY;
static uint8_t g_is_playing = 0;

// 初始化蜂鸣器
int BuzzerInit(void)
{
    // 配置GPIO引脚复用为PWM功能
    int ret = hi_io_set_func(BUZZER_PWM_PIN, BUZZER_GPIO_FUNC);
    if (ret != IOT_SUCCESS) {
        return ret;
    }
    
    // 初始化PWM
    ret = hi_pwm_init(BUZZER_PWM_PORT);
    if (ret != IOT_SUCCESS) {
        return ret;
    }
    
    return IOT_SUCCESS;
}

// 设置蜂鸣器频率
int BuzzerSetFrequency(uint32_t frequency)
{
    if (frequency == 0) {
        return IOT_FAILURE;
    }
    
    g_current_frequency = frequency;
    return IOT_SUCCESS;
}

// 开启蜂鸣器
int BuzzerOn(void)
{
    // 计算PWM参数
    uint32_t duty = 50;  // 占空比50%
    uint32_t period = 1000000 / g_current_frequency;  // 周期(us)
    
    // 启动PWM输出
    int ret = hi_pwm_start(BUZZER_PWM_PORT, duty, period);
    if (ret != IOT_SUCCESS) {
        return ret;
    }
    
    return IOT_SUCCESS;
}

// 关闭蜂鸣器
int BuzzerOff(void)
{
    int ret = hi_pwm_stop(BUZZER_PWM_PORT);
    if (ret != IOT_SUCCESS) {
        return ret;
    }
    
    return IOT_SUCCESS;
}

// 延时指定毫秒数
static void DelayMS(uint32_t ms)
{
    usleep(ms * 1000);
}

// 播放单次蜂鸣
static void PlaySingleBeep(void)
{
    BuzzerOn();
    DelayMS(BEEP_DURATION);
    BuzzerOff();
}

// 播放双蜂鸣
static void PlayDoubleBeep(void)
{
    PlaySingleBeep();
    DelayMS(BEEP_INTERVAL);
    PlaySingleBeep();
}

// 播放SOS音效
static void PlaySOS(void)
{
    // 三短音
    for (int i = 0; i < 3; i++) {
        BuzzerOn();
        DelayMS(200);
        BuzzerOff();
        DelayMS(200);
    }
    DelayMS(400);
    
    // 三长音
    for (int i = 0; i < 3; i++) {
        BuzzerOn();
        DelayMS(600);
        BuzzerOff();
        DelayMS(200);
    }
    DelayMS(400);
    
    // 三短音
    for (int i = 0; i < 3; i++) {
        BuzzerOn();
        DelayMS(200);
        BuzzerOff();
        DelayMS(200);
    }
}

// 播放警报音
static void PlayAlert(void)
{
    for (int i = 0; i < 3; i++) {
        BuzzerSetFrequency(2000);
        BuzzerOn();
        DelayMS(300);
        BuzzerSetFrequency(1500);
        DelayMS(300);
    }
    BuzzerOff();
}

// 播放指定模式的音效
int BuzzerPlayMode(BuzzerMode mode)
{
    if (g_is_playing) {
        return IOT_FAILURE;
    }
    
    g_is_playing = 1;
    
    switch (mode) {
        case BUZZER_MODE_SINGLE_BEEP:
            PlaySingleBeep();
            break;
        case BUZZER_MODE_DOUBLE_BEEP:
            PlayDoubleBeep();
            break;
        case BUZZER_MODE_CONTINUOUS:
            BuzzerOn();
            break;
        case BUZZER_MODE_SOS:
            PlaySOS();
            break;
        case BUZZER_MODE_ALERT:
            PlayAlert();
            break;
        default:
            g_is_playing = 0;
            return IOT_FAILURE;
    }
    
    if (mode != BUZZER_MODE_CONTINUOUS) {
        g_is_playing = 0;
    }
    
    return IOT_SUCCESS;
}

// 停止当前音效
int BuzzerStop(void)
{
    g_is_playing = 0;
    return BuzzerOff();
}
