#include <stdio.h>
#include <unistd.h>
#include "drivers/display/oled.h"

int main(void)
{
    printf("OLED测试程序启动...\n");
    
    // 获取OLED操作接口
    const oled_ops* ops = get_oled_ops();
    if (ops == NULL) {
        printf("获取OLED操作接口失败!\n");
        return -1;
    }
    
    // 初始化OLED
    if (ops->init() != 0) {
        printf("OLED初始化失败!\n");
        return -1;
    }
    printf("OLED初始化成功\n");
    
    // 清屏
    ops->clear();
    printf("清屏完成\n");
    
    // 显示测试
    ops->show_string(0, 0, "OLED Test");
    ops->show_string(0, 2, "Temperature:");
    ops->show_num(0, 3, 25);
    ops->show_string(24, 3, "C");
    ops->show_string(0, 5, "Humidity:");
    ops->show_num(0, 6, 60);
    ops->show_string(24, 6, "%");
    
    printf("显示测试完成\n");
    
    // 等待5秒
    sleep(5);
    
    // 反初始化
    ops->deinit();
    printf("OLED测试程序结束\n");
    
    return 0;
} 