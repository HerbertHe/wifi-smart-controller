#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <EEPROM.h>

// 定义宏
#define SERIAL_INFRARED_CHANNEL 0  // 红外串口通道
#define SERIAL_433_CHANNEL 1       // 433串口通道

// 餐厅灯红外遥控码表
unsigned char light_open_close_infrared[5] = { 0xA1, 0xF1, 0x80, 0x7F, 0x01 };               // 开关
unsigned char light_night_infrared[5] = { 0xA1, 0xF1, 0x80, 0x7F, 0x05 };                    // 夜灯
unsigned char light_luminance_plus_infrared[5] = { 0xA1, 0xF1, 0x80, 0x7F, 0x12 };           // 亮度+
unsigned char light_luminance_minus_infrared[5] = { 0xA1, 0xF1, 0x80, 0x7F, 0x1E };          // 亮度-
unsigned char light_luminance_infrared[5] = { 0xA1, 0xF1, 0x80, 0x7F, 0xFF };                // 亮度
unsigned char light_auxiliary_infrared[5] = { 0xA1, 0xF1, 0x80, 0x7F, 0x1A };                // 辅助光源
unsigned char light_color_temperature_minus_infrared[5] = { 0xA1, 0xF1, 0x80, 0x7F, 0x03 };  // 色温-
unsigned char light_color_temperature_plus_infrared[5] = { 0xA1, 0xF1, 0x80, 0x7F, 0x02 };   // 色温+
unsigned char light_subsection_infrared[5] = { 0xA1, 0xF1, 0x80, 0x7F, 0x06 };               // 分段

// MQTT 服务器参数定义
const char* MQTT_SERVER = "<MQTT Server>";
const int MQTT_PORT = 1883;
const char* MQTT_USER = "<MQTT USER>";
const char* MQTT_PASSWORD = "<MQTT PASSWORD>";
String MQTT_CLIENT_ID = "Client_ESP_Smart_Controller_";

const char* GREETING_TOPIC = "esc/greeting";
const char* SUB_SERIAL = "esc/serial";  // 串口信号

WiFiClient wifiClient;
PubSubClient mqttClient;

int serial_channel = 0;

String util_byte_to_string(byte* b, unsigned int len) {
  char res[len];
  for (int i = 0; i < len; i++) {
    res[i] = (char)b[i];
  }
  res[len] = '\0';
  return String(res);
}

// 使能通道芯片
void en_serial_rx_channel(int c) {
  switch (c) {
    case SERIAL_INFRARED_CHANNEL:
      {
        digitalWrite(D0, LOW);
        digitalWrite(D1, LOW);
        digitalWrite(D2, LOW);
        serial_channel = 0;
        break;
      }

    case SERIAL_433_CHANNEL:
      {
        digitalWrite(D0, HIGH);
        digitalWrite(D1, LOW);
        digitalWrite(D2, LOW);
        serial_channel = 1;
        break;
      }
  }
}

// 红外灯信号发送
void serial_infrared_light_sender_func(int channel) {
  if (channel == -1) return;

  byte* data;
  switch (channel) {
    case 0:
      {
        data = light_open_close_infrared;
        break;
      }

    case 1:
      {
        data = light_night_infrared;
        break;
      }

    case 2:
      {
        data = light_luminance_plus_infrared;
        break;
      }

    case 3:
      {
        data = light_luminance_minus_infrared;
        break;
      }

    case 4:
      {
        data = light_luminance_infrared;
        break;
      }

    case 5:
      {
        data = light_auxiliary_infrared;
        break;
      }

    case 6:
      {
        data = light_color_temperature_minus_infrared;
        break;
      }

    case 7:
      {
        data = light_color_temperature_plus_infrared;
        break;
      }

    case 8:
      {
        data = light_subsection_infrared;
        break;
      }
  }

  Serial.write(data, 5);
  delay(100);
}

// 判断发送渠道，进行 switch 发送
void serial_sender_func(int channel = SERIAL_INFRARED_CHANNEL, int sub_channel = -1) {
  // 串口信号发送函数
  en_serial_rx_channel(channel);
  Serial.println(serial_channel);
  delay(100);
  switch (channel) {
    case SERIAL_INFRARED_CHANNEL:
      {
        // 红外信号发送
        serial_infrared_light_sender_func(sub_channel);
        break;
      }

    case SERIAL_433_CHANNEL:
      {
        break;
      }
  }
}

// WiFi 连接设置
void setup_wifi_connection() {
  Serial.println("");
  Serial.println("Connecting WiFi...");
  WiFi.setHostname("NodeMCU Smart Controller");
  WiFi.begin();

  int try_count = 0;

  Serial.println("Waiting for WiFi Connection...");

  // TODO 这可能会逻辑有点不合理，WiFi临时坏掉的问题
  // 没有连接上就自动更新
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
    try_count++;

    if (try_count > 5) {
      Serial.println("");
      Serial.println("Start WiFi SmartConfig...");
      WiFi.mode(WIFI_STA);

      WiFi.beginSmartConfig();
      while (1) {
        delay(100);
        if (WiFi.smartConfigDone()) {
          WiFi.setAutoConnect(true);
          try_count = 0;
          break;
        }
      }
    }
  }

  Serial.println("");
  Serial.println("WiFi Connected!");
  Serial.printf("SSID: %s", WiFi.SSID().c_str());
  Serial.print(", Gateway IP: ");
  Serial.print(WiFi.gatewayIP());
  Serial.print("\nLocal IP: ");
  Serial.print(WiFi.localIP());
}

void mqtt_handler_serial(String payload) {
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, payload);
  JsonObject obj = doc.as<JsonObject>();

  int type = obj["type"];               // 串口类型
  String device_id = obj["device_id"];  // 设备识别码
  int channel = obj["channel"];         // 串口信道
  unsigned int value = obj["val"];      // 串口值

  // 红外串口信道
  if (type == SERIAL_INFRARED_CHANNEL) {
    if (device_id == "restaurant_light") {
      serial_sender_func(SERIAL_INFRARED_CHANNEL, channel);
    }
    return;
  }

  // TODO 433串口信道
  if (type == SERIAL_433_CHANNEL) {
    return;
  }
}

// MQTT 消息回调函数
void mqtt_callback_func(char* topic, byte* payload, unsigned int len) {
  Serial.println("");
  Serial.printf("Received message in topic: \"%s\", length: %d\n", topic, len);
  String pl = util_byte_to_string(payload, len);

  if (String(topic) == String(SUB_SERIAL)) {
    mqtt_handler_serial(pl);
    return;
  }
}

// MQTT 连接设置
void setup_mqtt_connection() {
  Serial.println("");
  Serial.println("Setup MQTT Connection...");
  mqttClient.setClient(wifiClient);
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setCallback(mqtt_callback_func);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(100);

  setup_wifi_connection();

  // MQTT 挂掉进行任务处理和自动激活
  setup_mqtt_connection();

  pinMode(D0, OUTPUT);
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  digitalWrite(D0, LOW);
  digitalWrite(D1, LOW);
  digitalWrite(D2, LOW);
}

// 串口信号处理函数
void serial_handler_func() {
  char d = Serial.read();
  // 不用管乱码
  Serial.println(d, HEX);
}

// mqtt 服务循环
void mqtt_service_loop() {
  if (!mqttClient.connected()) {
    Serial.println("MQTT server is not connected...");
    // 没有连接服务
    MQTT_CLIENT_ID += String(WiFi.macAddress());
    Serial.println("Connecting MQTT Server...");
    // TODO 这里的判定逻辑有问题
    if (mqttClient.connect(MQTT_CLIENT_ID.c_str(), MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("MQTT Server Connected!");
      Serial.printf("Greeting to MQTT Server in topic: \"%s\"\n", GREETING_TOPIC);
      mqttClient.publish(GREETING_TOPIC, "Hello!");
      // 订阅主题消息
      mqttClient.subscribe(SUB_SERIAL);
    } else {
      Serial.println("Retry MQTT Connection...");
      // 暂停等待下一次连接
      delay(500);
    }
  } else {
    // TODO 定时反馈数据
    // mqttClient.publish();
  }

  mqttClient.loop();
}

// serial receiver 服务
void serial_receiver_service_loop() {
  if (Serial.available() > 0) {
    delay(100);
    int n = Serial.available();
    while (n--) {
      serial_handler_func();
    }
  }
}

void loop() {
  // WiFi 连接情况确认
  if (!WiFi.isConnected()) {
    setup_wifi_connection();
  }

  // 处理串口信号接收数据
  serial_receiver_service_loop();
  mqtt_service_loop();
}
