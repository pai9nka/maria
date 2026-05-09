/*
  ============================================================================
   ПРОЕКТ: Умная игрушка для кота (ESP32)
   АВТОР: на основе оригинального кода с доработками
   ВЕРСИЯ: 3.0 - многомодульная структура
   ДАТА: 2026-05-07
  ============================================================================
   
   КРАТКОЕ ОПИСАНИЕ:
   =================
   Устройство привлекает кота движущимся лучом лазера и звуками.
   Когда кот приближается — игра становится активнее.
   При отсутствии кота — переходит в энергосберегающий режим.
   
   АППАРАТНАЯ ЧАСТЬ:
   =================
   - ESP32-WROOM-32
   - Датчик расстояния VL53L0X (I2C: SDA=21, SCL=22)
   - OLED дисплей SSD1306 (SPI: MOSI=23, CLK=18, DC=2, CS=5, RST=4)
   - Серво PAN (горизонталь) SG90 360° (непрерывное вращение) - пин 19
   - Серво TILT (вертикаль) SG90 180° (позиционное) - пин 27
   - Лазерный модуль (управление через транзистор) - пин 26
   - Динамик/пищалка (через резистор 1 кОм) - пин 25
   
   ЛОГИКА РАБОТЫ:
   ==============
   1. Датчик VL53L0X измеряет расстояние до ближайшего объекта.
   2. Если объект ближе 1200 мм — считаем, что кот в зоне.
   3. Если ближе 900 мм — активная игра.
   4. Если кота нет более 60 секунд — режим экономии энергии.
   
   FreeRTOS ЗАДАЧИ:
   ================
   - SensorTask  (ядро 1) — опрос датчика расстояния
   - MotionTask  (ядро 1) — управление сервоприводами и лазером
   - SoundTask   (ядро 0) — воспроизведение звуков
   - UITask      (ядро 0) — обновление OLED дисплея
   
   СЕРИЙНЫЕ КОМАНДЫ (115200 бод):
   ===============================
   help, show, save
   mode run/calib
   set pan_delta <60..220>
   set tilt_home <45..120>
   tilt_range <min> <max>
   pan_speed slow/normal/fast
   sound on/off
   test sound bird/mouse
   
  ============================================================================
*/

// Подключение заголовочных файлов проекта
#include "Config.h"           // Пины и базовые константы
#include "SharedState.h"      // Разделяемые структуры данных
#include "Sound.h"            // Звуковые эффекты
#include "Motion.h"           // Управление сервоприводами
#include "Sensor.h"           // Датчик VL53L0X
#include "Display.h"          // OLED дисплей
#include "SerialCommands.h"   // Обработка команд
#include "PreferencesStorage.h" // Сохранение настроек

// ============================================================================
// ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ (определены здесь, объявлены как extern в .h файлах)
// ============================================================================
RuntimeConfig gCfg;           // Текущая конфигурация (настройки)
SharedState gState;           // Текущее состояние системы
SemaphoreHandle_t gStateMutex; // Мьютекс для доступа к разделяемым данным
//Preferences gPrefs;            // Хранилище настроек в flash
String gSerialLine;            // Буфер для ввода с Serial
volatile bool systemReady = false; // Флаг готовности системы (старт задач)
int lastTiltPos = TILT_HOME_DEG_DEF; // Последняя позиция TILT (для плавности)


// Объявите глобальные переменные ДО setup()

TaskHandle_t taskMotionHandle = NULL;
TaskHandle_t taskSoundHandle = NULL;
TaskHandle_t taskUIHandle = NULL;
TaskHandle_t taskSensorHandle = NULL;


// ============================================================================
// ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ (обёртки для доступа к конфигурации)
// ============================================================================

// ============================================================================
// ФУНКЦИЯ SETUP() — ИНИЦИАЛИЗАЦИЯ
// ============================================================================

void setup() {
    // Инициализация последовательного порта для отладки
    Serial.begin(115200);
    delay(300);  // Даём время на открытие Serial Monitor
    Serial.println("\n\n=== УМНАЯ ИГРУШКА ДЛЯ КОТА v3.0 ===");
    
    // ------------------------------------------------------------------------
    // 1. СОЗДАНИЕ МЬЮТЕКСА (для синхронизации доступа к данным)
    // ------------------------------------------------------------------------
    gStateMutex = xSemaphoreCreateMutex();
    if (!gStateMutex) {
        Serial.println("КРИТИЧЕСКАЯ ОШИБКА: не удалось создать мьютекс!");
        while (true) delay(1000);
    }
    
    // ------------------------------------------------------------------------
    // 2. ЗАГРУЗКА НАСТРОЕК ИЗ FLASH (Preferences)
    // ------------------------------------------------------------------------
    initPreferences();  // Функция из PreferencesStorage.cpp
    // После initPreferences() для управления сном
    xSemaphoreTake(gStateMutex, portMAX_DELAY);
    gState.isAsleep = false;
    gState.sleepStartMs = 0;
    xSemaphoreGive(gStateMutex);
    
    // ------------------------------------------------------------------------
    // 3. ИНИЦИАЛИЗАЦИЯ GPIO (лазер, звук)
    // ------------------------------------------------------------------------
    pinMode(LASER_CTRL_PIN, OUTPUT);
    digitalWrite(LASER_CTRL_PIN, LOW);
    
    initSound();  // Настройка LEDC для динамика (из Sound.cpp)
    
    // ------------------------------------------------------------------------
    // 4. ИНИЦИАЛИЗАЦИЯ ДАТЧИКА РАССТОЯНИЯ VL53L0X
    // ------------------------------------------------------------------------
    initSensor();  // Из Sensor.cpp    ++++++++++++++++++++++++++++++++++++++++++++++++++
    
    // ------------------------------------------------------------------------
    // 5. ИНИЦИАЛИЗАЦИЯ OLED ДИСПЛЕЯ
    // ------------------------------------------------------------------------
    initDisplay(); // Из Display.cpp
    
    // ------------------------------------------------------------------------
    // 6. ИНИЦИАЛИЗАЦИЯ СЕРВОПРИВОДОВ
    // ------------------------------------------------------------------------
    initServos();  // Из Motion.cpp
    
    // ------------------------------------------------------------------------
    // 7. СОЗДАНИЕ ЗАДАЧ FREERTOS
    // ------------------------------------------------------------------------
    // Задача датчика (высокий приоритет 2, ядро 1)
    //xTaskCreatePinnedToCore(taskSensor, "SensorTask", 4096, nullptr, 2, nullptr, 1);
    
    

    // В setup() создавайте с хэндлами:
    xTaskCreatePinnedToCore(taskMotion, "MotionTask", 4096, nullptr, 2, &taskMotionHandle, 1);
    xTaskCreatePinnedToCore(taskSound, "SoundTask", 3072, nullptr, 1, &taskSoundHandle, 0);
    xTaskCreatePinnedToCore(taskUI, "UITask", 4096, nullptr, 1, &taskUIHandle, 0);
    
    //xTaskCreatePinnedToCore(taskSensor, "SensorTask", 4096, nullptr, 2, &taskSensorHandle, 1);
    /*    
    // Задача управления движением (высокий приоритет 2, ядро 1)
    xTaskCreatePinnedToCore(taskMotion, "MotionTask", 4096, nullptr, 2, nullptr, 1);
    
    // Задача звука (средний приоритет 1, ядро 0)
    xTaskCreatePinnedToCore(taskSound, "SoundTask", 3072, nullptr, 1, nullptr, 0);
    
    // Задача пользовательского интерфейса (низкий приоритет 1, ядро 0)
    xTaskCreatePinnedToCore(taskUI, "UITask", 4096, nullptr, 1, nullptr, 0);
    */

        // Проверяем, проснулись ли мы после Deep Sleep
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    
    if(wakeup_reason == ESP_SLEEP_WAKEUP_TIMER) {
        Serial.println("Пробуждение после Deep Sleep (12 часов прошло)");
        
        // Начинаем новый сеанс
        xSemaphoreTake(gStateMutex, portMAX_DELAY);
        gState.sessionStartMs = millis();
        gState.sessionActive = true;
        gState.catDetected = true;   // Принудительно для нового сеанса
        gState.activePlay = true;
        gState.mode = MODE_RUN;
        gState.powerSave = false;
        xSemaphoreGive(gStateMutex);
        
        Serial.println("Новый сеанс начат! (120 секунд)");
    } else {
        Serial.println("Обычный запуск (подано питание или сброс)");
        
        // Первый сеанс
        xSemaphoreTake(gStateMutex, portMAX_DELAY);
        gState.sessionStartMs = millis();
        gState.sessionActive = true;
        gState.catDetected = true;
        gState.activePlay = true;
        gState.mode = MODE_RUN;
        gState.powerSave = false;
        xSemaphoreGive(gStateMutex);
        
        Serial.println("Первый сеанс игры начат (120 секунд)");
    }
    
    // ------------------------------------------------------------------------
    // 8. ЗАПУСК СИСТЕМЫ
    // ------------------------------------------------------------------------
    systemReady = true;  // Все задачи теперь могут работать
    
    // Запускаем первый сеанс сразу
    xSemaphoreTake(gStateMutex, portMAX_DELAY);
    gState.sessionStartMs = millis();
    gState.sessionActive = true;
    gState.catDetected = true;   // Принудительно для первого сеанса
    gState.activePlay = true;    // Сразу активная игра
    gState.mode = MODE_RUN;
    gState.powerSave = false;
    xSemaphoreGive(gStateMutex);
    
    Serial.println("Первый сеанс игры начат (120 секунд)");

    //systemReady = false;         //  Все задачи теперь НЕ могут работать
    Serial.println("Система готова! Введите 'help' для списка команд.");
    printConfig();       // Показать текущие настройки (из SerialCommands.cpp)
    
}

void enterIdleMode() {
    Serial.println("Переход в режим сна...");
    
    // Выключаем лазер
    digitalWrite(LASER_CTRL_PIN, LOW);
    
    // Отключаем серво
    servoPan.detach();
    servoTilt.detach();
    
    // Выключаем звук
    ledcWriteTone(SPEAKER_PIN, 0);
    
    // Останавливаем задачи (кроме taskSensor — он нужен для пробуждения)
    vTaskSuspend(taskMotionHandle);
    vTaskSuspend(taskSoundHandle);
    vTaskSuspend(taskUIHandle);
    
    //vTaskSuspend(taskMotion);
    //vTaskSuspend(taskSound);
    // taskUI можно оставить, но лучше suspend
    //vTaskSuspend(taskUI);
    
    xSemaphoreTake(gStateMutex, portMAX_DELAY);
    gState.isAsleep = true;
    gState.laserOn = false;
    gState.sleepStartMs = millis();
    xSemaphoreGive(gStateMutex);
    
    Serial.println("Игрушка спит. Жду кота или расписания...");
}

void wakeUp() {
    Serial.println("Пробуждение!");
    
    // Переинициализируем серво
    servoPan.attach(SERVO_PAN_PIN, 500, 2400);
    servoTilt.attach(SERVO_TILT_PIN, 500, 2400);
    servoPan.write(90);
    servoTilt.write(TILT_HOME_DEG_DEF);
    
    // Возобновляем задачи
    vTaskResume(taskMotionHandle);
    vTaskResume(taskSoundHandle);
    vTaskResume(taskUIHandle);
    
    //vTaskResume(taskMotion);
    //vTaskResume(taskSound);
    //vTaskResume(taskUI);
    
    xSemaphoreTake(gStateMutex, portMAX_DELAY);
    gState.isAsleep = false;
    gState.sleepStartMs = 0;
    xSemaphoreGive(gStateMutex);
}
// ============================================================================
// ФУНКЦИЯ LOOP() — НЕ ИСПОЛЬЗУЕТСЯ (вся логика в задачах FreeRTOS)
// ============================================================================
void loop() {
    // Только обрабатываем последовательные команды из Serial Monitor
    processSerial();  // Из SerialCommands.cpp
    
    // Небольшая задержка, чтобы не нагружать CPU
    vTaskDelay(pdMS_TO_TICKS(30));

}