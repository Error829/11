#include <stdio.h>
#include <unistd.h>
#include "drivers/sensor/mq2.h"

int main(void)
{
    printf("MQ-2烟雾传感器测试程序启动...\n");
    
    // 获取MQ-2操作接口
    const mq2_ops* ops = get_mq2_ops();
    if (ops == NULL) {
        printf("获取MQ-2操作接口失败!\n");
        return -1;
    }
    
    // 初始化MQ-2
    if (ops->init() != 0) {
        printf("MQ-2初始化失败!\n");
        return -1;
    }
    printf("MQ-2初始化成功\n");
    
    // 等待传感器预热(30秒)
    printf("等待传感器预热...\n");
    sleep(30);
    
    // 设置报警阈值
    ops->set_threshold(2000);
    printf("报警阈值设置为2000\n");
    
    // 循环读取数据10次
    mq2_data data;
    for (int i = 0; i < 10; i++) {
        if (ops->read(&data) == 0) {
            printf("第%d次读取:\n", i + 1);
            printf("  ADC原始值: %u\n", data.raw_value);
            printf("  气体浓度: %.2f ppm\n", data.gas_density);
            printf("  报警状态: %s\n", data.is_alarm ? "报警" : "正常");
        } else {
            printf("读取数据失败!\n");
        }
        sleep(1);
    }
    
    // 关闭加热
    ops->heat_off();
    printf("关闭加热\n");
    
    // 反初始化
    ops->deinit();
    printf("MQ-2测试程序结束\n");
    
    return 0;
} 