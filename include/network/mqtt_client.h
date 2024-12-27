#ifndef NETWORK_MQTT_CLIENT_H
#define NETWORK_MQTT_CLIENT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// MQTT连接状态
typedef enum {
    MQTT_STATE_DISCONNECTED = 0,  // 未连接
    MQTT_STATE_CONNECTING,        // 正在连接
    MQTT_STATE_CONNECTED,         // 已连接
    MQTT_STATE_DISCONNECTING,     // 正在断开
    MQTT_STATE_ERROR              // 错误状态
} MqttState;

// MQTT错误码
typedef enum {
    MQTT_ERROR_NONE = 0,         // 无错误
    MQTT_ERROR_PARAM = -1,       // 参数错误
    MQTT_ERROR_MEMORY = -2,      // 内存错误
    MQTT_ERROR_NETWORK = -3,     // 网络错误
    MQTT_ERROR_TIMEOUT = -4,     // 超时错误
    MQTT_ERROR_PROTOCOL = -5,    // 协议错误
    MQTT_ERROR_AUTH = -6         // 认证错误
} MqttError;

// MQTT服务器配置
typedef struct {
    char host[128];              // 服务器地址
    uint16_t port;              // 服务器端口
    char client_id[64];         // 客户端ID
    char username[64];          // 用户名
    char password[64];          // 密码
    uint16_t keep_alive;        // 保活时间(秒)
    uint8_t clean_session;      // 清理会话标志
} MqttConfig;

// MQTT消息结构
typedef struct {
    char topic[128];            // 主题
    uint8_t* payload;           // 消息内容
    uint32_t payload_len;       // 消息长度
    uint8_t qos;               // 服务质量(0,1,2)
    uint8_t retained;          // 保留消息标志
} MqttMessage;

// MQTT事件回调函数类型
typedef void (*MqttEventCallback)(MqttState state, MqttError error);

// MQTT消息回调函数类型
typedef void (*MqttMessageCallback)(const MqttMessage* message);

// 初始化MQTT客户端
int MqttInit(void);

// 配置MQTT客户端
int MqttSetConfig(const MqttConfig* config);

// 获取MQTT配置
int MqttGetConfig(MqttConfig* config);

// 连接MQTT服务器
int MqttConnect(void);

// 断开MQTT连接
int MqttDisconnect(void);

// 获取MQTT状态
MqttState MqttGetState(void);

// 获取错误码
MqttError MqttGetError(void);

// 订阅主题
int MqttSubscribe(const char* topic, uint8_t qos);

// 取消订阅主题
int MqttUnsubscribe(const char* topic);

// 发布消息
int MqttPublish(const MqttMessage* message);

// 注册事件回调函数
int MqttRegisterEventCallback(MqttEventCallback callback);

// 注册消息回调函数
int MqttRegisterMessageCallback(MqttMessageCallback callback);

// 反初始化MQTT客户端
int MqttDeinit(void);

#ifdef __cplusplus
}
#endif

#endif // NETWORK_MQTT_CLIENT_H
