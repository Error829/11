#include <stdio.h>
#include <unistd.h>
#include "drivers/output/buzzer.h"

// 测试不同音效模式
void TestBuzzerModes(void)
{
    printf("Testing Single Beep...\n");
    BuzzerPlayMode(BUZZER_MODE_SINGLE_BEEP);
    sleep(1);
    
    printf("Testing Double Beep...\n");
    BuzzerPlayMode(BUZZER_MODE_DOUBLE_BEEP);
    sleep(1);
    
    printf("Testing Continuous Beep...\n");
    BuzzerPlayMode(BUZZER_MODE_CONTINUOUS);
    sleep(2);
    BuzzerStop();
    sleep(1);
    
    printf("Testing SOS...\n");
    BuzzerPlayMode(BUZZER_MODE_SOS);
    sleep(1);
    
    printf("Testing Alert...\n");
    BuzzerPlayMode(BUZZER_MODE_ALERT);
    sleep(1);
}

// 测试频率控制
void TestFrequencyControl(void)
{
    printf("Testing frequency control...\n");
    
    // 测试不同频率
    uint32_t frequencies[] = {1000, 2000, 3000, 4000};
    for (int i = 0; i < sizeof(frequencies) / sizeof(frequencies[0]); i++) {
        printf("Setting frequency to %uHz...\n", frequencies[i]);
        BuzzerSetFrequency(frequencies[i]);
        BuzzerOn();
        usleep(500000);  // 500ms
        BuzzerOff();
        usleep(200000);  // 200ms间隔
    }
}

int main(void)
{
    printf("Buzzer Test Program\n");
    
    // 初始化蜂鸣器
    if (BuzzerInit() != 0) {
        printf("Failed to initialize buzzer!\n");
        return -1;
    }
    printf("Buzzer initialized successfully.\n");
    
    // 测试频率控制
    TestFrequencyControl();
    sleep(1);
    
    // 测试音效模式
    TestBuzzerModes();
    
    printf("Buzzer test completed.\n");
    return 0;
} 