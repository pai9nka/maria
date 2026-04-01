/*
ОК = ПАИ 31ю03ю2026
*/

#include <Wire.h>
#include <Adafruit_VL53L0X.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define I2C_SDA 8
#define I2C_SCL 9
#define INTERRUPT_PIN 18

Adafruit_SSD1306 display(128, 64, &Wire, -1);
Adafruit_VL53L0X lox = Adafruit_VL53L0X();

volatile bool newData = false;
void IRAM_ATTR isr() { newData = true; }

// Переменные для расчета скорости
unsigned long lastTime = 0;
int lastDistance = 0;
float speed = 0;

unsigned long lastLoopTime = 0;
const unsigned long LOOP_INTERVAL = 10;

void setup() {
  Serial.begin(115200);
  delay(2000); 
  Serial.println("\n[SYSTEM READY]");

  pinMode(INTERRUPT_PIN, INPUT_PULLUP); 
  Wire.begin(I2C_SDA, I2C_SCL, 100000);

  if(display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.display();
  }

  if (lox.begin()) {
    attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), isr, FALLING);
    lox.setGpioConfig(VL53L0X_DEVICEMODE_CONTINUOUS_RANGING, 
                      VL53L0X_GPIOFUNCTIONALITY_NEW_MEASURE_READY, 
                      VL53L0X_INTERRUPTPOLARITY_LOW);
    lox.startRangeContinuous();
    lastTime = millis();
  }
}

void loop() {
  unsigned long currentTime = millis();
  if (currentTime - lastLoopTime >= LOOP_INTERVAL) {
    lastLoopTime = currentTime;

  if (newData) {
    newData = false;
    VL53L0X_RangingMeasurementData_t measure;
    lox.getRangingMeasurement(&measure, false);
    
    if (measure.RangeStatus != 4) { // 0 = Valid, 4 = Out of range
       unsigned long currentTime = millis();
       int currentDistance = measure.RangeMilliMeter;
       
       // Расчет времени в секундах
       float dt = (currentTime - lastTime) / 1000.0;
       
       if (dt > 0.02) { // Защита от дребезга времени  совет =0.02
         // Скорость: (разница расстояний) / время. 
         // Положительная = приближается, отрицательная = удаляется.
         speed = (float)(lastDistance - currentDistance) / dt;
         
         // Вывод в Serial для отладки
         Serial.printf("D: %d mm | V: %.1f mm/s\n", currentDistance, speed);
         
         // Вывод на дисплей
         display.clearDisplay();
         
         // Дистанция
         display.setTextSize(1);
         display.setCursor(0,0);
         display.print("DISTANCE:");
         display.setTextSize(2);
         display.setCursor(0,12);
         display.print(currentDistance);
         display.print(" mm");

         // Скорость
         display.setTextSize(1);
         display.setCursor(0,36);
         display.print("SPEED (mm/s):");
         display.setTextSize(2);
         display.setCursor(0,48);
         // Небольшой фильтр, чтобы не мельтешили мелкие значения
         if (abs(speed) < 15) speed = 0; 
         display.print((int)speed);
         
         display.display();

         lastDistance = currentDistance;
         lastTime = currentTime;
       }
    }
    lox.clearInterruptMask(false);
  }
   // Неблокирующая задержка
  static unsigned long lastLoopTime = 0;
  if (millis() - lastLoopTime >= 10) {
    lastLoopTime = millis();
  }
  }
  //delay(10); 
}