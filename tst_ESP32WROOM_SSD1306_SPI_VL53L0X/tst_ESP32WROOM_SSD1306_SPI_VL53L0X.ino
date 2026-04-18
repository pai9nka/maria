#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Adafruit_VL53L0X.h"

// Пины дисплея
#define OLED_MOSI   23       
#define OLED_CLK    18
#define OLED_DC     2
#define OLED_CS     5
#define OLED_RESET  4 

// Пин прерывания датчика
#define SENSOR_INT_PIN 25

Adafruit_SSD1306 display(128, 64, OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);
Adafruit_VL53L0X lox = Adafruit_VL53L0X();

volatile bool newDataReady = false;

void IRAM_ATTR isr_vl53l0x() {
  newDataReady = true;
}

void setup() {
  Serial.begin(115200);
  
  // 1. Инициализация дисплея
  if(!display.begin(SSD1306_SWITCHCAPVCC)) {
    for(;;);
  }
  display.clearDisplay();
  display.display();

  // 2. Инициализация датчика
  if (!lox.begin()) {
    Serial.println(F("Failed to boot VL53L0X"));
    while(1);
  }

  // 3. Настройка прерывания
  pinMode(SENSOR_INT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(SENSOR_INT_PIN), isr_vl53l0x, FALLING);

  // Исправленная константа для вашей версии библиотеки
  lox.setGpioConfig(VL53L0X_DEVICEMODE_CONTINUOUS_RANGING, 
                    VL53L0X_GPIOFUNCTIONALITY_NEW_MEASURE_READY, 
                    VL53L0X_INTERRUPTPOLARITY_LOW);

  lox.startRangeContinuous();
  
  // Сброс прерывания перед стартом, чтобы пин INT поднялся в HIGH
  lox.clearInterruptMask(false);
}

void loop() {
  if (newDataReady) {
    newDataReady = false;

    VL53L0X_RangingMeasurementData_t measure;
    lox.rangingTest(&measure, false);

    if (measure.RangeStatus != 4) {
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0,0);
      display.print("Dist: ");
      display.setTextSize(2);
      display.setCursor(0, 20);
      display.print(measure.RangeMilliMeter);
      display.print(" mm");
      display.display();
    }

    // Ключевая команда очистки для продолжения работы прерываний
    lox.clearInterruptMask(false);
  }
}
