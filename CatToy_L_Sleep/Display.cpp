/*
  ============================================================================
   ФАЙЛ: Display.cpp
   НАЗНАЧЕНИЕ: Реализация функций для OLED дисплея
  ============================================================================
*/

#include "Display.h"
#include "SharedState.h"
#include "Config.h"

// Внешние переменные
extern SharedState gState;
extern SemaphoreHandle_t gStateMutex;
extern volatile bool systemReady;

// Константы для дисплея (не вынесены в Config.h, т.к. специфичны для SSD1306)
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Объект дисплея
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &SPI, OLED_DC, OLED_RESET, OLED_CS);

// ============================================================================
// ИНИЦИАЛИЗАЦИЯ ДИСПЛЕЯ
// ============================================================================

void initDisplay() {
    Serial.print("Инициализация OLED SSD1306 (SPI)... ");

    // Инициализация SPI для дисплея
    SPI.begin(OLED_CLK, -1, OLED_MOSI, OLED_CS);

    // Инициализация дисплея
    if (!display.begin(SSD1306_SWITCHCAPVCC)) {
        Serial.println("ОШИБКА!");
        Serial.println("Проверьте подключение: MOSI=23, CLK=18, DC=2, CS=5, RST=4");
        return;
    }

    Serial.println("ОК");

    // Очищаем экран и выводим приветствие
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("OLED Ready");
    display.display();

    delay(1000);
}

// ============================================================================
// ЗАДАЧА ОБНОВЛЕНИЯ ДИСПЛЕЯ
// ============================================================================

void taskUI(void* pv) {
    // Ждём готовности системы
    while(!systemReady) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    for(;;) {
        // Копируем текущее состояние под мьютексом
        SharedState st;
        xSemaphoreTake(gStateMutex, portMAX_DELAY);
        st = gState;
        xSemaphoreGive(gStateMutex);

        // Очищаем экран
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);

        // Строка 0: Название проекта
        display.setCursor(0, 0);
        display.println("Smart Cat Toy v3.0");

        // Строка 12: Статус датчика
        display.setCursor(0, 12);
        display.print("Sensor: ");
        display.println(st.sensorOk ? "OK" : "ERR");

        // Строка 22: Расстояние
        display.setCursor(0, 22);
        display.print("Dist(mm): ");
        if(st.sensorOk) {
            display.println(st.distanceMM);
        } else {
            display.println("---");
        }

        // Строка 32: Обнаружение кота
        display.setCursor(0, 32);
        display.print("Cat: ");
        display.println(st.catDetected ? "YES" : "NO");

        // Строка 42: Режим работы
        display.setCursor(0, 42);
        display.print("Mode: ");
        if(st.mode == MODE_CALIB) {
            display.println("CALIB");
        } else if(st.powerSave) {
            display.println("POWER SAVE");
        } else if(st.activePlay) {
            display.println("ACTIVE PLAY");
        } else if(st.catDetected) {
            display.println("CALM PLAY");
        } else {
            display.println("SEARCH");
        }

        // Строка 54: Лазер, TILT угол, PAN позиция
        display.setCursor(0, 54);
        display.print("L:");
        display.print(st.laserOn ? "ON " : "OFF");
        display.print(" T:");
        display.print(st.tiltDeg);
        display.print(" H:");
        display.print(st.panRelDeg);

        // Отправляем на экран
        display.display();

        // Пауза для экономии ресурсов
        vTaskDelay(pdMS_TO_TICKS(UI_PERIOD_MS));
    }
}