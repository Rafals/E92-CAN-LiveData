# 📟 BMW E92 335d CAN Live Data Display

Projekt systemu do odczytu parametrów w czasie rzeczywistym z magistrali CAN w samochodzie **BMW E92 335d (2007, ECU EDC16CP35)** z wykorzystaniem **ESP32** oraz **TJA1050**. System zbudowany z myślą o integracji z ekranem wyświetlającym dane na żywo.

## 🚀 Technologie / Stack

- **Mikrokontroler:** ESP32 (WiFi + BT)
- **Transceiver CAN:** TJA1050
- **Protokół komunikacji:** ISO 15765-4 CAN / OBD-II (Mode 01 i Mode 22)
- **Interfejs użytkownika:** docelowo ekran TFT/OLED
- **Język programowania:** C++ (Arduino ESP32 Core)
- **Komunikacja z autem:** magistrala CAN 500kbps (złącze OBD2, pin 6/14)

## 💡 Założenia projektu

- Czytanie danych z magistrali CAN bezpośrednio (sniffing + custom PID requests)
- Obsługa zarówno **standardowych PID-ów OBD-II**, jak i **niestandardowych PID-ów BMW (Mode 22)**
- Wyświetlanie parametrów live na ekranie
- Skalowalna baza PID z możliwością rozszerzania
  
---

## 📊 Parametry live — status odczytu

### ✅ Udało się odczytać:

- ✅ Temperatura cieczy chłodzącej
- ✅ Temperatura w przepływomierzu
- ✅ Obroty silnika (RPM)
- ✅ Przepływ powietrza (MAF w g/s lub mg/str)
- ✅ Ciśnienie doładowania (boost)


### ⏳ Nadal w trakcie reverse engineeringu:
- [ ] Ciśnienie paliwa
- [ ] Temperatura paliwa
- [ ] Temperatura powietrza doładowanego (intercooler)
- [ ] Temperatura skrzyni biegów
- [ ] AFR (lambda)
- [ ] Napięcie akumulatora

---

## 📺 Możliwości wyświetlacza (planowane UI)

- Tryb "live dash" — ekran parametrów w czasie rzeczywistym
- Możliwość wyświetlania parametrów
- Intuicyjny interfejs oparty na bibliotece graficznej (TFT_eSPI / LVGL)

---

## ⚙️ Status projektu

- ✅ Odczyt standardowych PID OBD-II (Mode 01) → działa
- ✅ Wysłanie zapytań custom PID (Mode 22) → działa
- ✅ Sniffowanie CAN i analiza odpowiedzi → działa
- 🔄 W trakcie dekodowania custom PID (Mode 22)
- 🔜 W trakcie implementacji UI ekranu

---

## 📁 Struktura projektu (w przyszłości)

