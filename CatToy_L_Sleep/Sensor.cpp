/*
  ============================================================================
   ФАЙЛ: Sensor.cpp
   НАЗНАЧЕНИЕ: Реализация функций для датчика VL53L0X
  ============================================================================
*/

#include "Sensor.h"
#include "SharedState.h"
#include "Config.h"

// Внешние переменные
extern SharedState gState;
extern SemaphoreHandle_t gStateMutex;
extern volatile bool systemReady;

// Объект датчика
Adafruit_VL53L0X lox;

// ============================================================================
// ИНИЦИАЛИЗАЦИЯ
// ============================================================================
bool initSensor() {
    // Временно отключаем реальный датчик для тестирования
    Serial.println("Датчик VL53L0X ОТКЛЮЧЁН (режим тестирования)");
    
    xSemaphoreTake(gStateMutex, portMAX_DELAY);
    gState.sensorOk = true;      // Принудительно говорим, что датчик "OK"
    gState.catDetected = true;   // Принудительно считаем, что кот есть
    gState.activePlay = true;    // Принудительно включаем активную игру
    gState.lastCatSeenMs = millis();
    xSemaphoreGive(gStateMutex);
    
    return true;  // Возвращаем true, чтобы система думала, что всё нормально
}

// Для включенного сенсора
bool initSensor_() {
    Serial.print("Поиск датчика VL53L0X... ");

    if (!lox.begin()) {
        Serial.println("НЕ НАЙДЕН!");
        Serial.println("Проверьте подключение I2C (SDA=21, SCL=22) и питание датчика.");

        // Устанавливаем флаг ошибки под мьютексом
        xSemaphoreTake(gStateMutex, portMAX_DELAY);
        gState.sensorOk = false;
        xSemaphoreGive(gStateMutex);

        return false;
    }

    Serial.println("ОК");

    // Датчик найден
    xSemaphoreTake(gStateMutex, portMAX_DELAY);
    gState.sensorOk = true;
    gState.lastCatSeenMs = millis();  // Инициализируем таймер
    xSemaphoreGive(gStateMutex);

    return true;
}

// ============================================================================
// ЗАДАЧА ОПРОСА ДАТЧИКА
// ============================================================================

void taskSensor(void* pv) {
    // Ждём готовности системы
    while(!systemReady) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    // Буфер для результата измерения
    VL53L0X_RangingMeasurementData_t measure;

    for(;;) {
        // ========== ВОТ СЮДА ВСТАВИТЬ ПРОВЕРКУ ВРЕМЕНИ ==========
        static unsigned long lastScheduleCheck = 0;
        if(millis() - lastScheduleCheck > 60000) {  // Раз в минуту
            lastScheduleCheck = millis();
            
            if(gState.isAsleep) {
                struct tm timeinfo;
                if(getLocalTime(&timeinfo, 5)) {
                    int currentHour = timeinfo.tm_hour;
                    for(int i = 0; i < WAKEUP_COUNT; i++) {
                        if(currentHour == WAKEUP_HOURS[i]) {
                           // wakeUp();
                            break;
                        }
                    }
                }
            }
        }
        // ======================================================
        
        // ... остальной код опроса датчика (VL53L0X, измерение и т.д.) ...
     
        // Выполняем измерение
        bool ok = false;
        uint16_t mm = 8190;  // 8190 = "вне диапазона" (значение по умолчанию)

        lox.rangingTest(&measure, false);  // false = не ждать сигнала готовности

        // Проверяем статус: 4 означает "out of range" (слишком далеко)
        if(measure.RangeStatus != 4) {
            mm = measure.RangeMilliMeter;
            ok = true;
        }

        // Обновляем глобальное состояние под мьютексом
        xSemaphoreTake(gStateMutex, portMAX_DELAY);

        gState.sensorOk = ok;
        gState.distanceMM = mm;
        // Если игрушка спит и появился кот — будим
        if(gState.isAsleep && ok && mm <= CAT_DETECT_MM) {
           //wakeUp();
        }

        // А если игрушка не спит — обновляем таймер бездействия
        if(!gState.isAsleep && ok && mm <= CAT_DETECT_MM) {
            xSemaphoreTake(gStateMutex, portMAX_DELAY);
            gState.sleepStartMs = millis();  // Сброс таймера сна
            xSemaphoreGive(gStateMutex);
        }
        // Логика обнаружения кота
        if(ok && mm <= CAT_DETECT_MM) {
            gState.catDetected = true;
            gState.lastCatSeenMs = millis();
            gState.activePlay = (mm <= CAT_ACTIVE_MM);
        } else {
            gState.catDetected = false;
            gState.activePlay = false;
        }

        // Проверка таймаута бездействия (переход в энергосбережение)
        uint32_t now = millis();
        gState.powerSave = (now - gState.lastCatSeenMs) > INACTIVE_TIMEOUT_MS;

        xSemaphoreGive(gStateMutex);

        // Пауза до следующего измерения
        vTaskDelay(pdMS_TO_TICKS(SENSOR_PERIOD_MS));
    }
}