#include "driver/twai.h"

#define CAN_RX GPIO_NUM_12
#define CAN_TX GPIO_NUM_13

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

    Serial.println("System ready - Mode 01 OBD-II test");
}

const uint8_t STANDARD_PIDS[] = {
    0x05, // Coolant Temp
    0x0B, // Boost Pressure (MAP)
    0x0C, // RPM
    0x0F, // Intake Air Temp
    0x10 // MAF
};

void sendOBD2Mode01Request(uint8_t pid) {
    twai_message_t request;
    request.identifier = 0x7DF;
    request.extd = 0;
    request.rtr = 0;
    request.data_length_code = 8;

    request.data[0] = 0x02;     // liczba danych
    request.data[1] = 0x01;     // Mode 01
    request.data[2] = pid;      // PID
    memset(&request.data[3], 0x55, 5); // padding

    if (twai_transmit(&request, pdMS_TO_TICKS(100)) != ESP_OK) {
        Serial.println("Send error");
    } else {
        Serial.printf("Sent PID 0x%02X\n", pid);
    }
}

void processOBD2Response(twai_message_t &msg) {
    if (msg.identifier < 0x7E8 || msg.identifier > 0x7EF) return;
    if (msg.data[0] < 3 || msg.data[1] != 0x41) return;

    uint8_t pid = msg.data[2];
    float value = 0;
    String paramName;
    String unit;

    switch (pid) {
        case 0x05: // Coolant Temp
            value = msg.data[3] - 40;
            paramName = "Coolant Temp";
            unit = "°C";
            break;
        case 0x0B: // Boost Pressure (MAP)
            value = msg.data[3];
            paramName = "Boost Pressure";
            unit = "kPa";
            break;
        case 0x0C: // RPM
            value = ((msg.data[3] << 8) | msg.data[4]) / 4.0f;
            paramName = "RPM";
            unit = "";
            break;
        case 0x0F: // Intake Air Temp
            value = msg.data[3] - 40;
            paramName = "Intake Temp";
            unit = "°C";
            break;
        case 0x10: // MAF
            value = ((msg.data[3] << 8) | msg.data[4]) * 0.01f;
            paramName = "MAF";
            unit = "g/s";
            break;
        default:
            Serial.printf("Unknown PID: 0x%02X\n", pid);
            return;
    }

    Serial.printf("%s: %.1f %s\n", paramName.c_str(), value, unit.c_str());
}

void loop() {
    static unsigned long lastReq = 0;
    static uint8_t pidIndex = 0;

    if (millis() - lastReq > 1000) {
        lastReq = millis();
        sendOBD2Mode01Request(STANDARD_PIDS[pidIndex]);
        pidIndex = (pidIndex + 1) % (sizeof(STANDARD_PIDS) / sizeof(STANDARD_PIDS[0]));
    }

    twai_message_t response;
    if (twai_receive(&response, pdMS_TO_TICKS(100)) == ESP_OK) {
        processOBD2Response(response);
    }
}
