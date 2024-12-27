#include <stdio.h>
#include <unistd.h>
#include "drivers/sensor/bh1750.h"
#include "iot_i2c.h"

#define BH1750_I2C_IDX     0    // I2C设备索引
#define BH1750_I2C_BAUDRATE 100000  // 100KHz

// 写命令
static int bh1750_write_cmd(uint8_t cmd)
{
    uint32_t ret = IoTI2cWrite(BH1750_I2C_IDX, BH1750_I2C_ADDR, &cmd, 1);
    return (ret == 0) ? 0 : -1;
}

// 读取数据
static int bh1750_read_data(uint8_t* data, uint32_t data_len)
{
    uint32_t ret = IoTI2cRead(BH1750_I2C_IDX, BH1750_I2C_ADDR, data, data_len);
    return (ret == 0) ? 0 : -1;
}

// 初始化BH1750传感器
int BH1750Init(void)
{
    // 初始化I2C
    IoTI2cInit(BH1750_I2C_IDX, BH1750_I2C_BAUDRATE);
    
    // 开启传感器
    if (bh1750_write_cmd(BH1750_POWER_ON) != 0) {
        printf("BH1750 power on failed\n");
        return -1;
    }
    
    // 重置数据寄存器
    if (bh1750_write_cmd(BH1750_RESET) != 0) {
        printf("BH1750 reset failed\n");
        return -1;
    }
    
    // 设置为连续高分辨率模式
    if (bh1750_write_cmd(BH1750_CONT_H_MODE) != 0) {
        printf("BH1750 set mode failed\n");
        return -1;
    }
    
    return 0;
}

// 获取BH1750传感器数据
int BH1750GetData(float* light)
{
    if (light == NULL) {
        return -1;
    }
    
    uint8_t data[2];
    if (bh1750_read_data(data, 2) != 0) {
        printf("BH1750 read data failed\n");
        return -1;
    }
    
    // 计算光照强度(单位:lx)
    uint16_t value = (data[0] << 8) | data[1];
    *light = (float)value / 1.2f;
    
    return 0;
}

// 反初始化BH1750传感器
int BH1750Deinit(void)
{
    // 关闭传感器
    if (bh1750_write_cmd(BH1750_POWER_DOWN) != 0) {
        printf("BH1750 power down failed\n");
        return -1;
    }
    
    // 反初始化I2C
    IoTI2cDeinit(BH1750_I2C_IDX);
    
    return 0;
}
