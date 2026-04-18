#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Adafruit_VL53L0X.h"

// --- НАСТРОЙКИ ПИНОВ ---
#define PIN_LED         32    // Внешний светодиод
#define PIN_SPEAKER     27    // Динамик
#define SENSOR_INT_PIN  25    // Прерывание датчика (GPIO1/INT)

// --- ПАРАМЕТРЫ ЛОГИКИ ---
const int   DIST_THRESHOLD = 400;   // Порог активации (мм)
const long  LED_HOLD_TIME  = 7000;  // Удержание LED (мс)
const int   BEEP_DURATION  = 30;    // Длительность одного "пика" (мс)

// Настройки дисплея SPI
#define OLED_MOSI 23       
#define OLED_CLK  18
#define OLED_DC   2
#define OLED_CS   5
#define OLED_RESET 4 

Adafruit_SSD1306 display(128, 64, OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);
Adafruit_VL53L0X lox = Adafruit_VL53L0X();

// Глобальные переменные
volatile bool newDataReady = false;
void IRAM_ATTR isr_vl53l0x() { newDataReady = true; }

unsigned long ledOffTime   = 0;
unsigned long soundOffTime = 0;
unsigned long lastBeepTime = 0;
bool ledState = false;
float smoothedSpeed = 0;
int speedHistory[128];

void setup() {
  Serial.begin(115200);
  
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, HIGH);

  // Настройка звука через LEDC (аппаратный ШИМ ESP32) для старых библиотек ESP32
  //ledcSetup(0, 1000, 10);        // Канал 0, частота 1кГц, разрешение 10 бит
  //ledcAttachPin(PIN_SPEAKER, 0); // Привязка пина к каналу 0

  // для новых
  ledcAttach(PIN_SPEAKER, 1000, 10);
  
  if(!display.begin(SSD1306_SWITCHCAPVCC)) while(1);
  display.clearDisplay();
  display.display();

  if(!lox.begin()) while(1);

  pinMode(SENSOR_INT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(SENSOR_INT_PIN), isr_vl53l0x, FALLING);

  lox.setGpioConfig(VL53L0X_DEVICEMODE_CONTINUOUS_RANGING, 
                    VL53L0X_GPIOFUNCTIONALITY_NEW_MEASURE_READY, 
                    VL53L0X_INTERRUPTPOLARITY_LOW);
  lox.startRangeContinuous();
  
  for(int i=0; i<128; i++) speedHistory[i] = 45;
}

void loop() {
  unsigned long now = millis();

  // 1. НЕБЛОКИРУЮЩЕЕ УПРАВЛЕНИЕ (Таймеры)
  // Выключение звука по времени
  if (now > soundOffTime) {
    ledcWriteTone(PIN_SPEAKER, 0); // Первый параметр - пин, второй - 0 (тишина)
  }  

  // Выключение светодиода по времени
  if (ledState && (now > ledOffTime)) {
    ledState = false;
    digitalWrite(PIN_LED, HIGH);
  }

  // 2. ОБРАБОТКА ДАННЫХ ДАТЧИКА
  if (newDataReady) {
    newDataReady = false;
    VL53L0X_RangingMeasurementData_t measure;
    lox.rangingTest(&measure, false);

    int d = measure.RangeMilliMeter;

    // Работаем только с валидными данными до 1 метра
    if (measure.RangeStatus == 0 && d > 5 && d < 1000) {
      
      // Логика Светодиода
      if (d < DIST_THRESHOLD) {
        ledState = true;
        ledOffTime = now + LED_HOLD_TIME;
        digitalWrite(PIN_LED, LOW);
      }

      // Расчет скорости
      static float lastD = 0;
      static unsigned long lastT = 0;
      float dT = (now - lastT) / 1000.0;
      if (dT > 0.02) {
        float instantV = (float)(d - lastD) / dT;
        if (abs(instantV) > 1000) instantV = 0; 
        if (abs(instantV) < 15)   instantV = 0;
        smoothedSpeed = (instantV * 0.2) + (smoothedSpeed * 0.8);
        lastD = (float)d; lastT = now;
      }

      // Логика Звука (без delay)
      if (d < 500) { // Пищим только ближе 50 см
        // Интервал между бипами (дистанция)
        int beepInterval = map(constrain(d, 20, 500), 20, 500, 60, 600);
        
        if (now - lastBeepTime > beepInterval) {
          // Тон бипа (скорость)
          int toneFreq = map(constrain(abs((int)smoothedSpeed), 0, 500), 0, 500, 400, 1600);
  
          ledcWriteTone(PIN_SPEAKER, toneFreq); // Первый - пин, второй - частота
          soundOffTime = now + BEEP_DURATION;
          lastBeepTime = now;
        }
      }
    }

    // 3. ОТРИСОВКА ЭКРАНА
    renderScreen(d, (int)smoothedSpeed, ledState);
    
    lox.clearInterruptMask(false);
  }
}

void renderScreen(int d, int v, bool alarm) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("D: "); display.print(d); display.print(" mm");
  display.setCursor(80, 0);
  display.print("V: "); display.print(v);

  if(alarm) {
    display.setCursor(45, 12);
    display.print("[ ALARM ]");
  }

  // Обновление графика
  for (int i = 0; i < 127; i++) speedHistory[i] = speedHistory[i+1];
  speedHistory[127] = constrain(45 - (v / 6), 25, 63);

  display.drawFastHLine(0, 45, 128, SSD1306_WHITE); // Линия нуля
  for (int i = 1; i < 128; i++) {
    display.drawLine(i-1, speedHistory[i-1], i, speedHistory[i], SSD1306_WHITE);
  }
  display.display();
}
