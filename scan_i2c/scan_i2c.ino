#include <Wire.h>

void setup() {
  Wire.begin(); // Инициализация I²C на стандартных пинах
  Serial.begin(115200);
  while (!Serial); // Ждём открытия Serial-порта (для некоторых плат)
  Serial.println("\nI²C Scanner");
}

void loop() {
  byte error, address;
  int nDevices = 0;

  Serial.println("Scanning...");

  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I²C device found at address 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      Serial.println(" !");
      nDevices++;
    } else if (error == 4) {
      Serial.print("Unknown error at address 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
    }
  }

  if (nDevices == 0) {
    Serial.println("No I²C devices found\n");
  } else {
    Serial.println("Scan completed.\n");
  }

  delay(5000); // Пауза 5 секунд перед следующим сканированием
}