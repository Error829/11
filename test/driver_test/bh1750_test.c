#include <stdio.h>
#include <unistd.h>
#include "drivers/sensor/bh1750.h"

int main(void)
{
    printf("BH1750光照传感器测试程序启动...\n");
    
    // 获取BH1750操作接口
    const bh1750_ops_t* ops = get_bh1750_ops();
    if (ops == NULL) {
        printf("获取BH1750操作接口失败!\n");
        return -1;
    }
    
    // 初始化BH1750
    if (ops->init() != 0) {
        printf("BH1750初始化失败!\n");
        return -1;
    }
    printf("BH1750初始化成功\n");
    
    // 测试不同分辨率模式
    bh1750_mode_t modes[] = {BH1750_MODE_H, BH1750_MODE_H2, BH1750_MODE_L};
    const char* mode_names[] = {"高分辨率模式", "高分辨率模式2", "低分辨率模式"};
    
    for (int mode = 0; mode < 3; mode++) {
        // 设置测量模式
        if (ops->set_mode(modes[mode]) != 0) {
            printf("设置测量模式失败!\n");
            continue;
        }
        printf("\n切换到%s:\n", mode_names[mode]);
        
        // 等待模式切换完成
        usleep(200000);  // 200ms
        
        // 在当前模式下读取5次数��
        bh1750_data_t data;
        for (int i = 0; i < 5; i++) {
            if (ops->read(&data) == 0) {
                printf("第%d次读取:\n", i + 1);
                printf("  原始值: %u\n", data.raw_value);
                printf("  光照强度: %.2f lx\n", data.lux);
            } else {
                printf("读取数据失败!\n");
            }
            sleep(1);
        }
    }
    
    // 断电
    ops->power_down();
    printf("\n关闭传感器\n");
    
    // 反初始化
    ops->deinit();
    printf("BH1750测试程序结束\n");
    
    return 0;
} 