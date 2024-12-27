#include <stdio.h>
#include <stdlib.h>
#include "drivers/sensor/mq2.h"
#include "iot_gpio.h"
#include "iot_adc.h"
#include "hi_adc.h"

// ADC初始化和反初始化函数声明
extern unsigned int hi_adc_init(void);
extern unsigned int hi_adc_deinit(void);

// 初始化MQ2传感器
int MQ2Init(void)
{
    // 初始化ADC
    if (hi_adc_init() != 0) {
        printf("MQ2 ADC init failed\n");
        return -1;
    }
    
    // 初始化加热控制GPIO
    IoTGpioInit(MQ2_HEAT_GPIO);
    IoTGpioSetDir(MQ2_HEAT_GPIO, IOT_GPIO_DIR_OUT);
    
    // 默认开启加热
    MQ2HeatOn();
    
    return 0;
}

// 获取MQ2传感器数据
int MQ2GetData(float* smoke)
{
    if (smoke == NULL) {
        return -1;
    }
    
    unsigned short data;
    if (IoTAdcRead(MQ2_ADC_CHANNEL, &data, IOT_ADC_EQU_MODEL_8, IOT_ADC_CUR_BAIS_DEFAULT, 0) != 0) {
        printf("MQ2 read ADC failed\n");
        return -1;
    }
    
    // 将ADC值转换为PPM浓度值
    // 这里使用简单的线性转换,实际应用中需要根据传感器特性曲线进行校准
    *smoke = (float)data * 0.1f;
    
    return 0;
}

// 开启加热
int MQ2HeatOn(void)
{
    return IoTGpioSetOutputVal(MQ2_HEAT_GPIO, IOT_GPIO_VALUE1);
}

// 关闭加热
int MQ2HeatOff(void)
{
    return IoTGpioSetOutputVal(MQ2_HEAT_GPIO, IOT_GPIO_VALUE0);
}

// 反初始化MQ2传感器
int MQ2Deinit(void)
{
    // 关闭加热
    MQ2HeatOff();
    
    // 反初始化GPIO
    IoTGpioDeinit(MQ2_HEAT_GPIO);
    
    // 反初始化ADC
    hi_adc_deinit();
    
    return 0;
}
