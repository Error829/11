#include <stdio.h>
#include <unistd.h>
#include "drivers/sensor/sensor.h"

// 声明DHT11操作接口
extern const sensor_ops_t dht11_ops;

int main(void)
{
    sensor_status_t status;
    sensor_data_t sensor_data;

    printf("DHT11传感器测试程序启动...\n");

    // 初始化传感器
    status = dht11_ops.init();
    if (status != SENSOR_OK) {
        printf("DHT11初始化失败，错误码：%d\n", status);
        return -1;
    }
    printf("DHT11初始化成功\n");

    // 循环读取数据
    for (int i = 0; i < 10; i++) {
        // 读取传感器数据
        status = dht11_ops.read(&sensor_data);
        if (status != SENSOR_OK) {
            printf("读取数据失败，错误码：%d\n", status);
            continue;
        }

        // 打印数据
        printf("第%d次读取:\n", i + 1);
        printf("温度: %.1f℃\n", sensor_data.temperature);
        printf("湿度: %.1f%%\n", sensor_data.humidity);
        printf("\n");

        // 等待2秒再次读取
        sleep(2);
    }

    // 反初始化传感器
    status = dht11_ops.deinit();
    if (status != SENSOR_OK) {
        printf("DHT11反初始化失败，错误码：%d\n", status);
        return -1;
    }
    printf("DHT11测试程序结束\n");

    return 0;
} 