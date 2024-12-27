#include <stdio.h>
#include <unistd.h>
#include "drivers/sensor/sensor.h"
#include "hi_gpio.h"
#include "hi_time.h"

// DHT11 GPIO引脚定义
#define DHT11_GPIO_PIN          11    // GPIO11用于DHT11
#define DHT11_TIMEOUT_MS        1000  // 超时时间1秒

// DHT11内部延时参数（微秒）
#define DHT11_START_SIGNAL_MS   20    // 起始信号持续20ms
#define DHT11_RESPONSE_US       40    // 响应信号40us
#define DHT11_DATA_BIT_US       50    // 数据位信号50us

// 私有函数声明
static sensor_status_t dht11_reset(void);
static sensor_status_t dht11_read_byte(uint8_t* data);
static uint8_t dht11_check_sum(uint8_t* data, int len);

// GPIO操作封装
static sensor_status_t gpio_init(void)
{
    // 配置GPIO为输出模式
    if (hi_gpio_init() != HI_ERR_SUCCESS) {
        return SENSOR_ERROR_GPIO;
    }
    if (hi_gpio_set_dir(DHT11_GPIO_PIN, HI_GPIO_DIR_OUT) != HI_ERR_SUCCESS) {
        return SENSOR_ERROR_GPIO;
    }
    return SENSOR_OK;
}

static void gpio_set_output(uint8_t level)
{
    hi_gpio_set_dir(DHT11_GPIO_PIN, HI_GPIO_DIR_OUT);
    hi_gpio_set_ouput_val(DHT11_GPIO_PIN, level);
}

static uint8_t gpio_read_input(void)
{
    hi_gpio_value val;
    hi_gpio_set_dir(DHT11_GPIO_PIN, HI_GPIO_DIR_IN);
    hi_gpio_get_input_val(DHT11_GPIO_PIN, &val);
    return (uint8_t)val;
}

// DHT11初始化
static sensor_status_t dht11_init(void)
{
    sensor_status_t status = gpio_init();
    if (status != SENSOR_OK) {
        return status;
    }
    
    // 初始化完成后拉高数据线
    gpio_set_output(1);
    usleep(1000);  // 等待1ms稳定
    return SENSOR_OK;
}

// DHT11数据读取
static sensor_status_t dht11_read(sensor_data_t* data)
{
    uint8_t raw_data[5] = {0};  // DHT11原始数据
    sensor_status_t status;
    
    if (data == NULL) {
        return SENSOR_ERROR_DATA;
    }

    // 发送起始信号
    status = dht11_reset();
    if (status != SENSOR_OK) {
        return status;
    }

    // 读取5字节数据
    for (int i = 0; i < 5; i++) {
        status = dht11_read_byte(&raw_data[i]);
        if (status != SENSOR_OK) {
            return status;
        }
    }

    // 校验数据
    if (!dht11_check_sum(raw_data, 5)) {
        return SENSOR_ERROR_CHECKSUM;
    }

    // 解析数据
    data->humidity = (float)raw_data[0] + raw_data[1] * 0.1f;
    data->temperature = (float)raw_data[2] + raw_data[3] * 0.1f;
    
    return SENSOR_OK;
}

// DHT11反初��化
static sensor_status_t dht11_deinit(void)
{
    hi_gpio_deinit();
    return SENSOR_OK;
}

// DHT11复位和响应检测
static sensor_status_t dht11_reset(void)
{
    uint32_t timeout = 0;
    
    // 发送起始信号
    gpio_set_output(0);
    usleep(DHT11_START_SIGNAL_MS * 1000);
    gpio_set_output(1);
    usleep(30);  // 等待30us
    
    // 等待DHT11响应
    while (gpio_read_input() == 1) {
        if (++timeout > DHT11_TIMEOUT_MS) {
            return SENSOR_ERROR_TIMEOUT;
        }
        usleep(1);
    }
    
    timeout = 0;
    while (gpio_read_input() == 0) {
        if (++timeout > DHT11_TIMEOUT_MS) {
            return SENSOR_ERROR_TIMEOUT;
        }
        usleep(1);
    }
    
    timeout = 0;
    while (gpio_read_input() == 1) {
        if (++timeout > DHT11_TIMEOUT_MS) {
            return SENSOR_ERROR_TIMEOUT;
        }
        usleep(1);
    }
    
    return SENSOR_OK;
}

// 读取一个字节数据
static sensor_status_t dht11_read_byte(uint8_t* data)
{
    uint8_t bit;
    uint8_t byte = 0;
    uint32_t timeout;
    
    for (int i = 0; i < 8; i++) {
        // 等待50us低电平开始
        timeout = 0;
        while (gpio_read_input() == 0) {
            if (++timeout > DHT11_TIMEOUT_MS) {
                return SENSOR_ERROR_TIMEOUT;
            }
            usleep(1);
        }
        
        // 延时40us
        usleep(40);
        
        // 读取数据位
        bit = gpio_read_input();
        
        // 等待高电平结束
        timeout = 0;
        while (gpio_read_input() == 1) {
            if (++timeout > DHT11_TIMEOUT_MS) {
                return SENSOR_ERROR_TIMEOUT;
            }
            usleep(1);
        }
        
        byte <<= 1;
        if (bit) {
            byte |= 1;
        }
    }
    
    *data = byte;
    return SENSOR_OK;
}

// 校验和检查
static uint8_t dht11_check_sum(uint8_t* data, int len)
{
    if (len != 5) return 0;
    
    return (data[0] + data[1] + data[2] + data[3]) == data[4];
}

// 导出DHT11操作接口
const sensor_ops_t dht11_ops = {
    .type = SENSOR_TYPE_TEMP_HUMID,
    .init = dht11_init,
    .read = dht11_read,
    .deinit = dht11_deinit
};
