# 空间站环境监控系统技术文档

## 1. 项目概述

本项目是一个用于空间站的环境监控系统,通过多种传感器实时监测空间站内部环境参数,并提供报警和数据管理功能。系统采用分层设计,通过事件驱动的方式实现模块间的解耦,具有良好的可维护性和扩展性。

## 2. 系统功能

### 2.1 环境监测
- 温度监测: 使用DHT11传感器,实时监测环境温度
- 湿度监测: 使用DHT11传感器,实时监测环境湿度
- 烟雾监测: 使用MQ2传感器,监测空气中的烟雾浓度
- 光照监测: 使用BH1750传感器,监测环境光照强度

### 2.2 报警管理
- 多级报警: 支持信息、警告、严重三个报警级别
- 可配置规则: 支持设置各类环境参数的报警阈值
- 报警提示: LED指示灯和蜂鸣器提示
- 报警记录: 保存历史报警记录

### 2.3 数据管理
- 实时数据采集: 定时采集各传感器数据
- 数据缓存: 为每种传感器维护独立的数据缓存
- 数据查询: 支持查询最新数据和历史数据

## 3. 系统架构

### 3.1 分层设计
```
services/
    └── 依赖 --> business/
                  └── 依赖 --> data/
                                └── 依赖 --> drivers/
```

1. 驱动层(drivers/)
   - sensor: 传感器驱动(DHT11/MQ2/BH1750)
   - output: 输出设备驱动(LED/蜂鸣器)
   - display: 显示设备驱动
   - 不依赖其他模块,提供硬件抽象接口

2. 数据层(data/)
   - 数据采集模块
   - 数据缓存管理
   - 数据处理
   - 依赖驱动层获取传感器数据

3. 业务层(business/)
   - 报警管理
   - 规则处理
   - 事件处理
   - 依赖数据层和驱动层

4. 服务层(services/)
   - 系统服务
   - 通信服务
   - 依赖业务层实现功能

## 4. 运行机制

### 4.1 系统启动流程

```
main()
├── SystemInit()                    // 系统初始化
    ├── InitSensors()              // 传感器初始化
    │   ├── DHT11Init()            // 温湿度传感器初始化
    │   ├── MQ2Init()              // 烟雾传感器初始化
    │   └── BH1750Init()           // 光照传感器初始化
    │
    ├── CollectorInit()            // 数据采集模块初始化
    │   ├── 创建数据缓存
    │   └── 创建采集定时器
    │
    ├── AlarmInit()                // 报警模块初始化
    │   ├── 初始化LED和蜂鸣器
    │   └── 创建报警记录缓存
    │
    └── InitAlarmRules()           // 初始化默认报警规则

└── SystemStart()                  // 启动系统
    ├── CollectorStart()           // 启动数据采集
    └── 启动报警检查定时器
```

### 4.2 数据流向

1. 数据采集流向
```
传感器 --> 驱动层 --> 数据采集模块 --> 数据缓存
                                  └--> 触发回调通知
```

2. 报警处理流向
```
数据缓存 --> 报警检查 --> 报警触发 --> 输出设备
                               └--> 报警记录
```

### 4.3 事件处理机制

1. 定时器事件
   - 数据采集定时器: 周期性触发数据采集
   - 报警检查定时器: 周期性检查报警条件

2. 回调事件
   - 数据更新回调: 数据采集完成后触发
   - 报警回调: 报警触发时执行

### 4.4 状态管理

1. 系统状态机
```
SYSTEM_STATE_INIT --> SYSTEM_STATE_RUNNING
                 \--> SYSTEM_STATE_ERROR
```

2. 模块状态
   - 数据采集模块状态
   ```
   COLLECTOR_STATE_IDLE --> COLLECTOR_STATE_RUNNING
                       \--> COLLECTOR_STATE_ERROR
   ```
   - 报警模块状态
   ```
   ALARM_STATE_NORMAL --> ALARM_STATE_WARNING
                     \--> ALARM_STATE_CRITICAL
   ```

## 5. 核心代码实现

### 5.1 系统初始化
```c
// 系统初始化
int SystemInit(const SystemConfig* config)
{
    if (config == NULL) {
        UpdateSystemState(SYSTEM_STATE_ERROR, SYSTEM_ERROR_INIT_FAILED);
        return -1;
    }
    
    int ret = 0;
    
    // 初始化传感器
    ret = InitSensors();
    if (ret != 0) {
        UpdateSystemState(SYSTEM_STATE_ERROR, SYSTEM_ERROR_SENSOR);
        return -1;
    }
    
    // 初始化数据采集模块
    CollectorConfig collector_config = {
        .collect_interval = config->collect_interval,
        .cache_size = 10  // 每个传感器缓存10条数据
    };
    ret = CollectorInit(&collector_config);
    if (ret != 0) {
        UpdateSystemState(SYSTEM_STATE_ERROR, SYSTEM_ERROR_COLLECTOR);
        return -1;
    }
    
    // 初始化报警管理模块
    ret = AlarmInit();
    if (ret != 0) {
        UpdateSystemState(SYSTEM_STATE_ERROR, SYSTEM_ERROR_ALARM);
        return -1;
    }
    
    UpdateSystemState(SYSTEM_STATE_INIT, SYSTEM_ERROR_NONE);
    return 0;
}
```

### 5.2 数据采集实现
```c
// 数据采集定时器回调
static void TimerCallback(void* arg)
{
    if (g_state != COLLECTOR_STATE_RUNNING) {
        return;
    }
    
    // 采集所有传感器数据
    for (SensorType type = SENSOR_TYPE_DHT11; type < SENSOR_TYPE_MAX; type++) {
        if (CollectData(type) != 0) {
            UpdateState(COLLECTOR_STATE_ERROR, COLLECTOR_ERROR_SENSOR);
            return;
        }
    }
}
```

### 5.3 报警处理实现
```c
// 检查报警条件
int AlarmCheck(void)
{
    SensorData data = {0};
    
    // 检查每个报警规则
    for (int i = 0; i < ALARM_TYPE_COUNT; i++) {
        AlarmRule* rule = &g_alarm_rules[i];
        if (!rule->isEnabled) {
            continue;
        }
        
        // 获取对应的传感器数据
        SensorType sensor_type = GetSensorType(rule->type);
        if (sensor_type >= SENSOR_TYPE_MAX) {
            continue;
        }
        
        if (CollectorGetLatestData(sensor_type, &data) != 0) {
            continue;
        }
        
        // 获取传感器值并检查阈值
        float value = GetSensorValue(rule->type, &data);
        if (CheckThreshold(rule->type, value, rule)) {
            TriggerAlarm(rule, value);
        }
    }
    
    return 0;
}
```

### 5.4 传感器驱动实现
```c
// DHT11数据读取
static sensor_status_t dht11_read(sensor_data_t* data)
{
    uint8_t raw_data[5] = {0};  // DHT11原始数据
    sensor_status_t status;
    
    // 发送起始信号
    status = dht11_reset();
    if (status != SENSOR_OK) {
        return status;
    }

    // 读取5字节数据
    for (int i = 0; i < 5; i++) {
        status = dht11_read_byte(&raw_data[i]);
        if (status != SENSOR_OK) {
            return status;
        }
    }

    // 校验数据
    if (!dht11_check_sum(raw_data, 5)) {
        return SENSOR_ERROR_CHECKSUM;
    }

    // 解析数据
    data->humidity = (float)raw_data[0] + raw_data[1] * 0.1f;
    data->temperature = (float)raw_data[2] + raw_data[3] * 0.1f;
    
    return SENSOR_OK;
}
```

### 5.5 输出设备实现
```c
// LED控制
void LEDSetColor(LEDColor color)
{
    switch (color) {
        case LED_COLOR_RED:
            gpio_set_output(LED_PIN_R, 1);
            gpio_set_output(LED_PIN_G, 0);
            gpio_set_output(LED_PIN_B, 0);
            break;
        case LED_COLOR_GREEN:
            gpio_set_output(LED_PIN_R, 0);
            gpio_set_output(LED_PIN_G, 1);
            gpio_set_output(LED_PIN_B, 0);
            break;
        case LED_COLOR_BLUE:
            gpio_set_output(LED_PIN_R, 0);
            gpio_set_output(LED_PIN_G, 0);
            gpio_set_output(LED_PIN_B, 1);
            break;
        default:
            gpio_set_output(LED_PIN_R, 0);
            gpio_set_output(LED_PIN_G, 0);
            gpio_set_output(LED_PIN_B, 0);
            break;
    }
}

// 蜂鸣器控制
void BuzzerStart(uint32_t freq_hz, uint32_t duration_ms)
{
    // 设置PWM频率
    pwm_set_frequency(BUZZER_PIN, freq_hz);
    pwm_set_duty(BUZZER_PIN, 50);  // 50%占空比
    
    // 启动定时器自动关闭
    if (duration_ms > 0) {
        if (g_buzzer_timer == NULL) {
            osTimerAttr_t attr = {0};
            attr.name = "BuzzerTimer";
            g_buzzer_timer = osTimerNew(BuzzerTimerCallback, osTimerOnce, NULL, &attr);
        }
        
        if (g_buzzer_timer != NULL) {
            osTimerStart(g_buzzer_timer, duration_ms);
        }
    }
}
```

## 6. 错误处理

1. 传感器错误
   - 通信超时处理
   - 数据校验错误处理
   - 传感器故障检测

2. 系统错误
   - 内存分配错误处理
   - 定时器错误处理
   - 状态异常处理

## 7. 总结

该系统实现了一个功能完整、架构清晰的空间站环境监控系统。通过分层设计和模块化实现,系统具有良好的可维护性和扩展性。数据采集和报警处理是两个核心的处理链路,它们通过定时器触发运行,并通过回调机制实现模块间的通信。系统的状态管理确保了运行的可靠性,错误处理机制保障了系统的稳定性。 但系统仍有一定缺陷，该系统在开发过程中未能成功实现mqtt网络通信，导致无法将数据上传到云端，无法实现远程监控，具体原因为在开发过程中没能找到mqtt对应支持的部分文件导致如果使用mqtt功能在编译的时候就会一直因为找不到对应功能文件而报错，所以在mqtt部分我进行了略去，在项目具体编译的时候实际上没有进行mqtt相关功能板块的编译，于是项目依然有待完善，但受限于开发时间原因，后期有较多时间需要复习，所以暂时无法继续开发完善，止此项目暂时告一段落。