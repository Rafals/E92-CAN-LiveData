#include "driver/twai.h"

#define CAN_RX GPIO_NUM_12
#define CAN_TX GPIO_NUM_13

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("CAN – Temp cieczy i napięcie aku");

  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(CAN_RX, CAN_TX, TWAI_MODE_NORMAL);
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK || twai_start() != ESP_OK) {
    Serial.println("Błąd CAN");
    while (1);
  }

  Serial.println("CAN uruchomiony.");
}

void loop() {
  twai_message_t msg;
  if (twai_receive(&msg, pdMS_TO_TICKS(1000)) == ESP_OK) {
    // Temperaturka cieczy – ID 0x1D0, bajt 0
    if (msg.identifier == 0x1D0 && msg.data_length_code >= 1) {
      uint8_t raw = msg.data[0];
      int tempC = raw - 48;
      Serial.print("[0x1D0] Temp cieczy: ");
      Serial.print(tempC);
      Serial.println(" °C");
    }

    // Napięcie akumulatora – ID 0x3B4, bajty 0-2 (big endian 24-bit)
    if (msg.identifier == 0x3B4 && msg.data_length_code >= 3) {
  uint32_t raw = ((uint32_t)msg.data[0]) |
                 ((uint32_t)msg.data[1] << 8) |
                 ((uint32_t)(msg.data[2] & 0x0F) << 16); // tylko 4 bity z data[2]
  float voltage = raw * 0.001f;

  Serial.print("[0x3B4] Napięcie aku: ");
  Serial.print(voltage, 3);
  Serial.println(" V");
}

  }
}


