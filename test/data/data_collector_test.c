#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "data/data_collector.h"

// 测试配置参数
#define TEST_COLLECT_INTERVAL    1000    // 采集间隔1秒
#define TEST_CACHE_SIZE         10      // 缓存10条数据
#define TEST_RUN_TIME          10000    // 运行10秒

// 数据回调函数
static void OnData(const SensorData* data)
{
    if (data == NULL) {
        return;
    }
    
    printf("Sensor Data - Type: ");
    switch (data->type) {
        case SENSOR_TYPE_DHT11:
            printf("DHT11\n");
            printf("Temperature: %.1f°C\n", data->data.dht11.temperature);
            printf("Humidity: %.1f%%\n", data->data.dht11.humidity);
            break;
        case SENSOR_TYPE_MQ2:
            printf("MQ2\n");
            printf("Smoke: %.1fppm\n", data->data.mq2.smoke);
            break;
        case SENSOR_TYPE_BH1750:
            printf("BH1750\n");
            printf("Light: %.1flux\n", data->data.bh1750.light);
            break;
        default:
            printf("Unknown\n");
            break;
    }
    printf("Timestamp: %u\n", data->timestamp);
    printf("\n");
}

// 测试基本功能
void TestBasicFunction(void)
{
    printf("\nTesting basic function...\n");
    
    // 配置数据采集
    CollectorConfig config = {
        .collect_interval = TEST_COLLECT_INTERVAL,
        .cache_size = TEST_CACHE_SIZE
    };
    
    printf("Initializing collector...\n");
    if (CollectorInit(&config) != 0) {
        printf("Failed to initialize collector!\n");
        return;
    }
    
    // 注册回调函数
    CollectorRegisterCallback(OnData);
    
    // 启动采集
    printf("Starting collector...\n");
    if (CollectorStart() != 0) {
        printf("Failed to start collector!\n");
        CollectorDeinit();
        return;
    }
    
    // 运行一段时间
    printf("Running for %d seconds...\n", TEST_RUN_TIME / 1000);
    sleep(TEST_RUN_TIME / 1000);
    
    // 停止采集
    printf("Stopping collector...\n");
    CollectorStop();
    
    // 清理
    printf("Cleaning up...\n");
    CollectorDeinit();
}

// 测试手动触发
void TestManualTrigger(void)
{
    printf("\nTesting manual trigger...\n");
    
    // 配置数据采集
    CollectorConfig config = {
        .collect_interval = TEST_COLLECT_INTERVAL,
        .cache_size = TEST_CACHE_SIZE
    };
    
    printf("Initializing collector...\n");
    if (CollectorInit(&config) != 0) {
        printf("Failed to initialize collector!\n");
        return;
    }
    
    // 注册回调函数
    CollectorRegisterCallback(OnData);
    
    // 手动触发每个传感器
    printf("Triggering DHT11...\n");
    CollectorTrigger(SENSOR_TYPE_DHT11);
    sleep(1);
    
    printf("Triggering MQ2...\n");
    CollectorTrigger(SENSOR_TYPE_MQ2);
    sleep(1);
    
    printf("Triggering BH1750...\n");
    CollectorTrigger(SENSOR_TYPE_BH1750);
    sleep(1);
    
    // 清理
    printf("Cleaning up...\n");
    CollectorDeinit();
}

// 测试历史数据
void TestHistoryData(void)
{
    printf("\nTesting history data...\n");
    
    // 配置数据采集
    CollectorConfig config = {
        .collect_interval = TEST_COLLECT_INTERVAL,
        .cache_size = TEST_CACHE_SIZE
    };
    
    printf("Initializing collector...\n");
    if (CollectorInit(&config) != 0) {
        printf("Failed to initialize collector!\n");
        return;
    }
    
    // 启动采集
    printf("Starting collector...\n");
    if (CollectorStart() != 0) {
        printf("Failed to start collector!\n");
        CollectorDeinit();
        return;
    }
    
    // 运行一段时间收集数据
    printf("Collecting data for %d seconds...\n", TEST_RUN_TIME / 2000);
    sleep(TEST_RUN_TIME / 2000);
    
    // 停止采集
    printf("Stopping collector...\n");
    CollectorStop();
    
    // 获取历史数据
    printf("Getting history data...\n");
    for (SensorType type = SENSOR_TYPE_DHT11; type < SENSOR_TYPE_MAX; type++) {
        SensorData history[TEST_CACHE_SIZE];
        uint32_t count = TEST_CACHE_SIZE;
        
        if (CollectorGetHistoryData(type, history, &count) == 0) {
            printf("Found %u records for sensor type %d\n", count, type);
            for (uint32_t i = 0; i < count; i++) {
                OnData(&history[i]);
            }
        }
    }
    
    // 清除历史数据
    printf("Clearing history data...\n");
    for (SensorType type = SENSOR_TYPE_DHT11; type < SENSOR_TYPE_MAX; type++) {
        CollectorClearHistory(type);
    }
    
    // 清理
    printf("Cleaning up...\n");
    CollectorDeinit();
}

int main(void)
{
    printf("Data Collector Test Program\n");
    
    // 运行测试
    TestBasicFunction();
    TestManualTrigger();
    TestHistoryData();
    
    printf("\nTest completed.\n");
    return 0;
} 