#include <stdio.h>
#include "control/smart_controller.h"
#include "data/data_collector.h"

// 测试阈值规则
static void TestThresholdRule(void)
{
    printf("Testing threshold rule...\n");

    // 初始化控制器
    ControllerConfig config = {
        .max_rules = 10,
        .history_size = 100
    };
    
    if (ControllerInit(&config) != 0) {
        printf("Failed to initialize controller\n");
        return;
    }

    // 添加温度上限阈值规则
    ControlRule rule = {
        .type = RULE_TYPE_THRESHOLD,
        .enabled = 1,
        .device_id = 0, // 继电器
        .config.threshold = {
            .threshold = 30.0f,
            .hysteresis = 1.0f,
            .is_upper_bound = 1
        }
    };

    if (ControllerAddRule(&rule) != 0) {
        printf("Failed to add threshold rule\n");
        goto cleanup;
    }

    // 测试数据
    SensorData data = {
        .type = SENSOR_TYPE_DHT11,
        .timestamp = 0
    };

    // 测试温度变化
    float temps[] = {25.0f, 29.0f, 31.0f, 32.0f, 30.5f, 29.5f, 28.0f};
    uint32_t i;

    for (i = 0; i < sizeof(temps)/sizeof(temps[0]); i++) {
        data.data.dht11.temperature = temps[i];
        ControllerHandleData(&data);
        
        // 获取历史记录
        ControlHistory history[1];
        uint32_t count = 1;
        if (ControllerGetHistory(history, &count) == 0 && count > 0) {
            printf("Temperature: %.1f, Action: %d, Result: %d\n",
                   temps[i], history[0].action, history[0].result);
        }
    }

cleanup:
    ControllerDeinit();
}

// 测试定时规则
static void TestTimingRule(void)
{
    printf("\nTesting timing rule...\n");

    // 初始化控制器
    ControllerConfig config = {
        .max_rules = 10,
        .history_size = 100
    };
    
    if (ControllerInit(&config) != 0) {
        printf("Failed to initialize controller\n");
        return;
    }

    // 添加定时规则
    ControlRule rule = {
        .type = RULE_TYPE_TIMING,
        .enabled = 1,
        .device_id = 1, // 蜂鸣器
        .config.timing = {
            .start_time = 8 * 3600,  // 8:00
            .end_time = 18 * 3600,   // 18:00
            .interval = 3600         // 每小时
        }
    };

    if (ControllerAddRule(&rule) != 0) {
        printf("Failed to add timing rule\n");
        goto cleanup;
    }

    // 测试数据
    SensorData data = {
        .type = SENSOR_TYPE_DHT11,
        .timestamp = 0
    };

    // 模拟不同时间点
    uint32_t times[] = {7*3600, 8*3600, 9*3600, 17*3600, 18*3600, 19*3600};
    uint32_t i;

    for (i = 0; i < sizeof(times)/sizeof(times[0]); i++) {
        // 设置系统时间(仅用于测试)
        data.timestamp = times[i] * 1000;
        ControllerHandleData(&data);
        
        // 获取历史记录
        ControlHistory history[1];
        uint32_t count = 1;
        if (ControllerGetHistory(history, &count) == 0 && count > 0) {
            printf("Time: %02d:00, Action: %d, Result: %d\n",
                   times[i]/3600, history[0].action, history[0].result);
        }
    }

cleanup:
    ControllerDeinit();
}

// 测试联动规则
static void TestLinkageRule(void)
{
    printf("\nTesting linkage rule...\n");

    // 初始化控制器
    ControllerConfig config = {
        .max_rules = 10,
        .history_size = 100
    };
    
    if (ControllerInit(&config) != 0) {
        printf("Failed to initialize controller\n");
        return;
    }

    // 添加联动规则: 当烟雾浓度高时打开LED
    ControlRule rule = {
        .type = RULE_TYPE_LINKAGE,
        .enabled = 1,
        .device_id = 2, // LED
        .config.linkage = {
            .trigger_device = 1,  // MQ2
            .target_device = 2,   // LED
            .action = 1           // 打开
        }
    };

    if (ControllerAddRule(&rule) != 0) {
        printf("Failed to add linkage rule\n");
        goto cleanup;
    }

    // 测试数据
    SensorData data = {
        .type = SENSOR_TYPE_MQ2,
        .timestamp = 0
    };

    // 测试不同烟雾浓度
    float smoke_levels[] = {10.0f, 50.0f, 100.0f, 200.0f, 150.0f, 80.0f, 30.0f};
    uint32_t i;

    for (i = 0; i < sizeof(smoke_levels)/sizeof(smoke_levels[0]); i++) {
        data.data.mq2.smoke = smoke_levels[i];
        ControllerHandleData(&data);
        
        // 获取历史记录
        ControlHistory history[1];
        uint32_t count = 1;
        if (ControllerGetHistory(history, &count) == 0 && count > 0) {
            printf("Smoke: %.1f, Action: %d, Result: %d\n",
                   smoke_levels[i], history[0].action, history[0].result);
        }
    }

cleanup:
    ControllerDeinit();
}

// 主测试函数
int main(void)
{
    printf("Smart Controller Test\n");
    printf("====================\n\n");

    // 测试阈值规则
    TestThresholdRule();

    // 测试定时规则
    TestTimingRule();

    // 测试联动规则
    TestLinkageRule();

    printf("\nTest completed\n");
    return 0;
} 