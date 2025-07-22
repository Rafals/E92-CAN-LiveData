#include "driver/twai.h"

#define CAN_RX GPIO_NUM_12
#define CAN_TX GPIO_NUM_13

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("CAN sniffer start");

  // Konfiguracja magistrali CAN 500 kbps
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(CAN_RX, CAN_TX, TWAI_MODE_NORMAL);
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK) {
    Serial.println("❌ Błąd instalacji CAN");
    return;
  }

  if (twai_start() != ESP_OK) {
    Serial.println("❌ Błąd startu CAN");
    return;
  }

  Serial.println("✅ CAN uruchomiony – nasłuchiwanie ramek...");
}

void loop() {
  twai_message_t message;
  if (twai_receive(&message, pdMS_TO_TICKS(500)) == ESP_OK) {
    Serial.printf("ID: 0x%03X  DLC: %d  Data:", message.identifier, message.data_length_code);
    for (int i = 0; i < message.data_length_code; i++) {
      Serial.printf(" %02X", message.data[i]);
    }
    Serial.println();
  }
}
