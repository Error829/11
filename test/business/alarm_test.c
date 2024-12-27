#include "business/alarm.h"
#include "data/data_collector.h"
#include "drivers/output/led.h"
#include "drivers/output/buzzer.h"
#include <stdio.h>
#include <unistd.h>

// 报警回调函数
static void OnAlarm(const AlarmRecord* record)
{
    printf("\n收到报警:\n");
    printf("类型: %s\n", GetAlarmTypeString(record->type));
    printf("级别: %s\n", GetAlarmLevelString(record->level));
    printf("时间: %ld\n", record->timestamp);
    printf("数值: %.2f\n", record->value);
    printf("描述: %s\n", record->description);
    printf("\n");
}

// 测试默认报警规则
static void TestDefaultRules(void)
{
    printf("\n测试默认报警规则:\n");
    
    AlarmRule rule;
    const AlarmType types[] = {
        ALARM_TYPE_TEMPERATURE_HIGH,
        ALARM_TYPE_TEMPERATURE_LOW,
        ALARM_TYPE_HUMIDITY_HIGH,
        ALARM_TYPE_HUMIDITY_LOW,
        ALARM_TYPE_SMOKE,
        ALARM_TYPE_LIGHT_HIGH,
        ALARM_TYPE_LIGHT_LOW
    };

    for (int i = 0; i < sizeof(types) / sizeof(AlarmType); i++) {
        if (AlarmGetRule(types[i], &rule) == 0) {
            printf("\n规则 %d:\n", i + 1);
            printf("类型: %s\n", GetAlarmTypeString(rule.type));
            printf("级别: %s\n", GetAlarmLevelString(rule.level));
            printf("上限阈值: %.2f\n", rule.thresholdHigh);
            printf("下限阈��: %.2f\n", rule.thresholdLow);
            printf("是否启用: %s\n", rule.isEnabled ? "是" : "否");
            printf("延迟时间: %d秒\n", rule.delaySeconds);
        }
    }
}

// 测试报警规则设置
static void TestAlarmRuleSettings(void)
{
    printf("\n测试报警规则设置:\n");

    // 创建新的报警规则
    AlarmRule newRule = {
        .type = ALARM_TYPE_TEMPERATURE_HIGH,
        .level = ALARM_LEVEL_WARNING,
        .thresholdHigh = 28.0f,
        .thresholdLow = -999.0f,
        .isEnabled = true,
        .delaySeconds = 2
    };

    // 设置新规则
    if (AlarmSetRule(&newRule) == 0) {
        printf("设置新规则成功\n");
    } else {
        printf("设置新规则失败\n");
        return;
    }

    // 获取并验证规则
    AlarmRule checkRule;
    if (AlarmGetRule(ALARM_TYPE_TEMPERATURE_HIGH, &checkRule) == 0) {
        printf("\n验证新规则:\n");
        printf("类型: %s\n", GetAlarmTypeString(checkRule.type));
        printf("级别: %s\n", GetAlarmLevelString(checkRule.level));
        printf("上限阈值: %.2f\n", checkRule.thresholdHigh);
        printf("下限阈值: %.2f\n", checkRule.thresholdLow);
        printf("是否启用: %s\n", checkRule.isEnabled ? "是" : "否");
        printf("延迟时间: %d秒\n", checkRule.delaySeconds);
    }
}

// 测试报警记录管理
static void TestAlarmRecords(void)
{
    printf("\n测试报警记录管理:\n");

    // 触发一些报警
    printf("触发测试报警...\n");
    for (int i = 0; i < 5; i++) {
        AlarmCheck();
        sleep(1);
    }

    // 获取最近的报警记录
    AlarmRecord records[10];
    uint32_t actualCount;
    
    if (AlarmGetLatestRecords(records, 10, &actualCount) == 0) {
        printf("\n最近的报警记录 (%d 条):\n", actualCount);
        for (uint32_t i = 0; i < actualCount; i++) {
            printf("\n记录 %d:\n", i + 1);
            printf("类型: %s\n", GetAlarmTypeString(records[i].type));
            printf("级别: %s\n", GetAlarmLevelString(records[i].level));
            printf("时间: %ld\n", records[i].timestamp);
            printf("数值: %.2f\n", records[i].value);
            printf("描述: %s\n", records[i].description);
        }
    }

    // 清除报警记录
    printf("\n清除报警记录...\n");
    if (AlarmClearRecords() == 0) {
        printf("清除成功\n");
    } else {
        printf("清除失败\n");
    }
}

// 测试报警规则启用/禁用
static void TestAlarmRuleEnableDisable(void)
{
    printf("\n测试报警规则启用/禁用:\n");

    // 禁用温度过高报警
    if (AlarmDisableRule(ALARM_TYPE_TEMPERATURE_HIGH) == 0) {
        printf("禁用温度过高报警成功\n");
    } else {
        printf("禁用温度过高报警失败\n");
    }

    // 验证规则状态
    AlarmRule rule;
    if (AlarmGetRule(ALARM_TYPE_TEMPERATURE_HIGH, &rule) == 0) {
        printf("温度过高报警状态: %s\n", rule.isEnabled ? "启用" : "禁用");
    }

    // 重新启用温度过高报警
    if (AlarmEnableRule(ALARM_TYPE_TEMPERATURE_HIGH) == 0) {
        printf("重新启用温度过高报警成功\n");
    } else {
        printf("重新启用温度过高报警失败\n");
    }

    // 再次验证规则状态
    if (AlarmGetRule(ALARM_TYPE_TEMPERATURE_HIGH, &rule) == 0) {
        printf("温度过高报警状态: %s\n", rule.isEnabled ? "启用" : "禁用");
    }
}

int main(void)
{
    printf("开始报警管理模块测试...\n");

    // 初始化报警管理模块
    if (AlarmInit() != 0) {
        printf("报警管理模块初始化失败\n");
        return -1;
    }
    printf("报警管理���块初始化成功\n");

    // 注册报警回调函数
    AlarmRegisterCallback(OnAlarm);

    // 运行测试用例
    TestDefaultRules();
    TestAlarmRuleSettings();
    TestAlarmRuleEnableDisable();
    TestAlarmRecords();

    // 清理
    if (AlarmDeinit() != 0) {
        printf("报警管理模块清理失败\n");
        return -1;
    }
    printf("报警管理模块清理成功\n");

    printf("报警管理模块测试完成\n");
    return 0;
} 