#include <stdio.h>
#include <unistd.h>
#include "drivers/output/led.h"

int main(void)
{
    printf("LED控制测试程序启动...\n");
    
    // 获取LED操作接口
    const led_ops_t* ops = get_led_ops();
    if (ops == NULL) {
        printf("获取LED操作接口失败!\n");
        return -1;
    }
    
    // 初始化LED
    if (ops->init() != 0) {
        printf("LED初始化失败!\n");
        return -1;
    }
    printf("LED初始化成功\n");
    
    // 测试LED开关
    printf("\n测试LED开关状态:\n");
    const char* color_names[] = {"红色", "绿色", "蓝色"};
    for (int i = 0; i < LED_COLOR_MAX; i++) {
        printf("测试%sLED:\n", color_names[i]);
        
        // 打开LED
        ops->set_state(i, LED_STATE_ON);
        printf("  LED打开\n");
        sleep(1);
        
        // 关闭LED
        ops->set_state(i, LED_STATE_OFF);
        printf("  LED关闭\n");
        sleep(1);
    }
    
    // 测试LED闪烁
    printf("\n测试LED闪烁模式:\n");
    
    // 测试快速闪烁
    printf("测试快速闪烁(200ms):\n");
    led_blink_t fast_blink = {200, 200, 5};  // 200ms闪烁5次
    for (int i = 0; i < LED_COLOR_MAX; i++) {
        printf("  %sLED开始闪烁\n", color_names[i]);
        ops->set_blink(i, &fast_blink);
        sleep(3);
        ops->set_state(i, LED_STATE_OFF);
    }
    
    // 测试慢速闪烁
    printf("\n测试慢速闪烁(1000ms):\n");
    led_blink_t slow_blink = {1000, 1000, 3};  // 1000ms闪烁3次
    for (int i = 0; i < LED_COLOR_MAX; i++) {
        printf("  %sLED开始闪烁\n", color_names[i]);
        ops->set_blink(i, &slow_blink);
        sleep(7);
        ops->set_state(i, LED_STATE_OFF);
    }
    
    // 测试持续闪烁和停止
    printf("\n测试持续闪烁和停止:\n");
    led_blink_t cont_blink = {500, 500, 0};  // 持续闪烁
    for (int i = 0; i < LED_COLOR_MAX; i++) {
        printf("  %sLED开始持续闪烁\n", color_names[i]);
        ops->set_blink(i, &cont_blink);
    }
    printf("等待5秒后停止所有LED...\n");
    sleep(5);
    
    // 停止所有LED
    for (int i = 0; i < LED_COLOR_MAX; i++) {
        ops->set_state(i, LED_STATE_OFF);
    }
    printf("所有LED已停止\n");
    
    // 反初始化
    ops->deinit();
    printf("\nLED测试程序结束\n");
    
    return 0;
} 