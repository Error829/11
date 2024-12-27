#ifndef DRIVERS_BUZZER_H
#define DRIVERS_BUZZER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// 蜂鸣器音效模式
typedef enum {
    BUZZER_MODE_SINGLE_BEEP,    // 单次蜂鸣
    BUZZER_MODE_DOUBLE_BEEP,    // 双蜂鸣
    BUZZER_MODE_CONTINUOUS,     // 持续蜂鸣
    BUZZER_MODE_SOS,           // SOS报警音
    BUZZER_MODE_ALERT          // 警报音
} BuzzerMode;

// 初始化蜂鸣器
int BuzzerInit(void);

// 设置蜂鸣器频率
int BuzzerSetFrequency(uint32_t frequency);

// 开启蜂鸣器
int BuzzerOn(void);

// 关闭蜂鸣器
int BuzzerOff(void);

// 播放指定模式的音效
int BuzzerPlayMode(BuzzerMode mode);

// 停止当前音效
int BuzzerStop(void);

#ifdef __cplusplus
}
#endif

#endif // DRIVERS_BUZZER_H 