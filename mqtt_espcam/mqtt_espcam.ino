#include <Mini-Project_AI_arduino_inferencing.h>
#include "edge-impulse-sdk/dsp/image/image.hpp"
#include "esp_camera.h"
#include "Board_camera.h"
#include "Config_cam.h"
#include <WiFi.h>
#include <MqttClient.h>

// ----- ตั้งค่า WiFi -----
char ssid[] = "vivo V29e 5G";       // <<!>> ใส่ชื่อ WiFi ของคุณ
char password[] = "2345rocth";   // <<!>> ใส่รหัสผ่าน WiFi ของคุณ

// ----- ตั้งค่า MQTT -----
const char broker[] = "broker.hivemq.com"; // Broker สาธารณะ
int        port     = 1883;
const char pubTopic[] = "esp/cam32/status"; // Topic สำหรับส่งข้อมูล
const char pubDataAi[] = "esp/cam32/data"; // Topic สำหรับส่งข้อมูล
/*Wifi mqtt*/
WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);
unsigned long lastMillis = 0;

/* Function definitions ------------------------------------------------------- */
bool ei_camera_init(void);
void ei_camera_deinit(void);
bool ei_camera_capture(uint32_t img_width, uint32_t img_height, uint8_t *out_buf) ;

void setup() {
  
  Serial.begin(115200);
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);
   //Wifi-esp32cam
    WiFi.begin(ssid, password);
    WiFi.setSleep(false);

    Serial.print("WiFi connecting");
    while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");

    /*------------------เชื่อมต่อ MQTT Broker-------------------------------*/
    Serial.print("Attempting to connect to MQTT broker: ");
    Serial.println(broker);
    if (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());
    while (1);
    }
    Serial.println("You're connected to the MQTT broker!");

    /*------------------Ai cam edge Impulse----------------------------*/
    while (!Serial);
    Serial.println("Edge Impulse Inferencing Demo");
    if (ei_camera_init() == false) {
        ei_printf("Failed to initialize Camera!\r\n");
    }
    else {
        ei_printf("Camera initialized\r\n");
    }

    ei_printf("\nStarting continious inference in 2 seconds...\n");
    ei_sleep(2000);

  

}

void loop() {
  // ทำให้การเชื่อมต่อ MQTT ทำงานอยู่เสมอ
  mqttClient.poll();

  // Publish ข้อความทุก 5 วินาที
  if (millis() - lastMillis > 5000) {
    lastMillis = millis();
    
    Serial.println("Publishing message...");
    mqttClient.beginMessage(pubTopic);
    mqttClient.print("Hello from Esp32cam!");
    mqttClient.endMessage();
  }
  //เรียกใช้ฟังก์ชัน
  Aicam();

}

void Aicam(){

    // instead of wait_ms, we'll wait on the signal, this allows threads to cancel us...
  if (ei_sleep(5) != EI_IMPULSE_OK) {
    return;
  }

  snapshot_buf = (uint8_t*)malloc(EI_CAMERA_RAW_FRAME_BUFFER_COLS * EI_CAMERA_RAW_FRAME_BUFFER_ROWS * EI_CAMERA_FRAME_BYTE_SIZE);

  // check if allocation was successful
  if(snapshot_buf == nullptr) {
    ei_printf("ERR: Failed to allocate snapshot buffer!\n");
    return;
  }

  ei::signal_t signal;
  signal.total_length = EI_CLASSIFIER_INPUT_WIDTH * EI_CLASSIFIER_INPUT_HEIGHT;
  signal.get_data = &ei_camera_get_data;

  if (ei_camera_capture((size_t)EI_CLASSIFIER_INPUT_WIDTH, (size_t)EI_CLASSIFIER_INPUT_HEIGHT, snapshot_buf) == false) {
      ei_printf("Failed to capture image\r\n");
      free(snapshot_buf);
      Serial.println("Publishing message data Ai...");
      mqttClient.beginMessage(pubDataAi);
      mqttClient.print("Failed to capture image");
      mqttClient.endMessage();
      return;
  }

  // Run the classifier
  ei_impulse_result_t result = { 0 };

  EI_IMPULSE_ERROR err = run_classifier(&signal, &result, debug_nn);
  if (err != EI_IMPULSE_OK) {
      ei_printf("ERR: Failed to run classifier (%d)\n", err);
      return;
  }

  // print the predictions
  ei_printf("Predictions (DSP: %d ms., Classification: %d ms., Anomaly: %d ms.): \n",
            result.timing.dsp, result.timing.classification, result.timing.anomaly);

#if EI_CLASSIFIER_OBJECT_DETECTION == 1
    ei_printf("Object detection bounding boxes:\r\n");
    for (uint32_t i = 0; i < result.bounding_boxes_count; i++) {
        ei_impulse_result_bounding_box_t bb = result.bounding_boxes[i];
        if (bb.value == 0) {
          mqttClient.beginMessage(pubDataAi);
          mqttClient.print("No Detection");
          mqttClient.endMessage();
            continue;
        }
        ei_printf("  %s (%f) [ x: %u, y: %u, width: %u, height: %u ]\r\n",
                bb.label,
                bb.value,
                bb.x,
                bb.y,
                bb.width,
                bb.height);
        Serial.println("Publishing message data Ai...");
        mqttClient.beginMessage(pubDataAi);
        mqttClient.print(bb.label);
        mqttClient.endMessage();
    }

    // Print the prediction results (classification)
#else
    ei_printf("Predictions:\r\n");
    for (uint16_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
        ei_printf("  %s: ", ei_classifier_inferencing_categories[i]);
        ei_printf("%.5f\r\n", result.classification[i].value);
    }
#endif

    // Print anomaly result (if it exists)
#if EI_CLASSIFIER_HAS_ANOMALY
    ei_printf("Anomaly prediction: %.3f\r\n", result.anomaly);
#endif

#if EI_CLASSIFIER_HAS_VISUAL_ANOMALY
    ei_printf("Visual anomalies:\r\n");
    for (uint32_t i = 0; i < result.visual_ad_count; i++) {
        ei_impulse_result_bounding_box_t bb = result.visual_ad_grid_cells[i];
        if (bb.value == 0) {
            continue;
        }
        ei_printf("  %s (%f) [ x: %u, y: %u, width: %u, height: %u ]\r\n",
                bb.label,
                bb.value,
                bb.x,
                bb.y,
                bb.width,
                bb.height);
    }
#endif


  free(snapshot_buf);



}