#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "network/mqtt_client.h"

// MQTT配置参数
#define TEST_MQTT_HOST         "mqtt.example.com"
#define TEST_MQTT_PORT         1883
#define TEST_MQTT_CLIENT_ID    "SpaceStation"
#define TEST_MQTT_USERNAME     "admin"
#define TEST_MQTT_PASSWORD     "password"
#define TEST_MQTT_KEEP_ALIVE   60
#define TEST_MQTT_TOPIC        "spacestation/sensors"
#define TEST_MQTT_QOS          1

// MQTT事件回调函数
static void OnMqttEvent(MqttState state, MqttError error)
{
    printf("MQTT Event - State: ");
    switch (state) {
        case MQTT_STATE_DISCONNECTED:
            printf("DISCONNECTED");
            break;
        case MQTT_STATE_CONNECTING:
            printf("CONNECTING");
            break;
        case MQTT_STATE_CONNECTED:
            printf("CONNECTED");
            break;
        case MQTT_STATE_DISCONNECTING:
            printf("DISCONNECTING");
            break;
        case MQTT_STATE_ERROR:
            printf("ERROR");
            break;
        default:
            printf("UNKNOWN");
            break;
    }
    
    printf(", Error: ");
    switch (error) {
        case MQTT_ERROR_NONE:
            printf("NONE");
            break;
        case MQTT_ERROR_PARAM:
            printf("PARAM");
            break;
        case MQTT_ERROR_MEMORY:
            printf("MEMORY");
            break;
        case MQTT_ERROR_NETWORK:
            printf("NETWORK");
            break;
        case MQTT_ERROR_TIMEOUT:
            printf("TIMEOUT");
            break;
        case MQTT_ERROR_PROTOCOL:
            printf("PROTOCOL");
            break;
        case MQTT_ERROR_AUTH:
            printf("AUTH");
            break;
        default:
            printf("UNKNOWN");
            break;
    }
    printf("\n");
}

// MQTT消息回调函数
static void OnMqttMessage(const MqttMessage* message)
{
    printf("MQTT Message Received:\n");
    printf("Topic: %s\n", message->topic);
    printf("QoS: %d\n", message->qos);
    printf("Retained: %d\n", message->retained);
    printf("Payload (%d bytes): ", message->payload_len);
    for (uint32_t i = 0; i < message->payload_len; i++) {
        printf("%02X ", message->payload[i]);
    }
    printf("\n");
}

// 测试基本连接功能
void TestBasicConnection(void)
{
    printf("\nTesting basic connection...\n");
    
    // 配置MQTT客户端
    MqttConfig config = {0};
    strncpy(config.host, TEST_MQTT_HOST, sizeof(config.host) - 1);
    config.port = TEST_MQTT_PORT;
    strncpy(config.client_id, TEST_MQTT_CLIENT_ID, sizeof(config.client_id) - 1);
    strncpy(config.username, TEST_MQTT_USERNAME, sizeof(config.username) - 1);
    strncpy(config.password, TEST_MQTT_PASSWORD, sizeof(config.password) - 1);
    config.keep_alive = TEST_MQTT_KEEP_ALIVE;
    config.clean_session = 1;
    
    printf("Configuring MQTT client...\n");
    if (MqttSetConfig(&config) != 0) {
        printf("Failed to configure MQTT client!\n");
        return;
    }
    
    // 连接MQTT服务器
    printf("Connecting to MQTT server...\n");
    if (MqttConnect() != 0) {
        printf("Failed to connect to MQTT server!\n");
        return;
    }
    
    // 等待连接完成
    sleep(2);
    
    // 断开连接
    printf("Disconnecting from MQTT server...\n");
    MqttDisconnect();
    sleep(1);
}

// 测试发布订阅功能
void TestPubSub(void)
{
    printf("\nTesting publish/subscribe...\n");
    
    // 连接MQTT服务器
    printf("Connecting to MQTT server...\n");
    if (MqttConnect() != 0) {
        printf("Failed to connect to MQTT server!\n");
        return;
    }
    
    // 等待连接完成
    sleep(2);
    
    // 订��主题
    printf("Subscribing to topic: %s\n", TEST_MQTT_TOPIC);
    if (MqttSubscribe(TEST_MQTT_TOPIC, TEST_MQTT_QOS) != 0) {
        printf("Failed to subscribe to topic!\n");
        MqttDisconnect();
        return;
    }
    
    // 等待订阅完成
    sleep(1);
    
    // 发布消息
    printf("Publishing message...\n");
    uint8_t payload[] = {0x01, 0x02, 0x03, 0x04};
    MqttMessage message = {
        .topic = TEST_MQTT_TOPIC,
        .payload = payload,
        .payload_len = sizeof(payload),
        .qos = TEST_MQTT_QOS,
        .retained = 0
    };
    if (MqttPublish(&message) != 0) {
        printf("Failed to publish message!\n");
        MqttUnsubscribe(TEST_MQTT_TOPIC);
        MqttDisconnect();
        return;
    }
    
    // 等待消息接收
    sleep(2);
    
    // 取消订阅
    printf("Unsubscribing from topic...\n");
    MqttUnsubscribe(TEST_MQTT_TOPIC);
    sleep(1);
    
    // 断开连接
    printf("Disconnecting from MQTT server...\n");
    MqttDisconnect();
    sleep(1);
}

int main(void)
{
    printf("MQTT Client Test Program\n");
    
    // 初始化MQTT客户端
    if (MqttInit() != 0) {
        printf("Failed to initialize MQTT client!\n");
        return -1;
    }
    printf("MQTT client initialized successfully.\n");
    
    // 注册回调函数
    MqttRegisterEventCallback(OnMqttEvent);
    MqttRegisterMessageCallback(OnMqttMessage);
    
    // 运行测试
    TestBasicConnection();
    TestPubSub();
    
    // 清理
    printf("\nCleaning up...\n");
    MqttDeinit();
    printf("Test completed.\n");
    
    return 0;
} 