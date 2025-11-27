#include <Arduino.h>

// --- KONFIGURACJA PINÓW ---
const int PIN_PRESSURE = A0; 
const int PIN_TEMP = A1;     

// --- KONFIGURACJA ELEKTRONIKI ---
const float V_SUPPLY = 5.00;      // Zasilanie (zmierzyć miernikiem dla lepszej dokładności!)
const float R_PULLUP = 3000.0;    // Twój rezystor (zalecany bliski 3.16kOhm, np. 3.3kOhm)

// --- TABLICA KALIBRACYJNA TEMPERATURY (z Twojej tabeli) ---
// Wybrałem punkty co 10-20 stopni, kod sobie doliczy resztę (interpolacja)
struct TempPoint {
  float resistance;
  float tempC;
};

// Tabela punktów: {Ohm, Stopnie C}
TempPoint tempTable[] = {
  {44864, -40}, {33676, -35}, {25524, -30}, {19525, -25},
  {15067, -20}, {11724, -15}, {9195,  -10}, {7266,  -5},
  {5784,   0},  {4636,   5},  {3740,   10}, {3037,  15},
  {2480,  20},  {2038,  25},  {1683,   30}, {1398,  35},
  {1167,  40},  {978.9, 45},  {825.0, 50},  {594.0, 60},
  {434.9, 70},  {323.4, 80},  {244.0, 90},  {186.6, 100},
  {144.5, 110}, {113.3, 120}, {89.8,  130}, {71.9,  140}
};

void setup() {
  Serial.begin(9600);
  Serial.println("Bosch 0 261 230 340 - Fuel/Oil Pressure Mode");
}

float getTemperature(float resistance) {
  // Jeśli poza zakresem, zwróć krańce
  if (resistance >= tempTable[0].resistance) return -40.0;
  int tableSize = sizeof(tempTable) / sizeof(tempTable[0]);
  if (resistance <= tempTable[tableSize - 1].resistance) return 140.0;

  // Szukanie w tabeli i interpolacja liniowa
  for (int i = 0; i < tableSize - 1; i++) {
    if (resistance <= tempTable[i].resistance && resistance > tempTable[i+1].resistance) {
      // Znaleziono przedział
      float rHigh = tempTable[i].resistance;
      float rLow = tempTable[i+1].resistance;
      float tLow = tempTable[i].tempC;   // Odpowiada rHigh (bo rezystancja spada gdy temp rośnie)
      float tHigh = tempTable[i+1].tempC; 

      // Wzór na interpolację
      float fraction = (resistance - rLow) / (rHigh - rLow);
      return tHigh - (fraction * (tHigh - tLow));
    }
  }
  return 0.0; // Błąd
}

void loop() {
  // --- 1. OBLICZANIE CIŚNIENIA ---
  // Wg Twojego obrazka: U_out = (c1 * p + c0) * Us
  // c1 = 0.0008, c0 = 0.1, Us = 5.0
  // U_out = (0.0008 * p + 0.1) * 5.0
  // U_out = 0.004 * p + 0.5
  // Przekształcamy, aby wyliczyć p (kPa):
  // p = (U_out - 0.5) / 0.004  => czyli: p = (U_out - 0.5) * 250
  
  int adcPressure = analogRead(PIN_PRESSURE);
  float voltagePressure = (adcPressure * V_SUPPLY) / 1023.0;
  
  float pressure_kPa = (voltagePressure - 0.5) * 250.0;
  
  // Korekta ujemnych wartości (szum przy 0 bar)
  if (pressure_kPa < 0) pressure_kPa = 0.0;
  float pressure_bar = pressure_kPa / 100.0;


  // --- 2. OBLICZANIE TEMPERATURY ---
  int adcTemp = analogRead(PIN_TEMP);
  float voltageTemp = (adcTemp * V_SUPPLY) / 1023.0;
  
  // Oblicz rezystancję czujnika z dzielnika napięcia
  // V_out = V_supply * R_NTC / (R_pullup + R_NTC)
  // R_NTC = (V_out * R_pullup) / (V_supply - V_out)
  
  float resistanceNTC = 0.0;
  float temperature = 0.0;

  if (voltageTemp > 0.1 && voltageTemp < 4.9) {
     resistanceNTC = (voltageTemp * R_PULLUP) / (V_SUPPLY - voltageTemp);
     temperature = getTemperature(resistanceNTC);
  }

  // --- WYNIKI ---
  Serial.print("P_Volts: "); Serial.print(voltagePressure);
  Serial.print("V | Cisnienie: "); Serial.print(pressure_bar); Serial.print(" bar (");
  Serial.print(pressure_kPa); Serial.print(" kPa)");
  
  Serial.print(" || T_Res: "); Serial.print(resistanceNTC);
  Serial.print(" Ohm | Temp: "); Serial.print(temperature); Serial.println(" C");

  delay(500);
}