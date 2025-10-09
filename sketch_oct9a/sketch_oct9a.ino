#include <WiFiS3.h>
#include <MqttClient.h>

// ----- ตั้งค่า WiFi -----
char ssid[] = "vivo V29e 5G";       // <<!>> ใส่ชื่อ WiFi ของคุณ
char pass[] = "2345rocth";   // <<!>> ใส่รหัสผ่าน WiFi ของคุณ

// ----- ตั้งค่า MQTT -----
const char broker[] = "test.mosquitto.org"; // Broker สาธารณะ
int        port     = 1883;
const char pubTopic[] = "arduino/r4/status"; // Topic สำหรับส่งข้อมูล
const char subTopic[] = "arduino/r4/led";    // Topic สำหรับรับคำสั่ง

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

unsigned long lastMillis = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // ตั้งค่า LED Built-in
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  // เชื่อมต่อ WiFi
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    Serial.print(".");
    delay(5000);
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // เชื่อมต่อ MQTT Broker
  Serial.print("Attempting to connect to MQTT broker: ");
  Serial.println(broker);
  if (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());
    while (1);
  }
  Serial.println("You're connected to the MQTT broker!");

  // Subscribe รอรับคำสั่ง
  mqttClient.subscribe(subTopic);
  // ตั้งค่าฟังก์ชันที่จะทำงานเมื่อมีข้อความเข้ามา
  mqttClient.onMessage(onMqttMessage);
}

void loop() {
  // ทำให้การเชื่อมต่อ MQTT ทำงานอยู่เสมอ
  mqttClient.poll();

  // Publish ข้อความทุก 5 วินาที
  if (millis() - lastMillis > 5000) {
    lastMillis = millis();
    
    Serial.println("Publishing message...");
    mqttClient.beginMessage(pubTopic);
    mqttClient.print("Hello from Arduino!");
    mqttClient.endMessage();
  }
}

void Controlspeed(int value) {



}
// ฟังก์ชันที่จะทำงานเมื่อมีข้อความเข้ามาใน Topic ที่เรา Subscribe ไว้
void onMqttMessage(int messageSize) {
  Serial.print("Received a message on topic: ");
  Serial.println(mqttClient.messageTopic());

  String message;
  while (mqttClient.available()) {
    message += (char)mqttClient.read();
  }
  Serial.print("Message: ");
  Serial.println(message);

  if (message == "ON") {
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("LED is ON");
  } else if (message == "OFF") {
    digitalWrite(LED_BUILTIN, LOW);
    Serial.println("LED is OFF");
  }
}