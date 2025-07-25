#include "driver/twai.h"

#define CAN_RX GPIO_NUM_12
#define CAN_TX GPIO_NUM_13

#define ECU_REQUEST_ID 0x6F1
#define ECU_RESPONSE_ID 0x612

void setup() {
  Serial.begin(115200);
  delay(1000);

  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(CAN_TX, CAN_RX, TWAI_MODE_NORMAL);
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK) {
    Serial.println("CAN init error");
    return;
  }

  if (twai_start() != ESP_OK) {
    Serial.println("CAN start error");
    return;
  }

  Serial.println("System ready - Brute-force mode 0x22 (BMW EDC16CP35)");

  sendDiagnosticSessionRequest();
  delay(500);
}

void sendDiagnosticSessionRequest() {
  twai_message_t diag;
  diag.identifier = ECU_REQUEST_ID;
  diag.extd = 0;
  diag.rtr = 0;
  diag.data_length_code = 8;
  diag.data[0] = 0x10;
  diag.data[1] = 0x03;
  memset(&diag.data[2], 0x00, 6);

  if (twai_transmit(&diag, pdMS_TO_TICKS(100)) == ESP_OK) {
    Serial.println("Sent 0x10 0x03 - diagnostic session start");
  } else {
    Serial.println("Failed to send diagnostic session request");
  }
}

void sendMode22Request(uint16_t pid) {
  twai_message_t msg;
  msg.identifier = ECU_REQUEST_ID;
  msg.extd = 0;
  msg.rtr = 0;
  msg.data_length_code = 8;
  msg.data[0] = 0x03;        // 3 bajty danych
  msg.data[1] = 0x22;        // Service 0x22 - ReadDataByIdentifier
  msg.data[2] = (pid >> 8);  // PID high byte
  msg.data[3] = (pid & 0xFF);// PID low byte
  msg.data[4] = 0x00;
  msg.data[5] = 0x00;
  msg.data[6] = 0x00;
  msg.data[7] = 0x00;

  if (twai_transmit(&msg, pdMS_TO_TICKS(100)) != ESP_OK) {
    Serial.printf("Send error for PID 0x%04X\n", pid);
  }
}

void loop() {
  static uint16_t pid = 0x2000;
  static unsigned long lastSend = 0;

  // Wysyłaj co 150ms
  if (millis() - lastSend > 150) {
    lastSend = millis();
    sendMode22Request(pid);
    pid++;
    if (pid > 0x20FF) {
      Serial.println("Brute-force complete");
      while (1) delay(1000);
    }
  }

  // Odbiór odpowiedzi
  twai_message_t response;
  if (twai_receive(&response, pdMS_TO_TICKS(50)) == ESP_OK) {
    Serial.printf("Response for PID 0x%02X%02X: ", response.data[2], response.data[3]);
    for (int i = 0; i < response.data_length_code; i++) {
      Serial.printf("%02X ", response.data[i]);
    }
    Serial.println();
  }
}
