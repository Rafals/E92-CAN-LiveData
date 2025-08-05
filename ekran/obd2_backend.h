// obd2_backend.h
#pragma once
#include <driver/twai.h>
#include <Arduino.h>

class OBD2Backend {
public:
    // Inicjalizacja komunikacji CAN
    void begin(gpio_num_t rxPin = GPIO_NUM_22, gpio_num_t txPin = GPIO_NUM_21);
    
    // Aktualizacja danych - wywoływana w pętli głównej
    void update();
    
    // Gettery dla danych
    float getCoolantTemp() const { return coolantTemp; }
    float getBoostPressure() const { return boostPressure; }
    float getRPM() const { return rpm; }
    float getIntakeTemp() const { return intakeTemp; }
    float getMAF() const { return maf; }
    
    // Status połączenia
    bool isConnected() const { return connected; }

private:
    void sendRequest(uint8_t pid);
    void processResponse(twai_message_t &msg);
    
    // Zmienne przechowujące dane
    float coolantTemp = 0.0f;
    float boostPressure = 0.0f;
    float rpm = 0.0f;
    float intakeTemp = 0.0f;
    float maf = 0.0f;
    bool connected = false;
    
    // Czas ostatniego requestu
    unsigned long lastRequestTime = 0;
    uint8_t currentPidIndex = 0;
};