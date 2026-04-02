#include "SimpleKalman.h"
#include "Hardware_Utils.h"

// --- Настройки вывода ---
#define USE_SERIAL  true
#define USE_DISPLAY true
unsigned long lastDisplayUpdate = 0;
unsigned long lastMovementTime = 0;

// Интервал обновления экрана (мс). 100мс = 10 кадров в сек.
const int DISPLAY_FPS_MS = 100;

// --- Конфигурация пинов ---
#define SDA_PIN 8        // 41 было
#define SCL_PIN 9        // 42 
#define INT_PIN 18       // 11  21 


// --- Настройки чувствительности ---
const float MIN_DT_INTERVAL  = 0.03;   // Минимальный шаг расчета (33 Гц для скорости)
const float SPEED_THRESHOLD  = 10.0;   // Порог срабатывания скорости (мм/с)
const int   MAX_RANGE_MM     = 400;    // Рабочая зона (мм)
const long  IDLE_TIMEOUT_MS  = 1500;   // Таймаут очистки экрана (мс)
const int   SMOOTH_SAMPLES   = 8;      // Окно усреднения (8 замеров ~ 250 мс истории)

// Объекты
Adafruit_SSD1306 display(128, 64, &Wire, -1);
Adafruit_VL53L0X lox = Adafruit_VL53L0X();
SimpleKalman kalman(2, 2, 0.01); 

// Переменные состояния
volatile bool newData = false;
void IRAM_ATTR onDataReady() { newData = true; }

unsigned long lastTime = 0;
int lastDist = 0;

// Буфер для скользящего среднего скорости
float speedBuffer[SMOOTH_SAMPLES];
int speedIndex = 0;

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  // Очистка буфера скорости
  for(int i=0; i<SMOOTH_SAMPLES; i++) speedBuffer[i] = 0;

  initHardware(SDA_PIN, SCL_PIN, INT_PIN, onDataReady);
  Serial.println("[SYSTEM READY - MOVING AVERAGE MODE]");
  lastTime = millis();
}

void loop() {
  // 1. Проверяем наличие новых данных от датчика (по прерыванию)
  if (newData) {
    newData = false;
    VL53L0X_RangingMeasurementData_t measure;
    lox.getRangingMeasurement(&measure, false);
    
    // Статус 0 — замер корректен
    if (measure.RangeStatus == 0) {
      // Фильтруем дистанцию Калманом
      int filteredDist = (int)kalman.update(measure.RangeMilliMeter);
      
      unsigned long now = millis();
      float dt = (now - lastTime) / 1000.0;
      
      // Считаем скорость только если прошел заданный интервал (0.03 сек)
      if (dt >= MIN_DT_INTERVAL) {
        
        // Мгновенная скорость (мм/с)
        float instantSpeed = (float)(lastDist - filteredDist) / dt;

        // Записываем в буфер для усреднения
        speedBuffer[speedIndex] = instantSpeed;
        speedIndex = (speedIndex + 1) % SMOOTH_SAMPLES;

        // Вычисляем среднюю скорость (скользящее среднее)
        float avgSpeed = 0;
        for(int i=0; i<SMOOTH_SAMPLES; i++) avgSpeed += speedBuffer[i];
        avgSpeed /= (float)SMOOTH_SAMPLES;

        // ЛОГИКА СОБЫТИЯ: Дистанция 0-400 мм И только приближение (+)
        if (filteredDist > 0 && filteredDist <= MAX_RANGE_MM && avgSpeed > SPEED_THRESHOLD) {
          
          lastMovementTime = now; // Обновляем таймер активности

          // ВЫВОД В PORT (без ограничений по FPS)
          if (USE_SERIAL) {
            Serial.printf("APPROACH: %d mm | Speed: %.1f mm/s\n", filteredDist, avgSpeed);
          }

          // ВЫВОД НА ДИСПЛЕЙ (с ограничением FPS, чтобы не вешать I2C)
          if (USE_DISPLAY && (now - lastDisplayUpdate >= DISPLAY_FPS_MS)) {
            updateOLED(filteredDist, avgSpeed);
            lastDisplayUpdate = now;
          }
        }
        
        // Обновляем базу для следующего шага
        lastDist = filteredDist;
        lastTime = now;
      }
    }
    // Разрешаем датчику следующее прерывание
    lox.clearInterruptMask(false);
  }

  // 2. Очистка экрана при бездействии (Таймаут)
  unsigned long currentTime = millis();
  if (USE_DISPLAY && (currentTime - lastMovementTime > IDLE_TIMEOUT_MS)) {
    // Очищаем экран тоже не чаще, чем DISPLAY_FPS_MS
    if (currentTime - lastDisplayUpdate >= DISPLAY_FPS_MS) {
      display.clearDisplay();
      display.display();
      lastDisplayUpdate = currentTime;
      
      // Сброс буфера скорости, чтобы расчет не "прыгал" при новом появлении объекта
      for(int i=0; i<SMOOTH_SAMPLES; i++) speedBuffer[i] = 0;
    }
  }
}

