/*
ОК = ПАИ 31.03.2026
*/


#include <Wire.h>
#include <Adafruit_VL53L0X.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Пины подключения
#define I2C_SDA 8
#define I2C_SCL 9
#define INTERRUPT_PIN 18

// Инициализация дисплея (128x64 пикселей)
Adafruit_SSD1306 display(128, 64, &Wire, -1);
// Инициализация датчика расстояния VL53L0X
Adafruit_VL53L0X lox = Adafruit_VL53L0X();

// Флаг для обозначения новых данных от датчика
volatile bool newData = false;
// Обработчик прерывания — устанавливает флаг при срабатывании
void IRAM_ATTR isr() { newData = true; }

// Переменные для расчёта скорости
unsigned long lastTime = 0;        // Время предыдущего измерения (в мс)
int lastDistance = 0;               // Предыдущее измеренное расстояние (в мм)
float speed = 0;                   // Текущая скорость (в мм/с)

// Параметры фильтрации скорости
const int FILTER_SIZE = 5;         // Размер буфера для скользящего среднего
float speedBuffer[FILTER_SIZE] = {0}; // Буфер для хранения предыдущих значений скорости
int bufferIndex = 0;              // Текущий индекс в буфере

// Функция фильтрации скорости — скользящее среднее для сглаживания показаний
float filterSpeed(float newSpeed) {
  speedBuffer[bufferIndex] = newSpeed;
  bufferIndex = (bufferIndex + 1) % FILTER_SIZE;

  float sum = 0;
  for (int i = 0; i < FILTER_SIZE; i++) {
    sum += speedBuffer[i];
  }
  return sum / FILTER_SIZE;
}

void setup() {
  Serial.begin(115200);
  delay(2000); 
  Serial.println("\n[SYSTEM READY]");

  // Настройка пина прерывания с подтяжкой
  pinMode(INTERRUPT_PIN, INPUT_PULLUP); 

  // Инициализация I²C на заданных пинах с частотой 100 кГц
  Wire.begin(I2C_SDA, I2C_SCL, 100000);

  // Инициализация OLED‑дисплея
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    for (;;); // Остановка программы при ошибке инициализации дисплея
  } else {
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.display();
  }

  // Инициализация датчика VL53L0X
  if (lox.begin()) {
    // Настройка прерывания для датчика
    attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), isr, FALLING);

    // Конфигурация GPIO датчика: непрерывное измерение с сигналом готовности
    lox.setGpioConfig(VL53L0X_DEVICEMODE_CONTINUOUS_RANGING,
                      VL53L0X_GPIOFUNCTIONALITY_NEW_MEASURE_READY,
                      VL53L0X_INTERRUPTPOLARITY_LOW);

    // Запуск непрерывного режима измерений
    lox.startRangeContinuous();

    // Получение начального измерения для корректного расчёта скорости
    VL53L0X_RangingMeasurementData_t initialMeasure;
    lox.getRangingMeasurement(&initialMeasure, false);
    if (initialMeasure.RangeStatus == 0) {
      lastDistance = initialMeasure.RangeMilliMeter;
      lastTime = millis(); // Установка времени первого измерения
    } else {
      Serial.println("Initial distance measurement failed");
      lastDistance = 0; // Резервное значение
    }

    Serial.println("[VL53L0X READY]");
  } else {
    Serial.println("[ERROR] VL53L0X not found");
  }
}

void loop() {
  // Проверка флага новых данных от датчика
  if (newData) {
    newData = false; // Сбрасываем флаг сразу после проверки

    VL53L0X_RangingMeasurementData_t measure;
    lox.getRangingMeasurement(&measure, false);

    // Проверка статуса измерения (0 = Valid, 4 = Out of range)
    if (measure.RangeStatus != 4) {
      unsigned long currentTime = millis();
      int currentDistance = measure.RangeMilliMeter;

      // Расчёт времени между измерениями в секундах
      float dt = (currentTime - lastTime) / 1000.0;

      // Защита от дребезга времени и слишком частых измерений
      if (dt > 0.02) {
        // Расчёт сырой скорости: (предыдущее расстояние − текущее) / время
        float rawSpeed = (float)(lastDistance - currentDistance) / dt;
        // Фильтрация скорости через скользящее среднее
        speed = filterSpeed(rawSpeed);

        // Вывод данных в Serial Monitor для отладки
        Serial.printf("D: %d mm | V: %.1f mm/s\n", currentDistance, speed);

        // Очистка дисплея перед новым отображением
        display.clearDisplay();

        // Вывод дистанции на дисплей
        display.setTextSize(1);
        display.setCursor(0, 0);
        display.print("DISTANCE:");
        display.setTextSize(2);
        display.setCursor(0, 12);
        display.print(currentDistance);
        display.print(" mm");

        // Вывод скорости на дисплей
        display.setTextSize(1);
        display.setCursor(0, 36);
        display.print("SPEED (mm/s):");
        display.setTextSize(2);
        display.setCursor(0, 48);
        // Фильтр малых значений скорости (< 15 мм/с → 0) для устранения шума
        if (abs(speed) < 15) speed = 0;
        display.print((int)speed);
        display.display(); // Обновление содержимого дисплея

        // Обновление переменных для следующего цикла
        lastDistance = currentDistance;
        lastTime = currentTime;
      }
    } else {
      // Обработка случая «вне диапазона»
      Serial.println("Out of range");
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print("OUT OF RANGE");
      display.display();
    }

    // Очистка маски прерываний датчика для корректной работы следующего цикла
    lox.clearInterruptMask(false);
  }

  // Неблокирующая задержка — позволяет системе оставаться отзывчивой
  static unsigned long lastLoopTime = 0;
  if (millis() - lastLoopTime >= 10) {
    lastLoopTime = millis();
    // Здесь можно добавить дополнительные некритичные операции
  }
}