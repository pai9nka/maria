#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Adafruit_VL53L0X.h"

// --- НАСТРОЙКИ ДИСПЛЕЯ (SPI) ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_MOSI     23
#define OLED_CLK      18
#define OLED_DC        2
#define OLED_CS        5
#define OLED_RESET     4
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

// --- НАСТРОЙКИ ДАТЧИКОВ ---
Adafruit_VL53L0X lox = Adafruit_VL53L0X();
const int radarPin = 14;      // Выход OUT от HW-164
const int buzzerPin = 25;     // Динамик/наушник (через резистор!)

// --- ПЕРЕМЕННЫЕ ---
volatile int pulseCount = 0;  // Счётчик импульсов радара
unsigned long lastMillis = 0; // Таймер обновления экрана
unsigned long lastToneTime = 0; // Таймер для пульсации звука

// Прерывание для подсчета импульсов радара (эффект Доплера)
void IRAM_ATTR countPulse() {
  pulseCount++;
}

void setup() {
  // 1. Запуск Serial для отладки
  Serial.begin(115200);
  delay(1000); // Пауза для стабилизации питания

  // 2. Инициализация I2C для дальномера (явно указываем пины)
  Wire.begin(21, 22); 

  // 3. Запуск дисплея
  if(!display.begin(SSD1306_SWITCHCAPVCC)) {
    Serial.println(F("SSD1306 failed"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.display();

  // 4. Запуск лазерного дальномера
  if (!lox.begin()) {
    Serial.println(F("VL53L0X failed"));
  }

  // 5. Настройка пинов радара и звука
  pinMode(radarPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(radarPin), countPulse, RISING);

  Serial.println(F("System Ready!"));
}

void loop() {
  // Обновляем данные раз в 200мс для плавной работы
  if (millis() - lastMillis >= 200) {
    detachInterrupt(digitalPinToInterrupt(radarPin)); // Пауза для расчёта
    
    // --- РАСЧЁТ СКОРОСТИ ---
    // pulseCount * 5 (т.к. 200мс это 1/5 секунды) = Гц
    float hz = pulseCount * 5.0; 
    float speedKmH = hz / 19.49; // Константа для HB100
    if (speedKmH < 1.2) speedKmH = 0; // Отсекаем шум

    // --- РАСЧЁТ ДИСТАНЦИИ ---
    VL53L0X_RangingMeasurementData_t measure;
    lox.rangingTest(&measure, false);
    int distance = measure.RangeMilliMeter;

    // --- ЛОГИКА ЗВУКА ---
    if (measure.RangeStatus != 4 && distance < 1220) {
      // Частота звука от дистанции (ближе = выше)
      int toneFreq = map(distance, 50, 1200, 2200, 400);
      
      // Частота писка от скорости (быстрее = чаще)
      int beepDelay = map(speedKmH, 0, 25, 500, 60);
      beepDelay = constrain(beepDelay, 60, 500);

      if (millis() - lastToneTime > beepDelay) {
        tone(buzzerPin, toneFreq, 40); // Короткий импульс звука
        lastToneTime = millis();
      }
    } else {
      noTone(buzzerPin);
    }

    // --- ВЫВОД НА ЭКРАН ---
    display.clearDisplay();
    
    // Секция скорости
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("RADAR SPEED:");
    display.setTextSize(2);
    display.setCursor(10, 12);
    display.print(speedKmH, 1); display.print(" km/h");

    // Секция дистанции
    display.setTextSize(1);
    display.setCursor(0, 35);
    display.print("LASER DISTANCE:");
    display.setTextSize(2);
    display.setCursor(10, 47);
    if (measure.RangeStatus != 4) {
      display.print(distance); display.print(" mm");
    } else {
      display.print("--- mm");
    }

    display.display();

    // Сброс для следующего цикла
    pulseCount = 0;
    lastMillis = millis();
    attachInterrupt(digitalPinToInterrupt(radarPin), countPulse, RISING);
  }
}
