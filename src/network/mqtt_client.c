#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "at_api.h"
#include "at_mqtt_api.h"
#include "network/mqtt_client.h"

// 全局变量
static MqttState g_mqtt_state = MQTT_STATE_DISCONNECTED;
static MqttError g_mqtt_error = MQTT_ERROR_NONE;
static MqttConfig g_mqtt_config = {0};
static MqttEventCallback g_event_callback = NULL;
static MqttMessageCallback g_message_callback = NULL;

// 状态更新函数
static void UpdateState(MqttState state, MqttError error)
{
    g_mqtt_state = state;
    g_mqtt_error = error;
    
    if (g_event_callback != NULL) {
        g_event_callback(state, error);
    }
}

// MQTT事件回调
static void OnMqttEvent(int event_id)
{
    switch (event_id) {
        case MQTT_CONNECT_SUCCESS:
            UpdateState(MQTT_STATE_CONNECTED, MQTT_ERROR_NONE);
            break;
        case MQTT_CONNECT_FAIL:
            UpdateState(MQTT_STATE_ERROR, MQTT_ERROR_NETWORK);
            break;
        case MQTT_DISCONNECT:
            UpdateState(MQTT_STATE_DISCONNECTED, MQTT_ERROR_NETWORK);
            break;
        default:
            break;
    }
}

// MQTT消息回调
static void OnMqttMessage(const char* topic, const uint8_t* payload, uint32_t payload_len)
{
    if (g_message_callback != NULL && topic != NULL && payload != NULL) {
        MqttMessage mqtt_msg = {0};
        strncpy(mqtt_msg.topic, topic, sizeof(mqtt_msg.topic) - 1);
        mqtt_msg.payload = (uint8_t*)payload;
        mqtt_msg.payload_len = payload_len;
        mqtt_msg.qos = 0;  // AT MQTT不支持QoS设置
        mqtt_msg.retained = 0;  // AT MQTT不支持retained消息
        g_message_callback(&mqtt_msg);
    }
}

// 初始化MQTT客户端
int MqttInit(void)
{
    // 初始化AT MQTT
    if (AtMqttInit() != AT_OK) {
        UpdateState(MQTT_STATE_ERROR, MQTT_ERROR_HARDWARE);
        return -1;
    }
    
    // 注册事件回调
    AtMqttRegisterEventCallback(OnMqttEvent);
    AtMqttRegisterMessageCallback(OnMqttMessage);
    
    UpdateState(MQTT_STATE_DISCONNECTED, MQTT_ERROR_NONE);
    return 0;
}

// 配置MQTT客户端
int MqttSetConfig(const MqttConfig* config)
{
    if (config == NULL) {
        UpdateState(MQTT_STATE_ERROR, MQTT_ERROR_PARAM);
        return -1;
    }
    
    // 保存配置
    memcpy(&g_mqtt_config, config, sizeof(MqttConfig));
    
    // 配置AT MQTT
    if (AtMqttSetConfig(config->host, config->port, config->client_id,
                       config->username, config->password) != AT_OK) {
        UpdateState(MQTT_STATE_ERROR, MQTT_ERROR_CONFIG);
        return -1;
    }
    
    return 0;
}

// 获取MQTT配置
int MqttGetConfig(MqttConfig* config)
{
    if (config == NULL) {
        return -1;
    }
    
    memcpy(config, &g_mqtt_config, sizeof(MqttConfig));
    return 0;
}

// 连接MQTT服务器
int MqttConnect(void)
{
    if (g_mqtt_state == MQTT_STATE_CONNECTED) {
        return 0;
    }
    
    UpdateState(MQTT_STATE_CONNECTING, MQTT_ERROR_NONE);
    
    if (AtMqttConnect() != AT_OK) {
        UpdateState(MQTT_STATE_ERROR, MQTT_ERROR_NETWORK);
        return -1;
    }
    
    return 0;
}

// 断开MQTT连接
int MqttDisconnect(void)
{
    if (g_mqtt_state == MQTT_STATE_DISCONNECTED) {
        return 0;
    }
    
    UpdateState(MQTT_STATE_DISCONNECTING, MQTT_ERROR_NONE);
    
    if (AtMqttDisconnect() != AT_OK) {
        UpdateState(MQTT_STATE_ERROR, MQTT_ERROR_NETWORK);
        return -1;
    }
    
    return 0;
}

// 获取MQTT状态
MqttState MqttGetState(void)
{
    return g_mqtt_state;
}

// 获取错误码
MqttError MqttGetError(void)
{
    return g_mqtt_error;
}

// 订阅主题
int MqttSubscribe(const char* topic, uint8_t qos)
{
    if (topic == NULL) {
        UpdateState(MQTT_STATE_ERROR, MQTT_ERROR_PARAM);
        return -1;
    }
    
    if (g_mqtt_state != MQTT_STATE_CONNECTED) {
        UpdateState(MQTT_STATE_ERROR, MQTT_ERROR_NETWORK);
        return -1;
    }
    
    if (AtMqttSubscribe(topic) != AT_OK) {
        UpdateState(MQTT_STATE_ERROR, MQTT_ERROR_PROTOCOL);
        return -1;
    }
    
    return 0;
}

// 取消订阅主题
int MqttUnsubscribe(const char* topic)
{
    if (topic == NULL) {
        UpdateState(MQTT_STATE_ERROR, MQTT_ERROR_PARAM);
        return -1;
    }
    
    if (g_mqtt_state != MQTT_STATE_CONNECTED) {
        UpdateState(MQTT_STATE_ERROR, MQTT_ERROR_NETWORK);
        return -1;
    }
    
    if (AtMqttUnsubscribe(topic) != AT_OK) {
        UpdateState(MQTT_STATE_ERROR, MQTT_ERROR_PROTOCOL);
        return -1;
    }
    
    return 0;
}

// 发布消息
int MqttPublish(const MqttMessage* message)
{
    if (message == NULL || message->payload == NULL) {
        UpdateState(MQTT_STATE_ERROR, MQTT_ERROR_PARAM);
        return -1;
    }
    
    if (g_mqtt_state != MQTT_STATE_CONNECTED) {
        UpdateState(MQTT_STATE_ERROR, MQTT_ERROR_NETWORK);
        return -1;
    }
    
    if (AtMqttPublish(message->topic, message->payload, message->payload_len) != AT_OK) {
        UpdateState(MQTT_STATE_ERROR, MQTT_ERROR_PROTOCOL);
        return -1;
    }
    
    return 0;
}

// 注册事件回调函数
int MqttRegisterEventCallback(MqttEventCallback callback)
{
    if (callback == NULL) {
        return -1;
    }
    
    g_event_callback = callback;
    return 0;
}

// 注册消息回调函数
int MqttRegisterMessageCallback(MqttMessageCallback callback)
{
    if (callback == NULL) {
        return -1;
    }
    
    g_message_callback = callback;
    return 0;
}

// 反初始化MQTT客户端
int MqttDeinit(void)
{
    if (g_mqtt_state == MQTT_STATE_CONNECTED) {
        MqttDisconnect();
    }
    
    AtMqttDeinit();
    UpdateState(MQTT_STATE_DISCONNECTED, MQTT_ERROR_NONE);
    return 0;
}
