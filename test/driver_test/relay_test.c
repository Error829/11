#include <stdio.h>
#include <unistd.h>
#include "drivers/output/relay.h"

// 测试继电器基本功能
void TestRelayBasic(void)
{
    printf("Testing basic relay control...\n");
    
    // 测试开启继电器
    printf("Turning relay ON...\n");
    RelaySetState(RELAY_STATE_ON);
    sleep(2);
    
    // 检查状态
    RelayState state = RelayGetState();
    printf("Relay state: %s\n", state == RELAY_STATE_ON ? "ON" : "OFF");
    
    // 测试关闭继电器
    printf("Turning relay OFF...\n");
    RelaySetState(RELAY_STATE_OFF);
    sleep(1);
    
    // 再次检查状态
    state = RelayGetState();
    printf("Relay state: %s\n", state == RELAY_STATE_ON ? "ON" : "OFF");
}

// 测试风扇速度控制
void TestFanSpeed(void)
{
    printf("\nTesting fan speed control...\n");
    
    // 测试各个速度级别
    FanSpeed speeds[] = {
        FAN_SPEED_LOW,
        FAN_SPEED_MEDIUM,
        FAN_SPEED_HIGH,
        FAN_SPEED_OFF
    };
    
    const char* speed_names[] = {
        "LOW",
        "MEDIUM",
        "HIGH",
        "OFF"
    };
    
    for (int i = 0; i < sizeof(speeds) / sizeof(speeds[0]); i++) {
        printf("Setting fan speed to %s...\n", speed_names[i]);
        RelaySetFanSpeed(speeds[i]);
        
        // 检查速度设置
        FanSpeed current_speed = RelayGetFanSpeed();
        printf("Current fan speed: %s\n", speed_names[current_speed]);
        
        sleep(3);  // 让风扇运行一段时间
    }
}

// 测试保护机制
void TestProtection(void)
{
    printf("\nTesting protection mechanisms...\n");
    
    // 测试超时保护
    printf("Testing timeout protection...\n");
    printf("Turning relay ON and waiting for timeout...\n");
    RelaySetState(RELAY_STATE_ON);
    
    // 等待超过保护时间
    sleep(6);  // 超过5秒的保护时间
    
    // 检查状态和错误
    RelayState state = RelayGetState();
    RelayError error = RelayGetError();
    
    printf("Relay state: %s\n", state == RELAY_STATE_ON ? "ON" : "OFF");
    printf("Error state: %d\n", error);
    
    // 清除错误
    printf("Clearing error...\n");
    RelayClearError();
    printf("Current error state: %d\n", RelayGetError());
}

int main(void)
{
    printf("Relay and Fan Control Test Program\n");
    
    // 初始化
    if (RelayInit() != 0) {
        printf("Failed to initialize relay and fan control!\n");
        return -1;
    }
    printf("Initialization successful.\n\n");
    
    // 运行测试
    TestRelayBasic();
    TestFanSpeed();
    TestProtection();
    
    // 清理
    printf("\nCleaning up...\n");
    RelayDeinit();
    printf("Test completed.\n");
    
    return 0;
} 