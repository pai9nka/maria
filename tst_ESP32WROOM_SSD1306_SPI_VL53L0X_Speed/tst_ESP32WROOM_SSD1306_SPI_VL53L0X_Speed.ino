#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Adafruit_VL53L0X.h"

#define SENSOR_INT_PIN 25
#define OLED_MOSI 23       
#define OLED_CLK 18
#define OLED_DC 2
#define OLED_CS 5
#define OLED_RESET 4 

Adafruit_SSD1306 display(128, 64, OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);
Adafruit_VL53L0X lox = Adafruit_VL53L0X();

volatile bool newDataReady = false;
void IRAM_ATTR isr_vl53l0x() { newDataReady = true; }

// Настройки фильтрации
const int NUM_READINGS = 10; 
int readings[NUM_READINGS];
unsigned long times[NUM_READINGS];
int readIndex = 0;
bool bufferFull = false;

float smoothedSpeed = 0;
const float filterAlpha = 0.15; // Коэффициент плавности

const int GRAPH_WIDTH = 128;
int speedHistory[GRAPH_WIDTH];

void setup() {
  Serial.begin(115200);
  display.begin(SSD1306_SWITCHCAPVCC);
  
  if(!lox.begin()) while(1);

  pinMode(SENSOR_INT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(SENSOR_INT_PIN), isr_vl53l0x, FALLING);

  lox.setGpioConfig(VL53L0X_DEVICEMODE_CONTINUOUS_RANGING, 
                    VL53L0X_GPIOFUNCTIONALITY_NEW_MEASURE_READY, 
                    VL53L0X_INTERRUPTPOLARITY_LOW);
  lox.startRangeContinuous();
  
  for(int i=0; i<GRAPH_WIDTH; i++) speedHistory[i] = 45; // Линия на уровне оси
}

void loop() {
  if (newDataReady) {
    newDataReady = false;
    VL53L0X_RangingMeasurementData_t measure;
    lox.rangingTest(&measure, false);

    if (measure.RangeStatus != 4 && measure.RangeMilliMeter <= 500) {
      int currentDist = measure.RangeMilliMeter;
      unsigned long currentTime = millis();
      
      static float filteredDist = 0;
      static float lastFilteredDist = 0;
      static unsigned long lastTime = 0;
      
      // 1. Легкое сглаживание дистанции (чтобы убрать дрожание +-1мм)
      if (filteredDist == 0) filteredDist = currentDist; // Инициализация
      filteredDist = (currentDist * 0.4) + (filteredDist * 0.6);

      // 2. Расчет времени (в секундах)
      float dT = (currentTime - lastTime) / 1000.0;

      if (dT > 0.02) { // Рассчитываем, если прошло хотя бы 20мс
        // 3. Вычисляем мгновенную скорость
        float instantSpeed = (filteredDist - lastFilteredDist) / dT;

        // "Мертвая зона" — если рука движется медленнее 1 см/сек, считаем 0
        if (abs(instantSpeed) < 10) instantSpeed = 0;

        // 4. Сглаживание скорости (Alpha = 0.2 для плавности графика)
        smoothedSpeed = (instantSpeed * 0.2) + (smoothedSpeed * 0.8);

        lastFilteredDist = filteredDist;
        lastTime = currentTime;
      }

      // 5. Обновление массива для графика
      for (int i = 0; i < GRAPH_WIDTH - 1; i++) speedHistory[i] = speedHistory[i + 1];
      
      // Масштаб для графика: центр 45, +/- 1 пиксель на 5 мм/с
      int graphY = 45 - (int)(smoothedSpeed / 5);
      speedHistory[GRAPH_WIDTH - 1] = constrain(graphY, 15, 63);

      // 6. Отрисовка на дисплей
      display.clearDisplay();
      display.setTextColor(SSD1306_WHITE);
      
      // Компактный вывод в верхней строке
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print("D:"); display.print((int)filteredDist); display.print("mm");
      display.setCursor(75, 0);
      display.print("V:"); display.print((int)smoothedSpeed);

      // Рисуем график
      display.drawFastHLine(0, 45, 128, SSD1306_WHITE); // Ось X (0 скорость)
      for (int i = 1; i < GRAPH_WIDTH; i++) {
        display.drawLine(i-1, speedHistory[i-1], i, speedHistory[i], SSD1306_WHITE);
      }
      display.display();
    }

/*
    // Ограничиваем рабочий диапазон 10-500 мм
    if (measure.RangeStatus != 4 && measure.RangeMilliMeter <= 500) {
      int currentDist = measure.RangeMilliMeter;
      unsigned long currentTime = millis();

      // Индекс самого старого значения
      int oldestIdx = readIndex; 
      
      // Расчет скорости по разнице с самым старым значением в буфере (на за 1 шаг, а за 10)
      if (bufferFull) {
        float dD = (float)currentDist - readings[oldestIdx];
        float dT = (float)(currentTime - times[oldestIdx]) / 1000.0;
        
        if (dT > 0) {
          float instantSpeed = dD / dT;
          if (abs(instantSpeed) < 25) instantSpeed = 0; // Игнорируем дрожание
          smoothedSpeed = (instantSpeed * filterAlpha) + (smoothedSpeed * (1.0 - filterAlpha));
        }
      }

      // Обновляем буфер
      readings[readIndex] = currentDist;
      times[readIndex] = currentTime;
      readIndex = (readIndex + 1) % NUM_READINGS;
      if (readIndex == 0) bufferFull = true;

      // Среднее расстояние для вывода
      long sum = 0;
      for (int i = 0; i < NUM_READINGS; i++) sum += readings[i];
      int filteredDist = sum / NUM_READINGS;

      // Сдвиг графика
      for (int i = 0; i < GRAPH_WIDTH - 1; i++) speedHistory[i] = speedHistory[i + 1];
      // Масштабируем: центр 45, +/- 1 пиксель на каждые 4 мм/с
      speedHistory[GRAPH_WIDTH - 1] = 45 - (int)(smoothedSpeed / 4);

      // Отрисовка
      display.clearDisplay();
      display.setTextColor(SSD1306_WHITE);
      
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print("Dist: "); display.print(filteredDist); display.print("mm");
      
      display.setCursor(80, 0);
      display.print("V:"); display.print((int)smoothedSpeed);

      display.drawFastHLine(0, 45, 128, SSD1306_WHITE); // Ось нуля

      for (int i = 1; i < GRAPH_WIDTH; i++) {
        int y0 = constrain(speedHistory[i-1], 12, 63);
        int y1 = constrain(speedHistory[i], 12, 63);
        display.drawLine(i-1, y0, i, y1, SSD1306_WHITE);
      }
      display.display();
    }
*/    
    lox.clearInterruptMask(false);
  }
}
