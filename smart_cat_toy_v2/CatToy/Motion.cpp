/*
  ============================================================================
   ФАЙЛ: Motion.cpp
   НАЗНАЧЕНИЕ: Управление двумя сервоприводами SG90 180° (позиционные)
   ВЕРСИЯ: Для горизонтального и вертикального позиционирования лазера
  ============================================================================
*/

#include "Motion.h"
#include "SharedState.h"
#include "Config.h"
#include "PreferencesStorage.h"   // для saveConfig()
#include "Display.h"              // для display
#include "Sound.h"

// Внешние переменные из CatToy.ino
extern TaskHandle_t taskMotionHandle;
extern TaskHandle_t taskSoundHandle;
extern TaskHandle_t taskUIHandle;

// Внешние переменные (определены в CatToy.ino)
extern RuntimeConfig gCfg;
extern SharedState gState;
extern SemaphoreHandle_t gStateMutex;
extern volatile bool systemReady;
extern int lastTiltPos;

// Объекты сервоприводов
Servo servoPan;
Servo servoTilt;

// ============================================================================
// ИНИЦИАЛИЗАЦИЯ
// ============================================================================

void initServos() {
    // Частота 50 Гц (стандарт для SG90)
    servoPan.setPeriodHertz(50);
    servoTilt.setPeriodHertz(50);
    
    // Диапазон импульсов: 500..2400 мкс (типично для SG90)
    servoPan.attach(SERVO_PAN_PIN, 500, 2400);
    servoTilt.attach(SERVO_TILT_PIN, 500, 2400);
    
    // Устанавливаем начальные положения
    servoPan.write(90);           // PAN в центр (90° из 180)
    servoTilt.write(TILT_HOME_DEG_DEF);
    
    // Сохраняем состояние
    lastTiltPos = TILT_HOME_DEG_DEF;
    
    delay(400);
    Serial.println("Сервоприводы (оба 180°) инициализированы");
}

// ============================================================================
// УПРАВЛЕНИЕ ЛАЗЕРОМ
// ============================================================================

void setLaser(bool on) {
    digitalWrite(LASER_CTRL_PIN, on ? HIGH : LOW);
    
    xSemaphoreTake(gStateMutex, portMAX_DELAY);
    gState.laserOn = on;
    xSemaphoreGive(gStateMutex);
}

// ============================================================================
// ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ
// ============================================================================

/**
 * @brief Генератор случайного числа
 */
static int randomRange(int min, int max) {
    return min + random(max - min + 1);
}

/**
 * @brief Плавное перемещение PAN (горизонталь) в целевой угол
 * @param targetDeg угол в градусах (0..180)
 * @param speed скорость движения (градусов за шаг)
 */
void panMoveTo(int targetDeg, int speed) {
    // Ограничиваем угол (0-180 для SG90)
    targetDeg = constrain(targetDeg, 0, 180);
    
    // Локальная статическая переменная для запоминания текущей позиции
    static int currentDeg = 90;
    
    // Если позиция не изменилась — не двигаем
    if (targetDeg == currentDeg) return;
    
    // Определяем направление и шаг
    int step = (targetDeg > currentDeg) ? speed : -speed;
    
    // Плавно перемещаемся
    for(int deg = currentDeg; 
        (step > 0 && deg <= targetDeg) || (step < 0 && deg >= targetDeg); 
        deg += step) {
        servoPan.write(deg);
        vTaskDelay(pdMS_TO_TICKS(15));
    }
    
    // Финальная установка
    servoPan.write(targetDeg);
    currentDeg = targetDeg;
    
    // Обновляем состояние под мьютексом
    xSemaphoreTake(gStateMutex, portMAX_DELAY);
    gState.panUs = targetDeg;  // переиспользуем поле для хранения угла
    gState.panRelDeg = targetDeg - 90;  // относительное положение (-90..90)
    xSemaphoreGive(gStateMutex);
}

/**
 * @brief Плавное перемещение TILT (вертикаль) в целевой угол
 * @param newPos угол в градусах
 * @param speed скорость движения
 */
void servoTiltSlowWrite(int newPos, int speed) {
    if(newPos == lastTiltPos) return;
    
    int step = (newPos > lastTiltPos) ? speed : -speed;
    
    for(int pos = lastTiltPos; 
        (step > 0 && pos <= newPos) || (step < 0 && pos >= newPos); 
        pos += step) {
        servoTilt.write(pos);
        vTaskDelay(pdMS_TO_TICKS(15));
    }
    
    servoTilt.write(newPos);
    lastTiltPos = newPos;
    
    // Обновляем состояние
    xSemaphoreTake(gStateMutex, portMAX_DELAY);
    gState.tiltDeg = newPos;
    xSemaphoreGive(gStateMutex);
}

// ============================================================================
// ЗАДАЧА УПРАВЛЕНИЯ ДВИЖЕНИЕМ
// ============================================================================

void taskMotion(void* pv) {
    // Ждём готовности системы
    while(!systemReady) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    // Инициализация генератора случайных чисел
    randomSeed(analogRead(0));
    
    // Локальные переменные для состояния движения
    RuntimeConfig cfg = gCfg;
    int tilt = cfg.tiltHomeDeg;        // Текущий угол TILT
    int panAngle = 90;                  // Текущий угол PAN (0-180, 90=центр)
    bool panDirRight = true;            // Направление движения PAN
    int8_t orbitPhase = 1;              // Направление TILT по орбите
    
    for(;;) {
        // ---------- 1. КОПИРУЕМ СОСТОЯНИЕ ----------
        SharedState st;
        xSemaphoreTake(gStateMutex, portMAX_DELAY);
        st = gState;
        cfg = gCfg;
        xSemaphoreGive(gStateMutex);

        // ========== ВОТ СЮДА ВСТАВЬТЕ ЭТОТ БЛОК ==========   ++++++++++++++++++++++++
        static unsigned long lastDebug = 0;
        if (millis() - lastDebug > 2000) {
            lastDebug = millis();
            /*
            Serial.printf("Режим: mode=%d, activePlay=%d, catDetected=%d, powerSave=%d\n", 
                          st.mode, st.activePlay, st.catDetected, st.powerSave);
            Serial.printf("PAN cfg: delta=%d, step=%d, settle=%d\n", 
                          cfg.panDeltaUs, cfg.panStepMs, cfg.panSettleMs);
            Serial.printf("TILT cfg: home=%d, min=%d, max=%d, speed=%d\n", 
                          cfg.tiltHomeDeg, cfg.tiltMinDeg, cfg.tiltMaxDeg, cfg.tiltSpeed);
            */              
        }
        // =================================================
        // Управление длительностью сеанса
        if(st.sessionActive && (millis() - st.sessionStartMs >= SESSION_DURATION_SEC * 1000)) {
            Serial.println("Сеанс завершён. Переход в спящий режим на 12 часов.");
            enterSleepMode();  // Функцию добавим ниже
            // После пробуждения выполнение продолжится с этого места
            continue;
        }

        // Границы конуса внимания (в градусах для PAN и TILT)
        const int coneHalf = cfg.coneHalfDeg;
        const int panMin = 90 - coneHalf;
        const int panMax = 90 + coneHalf;
        const int tiltMin = max(cfg.tiltMinDeg, cfg.tiltHomeDeg - coneHalf);
        const int tiltMax = min(cfg.tiltMaxDeg, cfg.tiltHomeDeg + coneHalf);
        
        // ---------- 2. РЕЖИМ КАЛИБРОВКИ ----------
        if(st.mode == MODE_CALIB) {
            setLaser(false);
            panMoveTo(90, 3);           // PAN в центр
            servoTiltSlowWrite(cfg.tiltHomeDeg, 3);
            
            vTaskDelay(pdMS_TO_TICKS(CALIB_DELAY_MS));
            continue;
        }
        
        // ---------- 3. РЕЖИМ ЭКОНОМИИ ЭНЕРГИИ ----------
        if(st.powerSave) {
            setLaser(false);
            servoPan.write(90);
            servoTilt.write(cfg.tiltHomeDeg);
    
            // ===== ЛОГИКА ПЕРЕХОДА В СОСТОЯНИЕ СНА =====
            static uint32_t idleStartTime = 0;
            if(idleStartTime == 0) idleStartTime = millis();
    
            // Если бездействуем больше IDLE_TIMEOUT_SEC — засыпаем
            if((millis() - idleStartTime) > (IDLE_TIMEOUT_SEC * 1000)) {
                //enterIdleMode();
                enterSleepMode();
                idleStartTime = 0;
            }
            // =========================================
    
            xSemaphoreTake(gStateMutex, portMAX_DELAY);
            gState.panUs = 90;
            gState.tiltDeg = cfg.tiltHomeDeg;
            xSemaphoreGive(gStateMutex);
    
            vTaskDelay(pdMS_TO_TICKS(500));
            continue;
        }
/*

            panMoveTo(90, 2);
            servoTiltSlowWrite(cfg.tiltHomeDeg, 2);
            
            tilt = cfg.tiltHomeDeg;
            panAngle = 90;
            panDirRight = true;
            
            vTaskDelay(pdMS_TO_TICKS(500));
            continue;
        }
 */       
        // ---------- 4. КОТА НЕТ — РЕЖИМ ПОИСКА ----------
        if(!st.catDetected) {
            setLaser(false);
            
            // Медленное сканирование горизонта
            int newPan = panAngle + (panDirRight ? 5 : -5);
            if(newPan >= panMax) {
                newPan = panMax;
                panDirRight = false;
            } else if(newPan <= panMin) {
                newPan = panMin;
                panDirRight = true;
            }
            panMoveTo(newPan, 2);
            panAngle = newPan;
            
            vTaskDelay(pdMS_TO_TICKS(randomRange(200, 500)));
        }
        
        // ---------- 5. АКТИВНАЯ ИГРА (КОТ БЛИЖЕ 900 ММ) ----------
        else if(st.activePlay) {
            setLaser(true);
            
            // --- СЛУЧАЙНАЯ ТРАЕКТОРИЯ ---
            int r = random(100);
            
            if(r < 40) {
                // Плавное качание
                int newPan = panAngle + (panDirRight ? 4 : -4);
                if(newPan >= panMax || newPan <= panMin) {
                    panDirRight = !panDirRight;
                    newPan = panAngle;
                }
                panMoveTo(newPan, 3);
                panAngle = newPan;
                
                tilt += orbitPhase;
                if(tilt >= tiltMax || tilt <= tiltMin) orbitPhase = -orbitPhase;
                servoTiltSlowWrite(tilt, cfg.tiltSpeed);
            }
            else if(r < 70) {
                // Резкий скачок в случайную позицию
                int randomPan = randomRange(panMin, panMax);
                panMoveTo(randomPan, 4);
                panAngle = randomPan;
                
                int randomTilt = randomRange(tiltMin, tiltMax);
                servoTiltSlowWrite(randomTilt, 4);
                tilt = randomTilt;
            }
            else {
                soundMouse();
                // Застывание + звук (звук генерируется в SoundTask)
                vTaskDelay(pdMS_TO_TICKS(randomRange(500, 1500)));
            }
            
            vTaskDelay(pdMS_TO_TICKS(randomRange(80, 250)));
        }
        
        // ---------- 6. СПОКОЙНАЯ ИГРА ----------
        else {
            setLaser(true);
            
            // PAN медленно сканирует
            int newPan = panAngle + (panDirRight ? 2 : -2);
            if(newPan >= panMax) {
                newPan = panMax;
                panDirRight = false;
            } else if(newPan <= panMin) {
                newPan = panMin;
                panDirRight = true;
            }
            panMoveTo(newPan, 2);
            panAngle = newPan;
            
            // TILT медленно покачивается
            static bool tiltDir = true;
            tilt += tiltDir ? 1 : -1;
            if(tilt >= cfg.tiltHomeDeg + 8) tiltDir = false;
            if(tilt <= cfg.tiltHomeDeg - 8) tiltDir = true;
            servoTiltSlowWrite(tilt, 2);
            
            vTaskDelay(pdMS_TO_TICKS(randomRange(150, 350)));
        }
    }
    // ============================================================================
// ФУНКЦИЯ УПРАВЛЕНИЯ СНОМ
// ============================================================================
}

void enterSleepMode() {
    Serial.println("Сеанс завершён. Уходим в Deep Sleep на 12 часов.");
    delay(100);
    
    // 1. Сохраняем настройки перед сном
    saveConfig();
    delay(100);
    
    // 2. Выключаем всю периферию
    digitalWrite(LASER_CTRL_PIN, LOW);
    servoPan.detach();
    servoTilt.detach();
    ledcWriteTone(SPEAKER_PIN, 0);
    
    // 3. Очищаем и выключаем OLED (экономия энергии)
    display.clearDisplay();
    display.display();
    display.ssd1306_command(SSD1306_DISPLAYOFF);
    delay(100);
    
    // 4. Отключаем ненужные GPIO (экономия микроампер)
    pinMode(LASER_CTRL_PIN, INPUT);
    pinMode(SPEAKER_PIN, INPUT);
    
    // 5. Настраиваем пробуждение по таймеру
    esp_sleep_enable_timer_wakeup(DEEP_SLEEP_SEC * 1000000LL);  // микросекунды
    delay(100);
    
    // 6. Отключаем Serial перед сном (экономия энергии)
    Serial.end();
    delay(100);
    
    // 7. Уходим в Deep Sleep
    esp_deep_sleep_start();
    // С этого момента код не выполняется до пробуждения
}

void enterSleepMode_() {    // ДЛЯ легкого сна
    Serial.println("Отключаем всё и засыпаем...");
    
    // 1. Отключаем лазер
    digitalWrite(LASER_CTRL_PIN, LOW);
    
    // 2. Отключаем серво
    servoPan.detach();
    servoTilt.detach();
    
    // 3. Выключаем звук
    ledcWriteTone(SPEAKER_PIN, 0);
    
    // 4. Обновляем состояние
    xSemaphoreTake(gStateMutex, portMAX_DELAY);
    gState.sessionActive = false;
    gState.laserOn = false;
    gState.activePlay = false;
    gState.catDetected = false;
    xSemaphoreGive(gStateMutex);
    
    // 5. Приостанавливаем задачи (кроме taskSensor)
    /*  для глубокого сна выключать их не нужно
    vTaskSuspend(taskMotionHandle);
    vTaskSuspend(taskSoundHandle);
    vTaskSuspend(taskUIHandle);
    */

    // 6. Ждём SLEEP_DURATION_SEC секунд
    Serial.printf("Сплю %d часов...\n", SLEEP_DURATION_SEC / 3600);
    for(uint32_t i = 0; i < SLEEP_DURATION_SEC; i++) {
        vTaskDelay(pdMS_TO_TICKS(1000));  // Спим по 1 секунде
    }
    
    // 7. Просыпаемся — переинициализируем
    Serial.println("Пробуждение! Начинаем новый сеанс.");
    
    servoPan.attach(SERVO_PAN_PIN, 500, 2400);
    servoTilt.attach(SERVO_TILT_PIN, 500, 2400);
    servoPan.write(90);
    servoTilt.write(TILT_HOME_DEG_DEF);
    
    // 8. Запускаем новый сеанс
    xSemaphoreTake(gStateMutex, portMAX_DELAY);
    gState.sessionStartMs = millis();
    gState.sessionActive = true;
    gState.catDetected = true;
    gState.activePlay = true;
    gState.mode = MODE_RUN;
    gState.powerSave = false;
    gState.laserOn = true;
    xSemaphoreGive(gStateMutex);
    
    // 9. Возобновляем задачи
    vTaskResume(taskMotionHandle);
    vTaskResume(taskSoundHandle);
    vTaskResume(taskUIHandle);
 
    Serial.println("Новый сеанс начат! (120 секунд)");
 
}