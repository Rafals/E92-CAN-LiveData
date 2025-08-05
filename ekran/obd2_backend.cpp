// obd2_backend.cpp
#include "obd2_backend.h"

// Lista obsługiwanych PIDów
const uint8_t STANDARD_PIDS[] = {
    0x05, // Coolant Temp
    0x0B, // Boost Pressure (MAP)
    0x0C, // RPM
    0x0F, // Intake Air Temp
    0x10  // MAF
};

void OBD2Backend::begin(gpio_num_t rxPin, gpio_num_t txPin) {
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(txPin, rxPin, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
        if (twai_start() == ESP_OK) {
            connected = true;
            Serial.println("CAN started successfully");
            return;
        }
    }
    Serial.println("CAN initialization failed");
}

void OBD2Backend::update() {
    if (!connected) return;

    // Wysyłaj requesty co sekundę
    if (millis() - lastRequestTime > 100) {
        sendRequest(STANDARD_PIDS[currentPidIndex]);
        currentPidIndex = (currentPidIndex + 1) % (sizeof(STANDARD_PIDS) / sizeof(STANDARD_PIDS[0]));
        lastRequestTime = millis();
    }

    // Odbieraj odpowiedzi
    twai_message_t response;
    if (twai_receive(&response, pdMS_TO_TICKS(100)) == ESP_OK) {
        processResponse(response);
    }
}

void OBD2Backend::sendRequest(uint8_t pid) {
    twai_message_t request;
    request.identifier = 0x7DF;
    request.extd = 0;
    request.rtr = 0;
    request.data_length_code = 8;

    request.data[0] = 0x02; // Liczba bajtów danych
    request.data[1] = 0x01; // Tryb OBD2 (Show current data)
    request.data[2] = pid;  // PID
    memset(&request.data[3], 0x55, 5); // Padding

    if (twai_transmit(&request, pdMS_TO_TICKS(100)) != ESP_OK) {
        Serial.println("Failed to send CAN request");
    }
}

void OBD2Backend::processResponse(twai_message_t &msg) {
    // Sprawdź czy to prawidłowa odpowiedź OBD2
    if (msg.identifier < 0x7E8 || msg.identifier > 0x7EF) return;
    if (msg.data[0] < 3 || msg.data[1] != 0x41) return;

    // Przetwarzanie danych na podstawie PID
    switch (msg.data[2]) {
        case 0x05: // Coolant Temp (A-40°C)
            coolantTemp = msg.data[3] - 40;
            break;
            
        case 0x0B: // Boost Pressure (MAP) (kPa)
            boostPressure = msg.data[3] * 0.01f;
            break;
            
        case 0x0C: // RPM ((256*A+B)/4)
            rpm = ((msg.data[3] << 8) | msg.data[4]) / 4.0f;
            break;
            
        case 0x0F: // Intake Air Temp (A-40°C)
            intakeTemp = msg.data[3] - 40;
            break;
            
        case 0x10: // MAF ((256*A+B)/100 g/s)
            maf = ((msg.data[3] << 8) | msg.data[4]) * 0.01f;
            break;
            
        default:
            Serial.printf("Unknown PID: 0x%02X\n", msg.data[2]);
            break;
    }
}