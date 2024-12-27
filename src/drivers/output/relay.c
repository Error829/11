#include <stdio.h>
#include <unistd.h>
#include "iot_gpio.h"
#include "iot_errno.h"
#include "drivers/output/relay.h"

// GPIO引脚定义
#define RELAY_GPIO_PIN      10    // 继电器控制引脚
#define FAN_SPEED_PIN_1     11    // 风扇速度控制引脚1
#define FAN_SPEED_PIN_2     12    // 风扇速度控制引脚2

// 保护参数定义
#define RELAY_PROTECT_TIMEOUT    5000   // 继电器保护超时时间(ms)
#define RELAY_CHECK_INTERVAL     100    // 状态检查间隔(ms)

// 全局变量
static RelayState g_relay_state = RELAY_STATE_OFF;
static FanSpeed g_fan_speed = FAN_SPEED_OFF;
static RelayError g_relay_error = RELAY_ERROR_NONE;
static uint32_t g_relay_on_time = 0;    // 继电器开启时间

// 初始化GPIO
static int InitGpio(void)
{
    // 初始化继电器控制引脚
    int ret = IoTGpioInit(RELAY_GPIO_PIN);
    if (ret != IOT_SUCCESS) {
        return ret;
    }
    ret = IoTGpioSetDir(RELAY_GPIO_PIN, IOT_GPIO_DIR_OUT);
    if (ret != IOT_SUCCESS) {
        return ret;
    }

    // 初始化风扇控制引脚
    ret = IoTGpioInit(FAN_SPEED_PIN_1);
    if (ret != IOT_SUCCESS) {
        return ret;
    }
    ret = IoTGpioSetDir(FAN_SPEED_PIN_1, IOT_GPIO_DIR_OUT);
    if (ret != IOT_SUCCESS) {
        return ret;
    }

    ret = IoTGpioInit(FAN_SPEED_PIN_2);
    if (ret != IOT_SUCCESS) {
        return ret;
    }
    ret = IoTGpioSetDir(FAN_SPEED_PIN_2, IOT_GPIO_DIR_OUT);
    if (ret != IOT_SUCCESS) {
        return ret;
    }

    return IOT_SUCCESS;
}

// 设置风扇GPIO状态
static int SetFanGpio(FanSpeed speed)
{
    int ret;
    switch (speed) {
        case FAN_SPEED_OFF:
            ret = IoTGpioSetOutputVal(FAN_SPEED_PIN_1, 0);
            if (ret != IOT_SUCCESS) return ret;
            ret = IoTGpioSetOutputVal(FAN_SPEED_PIN_2, 0);
            break;
        case FAN_SPEED_LOW:
            ret = IoTGpioSetOutputVal(FAN_SPEED_PIN_1, 1);
            if (ret != IOT_SUCCESS) return ret;
            ret = IoTGpioSetOutputVal(FAN_SPEED_PIN_2, 0);
            break;
        case FAN_SPEED_MEDIUM:
            ret = IoTGpioSetOutputVal(FAN_SPEED_PIN_1, 0);
            if (ret != IOT_SUCCESS) return ret;
            ret = IoTGpioSetOutputVal(FAN_SPEED_PIN_2, 1);
            break;
        case FAN_SPEED_HIGH:
            ret = IoTGpioSetOutputVal(FAN_SPEED_PIN_1, 1);
            if (ret != IOT_SUCCESS) return ret;
            ret = IoTGpioSetOutputVal(FAN_SPEED_PIN_2, 1);
            break;
        default:
            return IOT_FAILURE;
    }
    return ret;
}

// 检查继电器状态
static void CheckRelayStatus(void)
{
    if (g_relay_state == RELAY_STATE_ON) {
        uint32_t current_time = (uint32_t)time(NULL) * 1000;  // 转换为毫秒
        if (current_time - g_relay_on_time > RELAY_PROTECT_TIMEOUT) {
            // 超时保护
            RelaySetState(RELAY_STATE_OFF);
            g_relay_error = RELAY_ERROR_TIMEOUT;
        }
    }
}

// 初始化继电器和风扇
int RelayInit(void)
{
    int ret = InitGpio();
    if (ret != IOT_SUCCESS) {
        g_relay_error = RELAY_ERROR_HARDWARE;
        return ret;
    }

    // 初始状态设置
    g_relay_state = RELAY_STATE_OFF;
    g_fan_speed = FAN_SPEED_OFF;
    g_relay_error = RELAY_ERROR_NONE;

    // 设置初始输出状态
    ret = IoTGpioSetOutputVal(RELAY_GPIO_PIN, 0);
    if (ret != IOT_SUCCESS) {
        g_relay_error = RELAY_ERROR_HARDWARE;
        return ret;
    }

    ret = SetFanGpio(FAN_SPEED_OFF);
    if (ret != IOT_SUCCESS) {
        g_relay_error = RELAY_ERROR_HARDWARE;
        return ret;
    }

    return IOT_SUCCESS;
}

// 设置继电器状态
int RelaySetState(RelayState state)
{
    if (g_relay_error != RELAY_ERROR_NONE && state == RELAY_STATE_ON) {
        return IOT_FAILURE;
    }

    int ret = IoTGpioSetOutputVal(RELAY_GPIO_PIN, state);
    if (ret != IOT_SUCCESS) {
        g_relay_error = RELAY_ERROR_HARDWARE;
        return ret;
    }

    g_relay_state = state;
    if (state == RELAY_STATE_ON) {
        g_relay_on_time = (uint32_t)time(NULL) * 1000;  // 记录开启时间
    }

    return IOT_SUCCESS;
}

// 获取继电器状态
RelayState RelayGetState(void)
{
    CheckRelayStatus();  // 检查状态
    return g_relay_state;
}

// 设置风扇速度
int RelaySetFanSpeed(FanSpeed speed)
{
    if (g_relay_error != RELAY_ERROR_NONE) {
        return IOT_FAILURE;
    }

    int ret = SetFanGpio(speed);
    if (ret != IOT_SUCCESS) {
        g_relay_error = RELAY_ERROR_HARDWARE;
        return ret;
    }

    g_fan_speed = speed;
    return IOT_SUCCESS;
}

// 获取风扇速度
FanSpeed RelayGetFanSpeed(void)
{
    return g_fan_speed;
}

// 获取错误状态
RelayError RelayGetError(void)
{
    return g_relay_error;
}

// 清除错误状态
int RelayClearError(void)
{
    if (g_relay_error == RELAY_ERROR_HARDWARE) {
        return IOT_FAILURE;  // 硬件错误不能通过软件清除
    }
    g_relay_error = RELAY_ERROR_NONE;
    return IOT_SUCCESS;
}

// 关闭继电器和风扇
int RelayDeinit(void)
{
    // 关闭所有输出
    RelaySetState(RELAY_STATE_OFF);
    RelaySetFanSpeed(FAN_SPEED_OFF);

    // 释放GPIO资源
    int ret = IoTGpioDeinit(RELAY_GPIO_PIN);
    if (ret != IOT_SUCCESS) return ret;

    ret = IoTGpioDeinit(FAN_SPEED_PIN_1);
    if (ret != IOT_SUCCESS) return ret;

    ret = IoTGpioDeinit(FAN_SPEED_PIN_2);
    if (ret != IOT_SUCCESS) return ret;

    return IOT_SUCCESS;
}
