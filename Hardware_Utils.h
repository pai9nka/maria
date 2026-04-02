#ifndef HARDWARE_UTILS_H
#define HARDWARE_UTILS_H

#include <Wire.h>
#include <Adafruit_VL53L0X.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Объявляем, что эти объекты существуют где-то еще (в .ino файле)
extern Adafruit_SSD1306 display;
extern Adafruit_VL53L0X lox;

// Инициализация
void initHardware(int sda, int scl, int int_pin, void (*isr_func)()) {
  Wire.begin(sda, scl, 100000);
  delay(100); // Даем время шине стабилизироваться

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED Fail");
  }
  display.clearDisplay();
  display.display();

  delay(100); // Пауза перед инициализацией датчика
  if (!lox.begin()) {
    Serial.println("VL53L0X Fail");
    // Вместо бесконечного цикла, попробуем еще раз через секунду
    delay(1000);
    if (!lox.begin()) while(1); 
  }

  pinMode(int_pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(int_pin), isr_func, FALLING);
  
  lox.setGpioConfig(VL53L0X_DEVICEMODE_CONTINUOUS_RANGING, 
                    VL53L0X_GPIOFUNCTIONALITY_NEW_MEASURE_READY, 
                    VL53L0X_INTERRUPTPOLARITY_LOW);
  lox.startRangeContinuous();
}

// Вывод на дисплей
void updateOLED(int dist, float spd) {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1); display.setCursor(0,0); 
  display.print("TARGET IN RANGE:");
  
  display.setTextSize(2); display.setCursor(0,12); 
  display.print(dist); display.print(" mm");
  
  display.setTextSize(1); display.setCursor(0,36); 
  display.print("APPROACH SPEED:");
  
  display.setTextSize(2); display.setCursor(0,48); 
  display.print((int)spd); display.print(" mm/s");
  display.display();
}

#endif
