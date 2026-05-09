/*
  ============================================================================
   ФАЙЛ: SerialCommands.cpp
   НАЗНАЧЕНИЕ: Реализация обработчика команд Serial
  ============================================================================
*/

#include "SerialCommands.h"
#include "Sound.h"
#include "Motion.h"
#include "PreferencesStorage.h"
#include "Config.h"

// Внешние переменные
extern RuntimeConfig gCfg;
extern SharedState gState;
extern SemaphoreHandle_t gStateMutex;
extern String gSerialLine;

// ============================================================================
// ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ
// ============================================================================

/**
 * @brief Установить диапазон углов для TILT (вертикальной сервы)
 * @param minDeg минимальный угол (градусы, 10..170)
 * @param maxDeg максимальный угол (градусы, 10..170)
 */
static void setTiltRange(int minDeg, int maxDeg) {
    xSemaphoreTake(gStateMutex, portMAX_DELAY);
    gCfg.tiltMinDeg = constrain(minDeg, 10, 170);
    gCfg.tiltMaxDeg = constrain(maxDeg, 10, 170);
    if(gCfg.tiltMinDeg >= gCfg.tiltMaxDeg) {
        gCfg.tiltMaxDeg = gCfg.tiltMinDeg + 10;
    }
    xSemaphoreGive(gStateMutex);
    Serial.println("Диапазон TILT обновлён");
}

/**
 * @brief Обработать команду "set" (установка параметра)
 * @param cmd строка вида "set pan_delta 130"
 * @return true если команда распознана
 */
static bool parseSetCommand(const String& cmd) {
    // Ищем первый и второй пробелы
    int sp1 = cmd.indexOf(' ');
    int sp2 = cmd.indexOf(' ', sp1 + 1);

    if(sp1 < 0) return false;

    String key, valStr;

    if(sp2 < 0) {
        // Формат: "set pan_delta=130"
        int eq = cmd.indexOf('=', sp1 + 1);
        if(eq < 0) return false;
        key = cmd.substring(sp1 + 1, eq);
        valStr = cmd.substring(eq + 1);
    } else {
        // Формат: "set pan_delta 130"
        key = cmd.substring(sp1 + 1, sp2);
        valStr = cmd.substring(sp2 + 1);
    }

    key.trim();
    valStr.trim();

    if(key.length() == 0 || valStr.length() == 0) return false;

    int val = valStr.toInt();

    xSemaphoreTake(gStateMutex, portMAX_DELAY);

    if(key == "pan_delta") {
        gCfg.panDeltaUs = constrain(val, 60, 220);
    } else if(key == "pan_step") {
        gCfg.panStepMs = constrain(val, 5, 120);
    } else if(key == "pan_settle") {
        gCfg.panSettleMs = constrain(val, 20, 200);
    } else if(key == "cone") {
        gCfg.coneHalfDeg = constrain(val, 8, 45);
    } else if(key == "tilt_home") {
        gCfg.tiltHomeDeg = constrain(val, 45, 120);
    } else {
        xSemaphoreGive(gStateMutex);
        return false;
    }

    xSemaphoreGive(gStateMutex);
    return true;
}

// ============================================================================
// ПУБЛИЧНЫЕ ФУНКЦИИ
// ============================================================================

void printHelp() {
    Serial.println("\n===== ДОСТУПНЫЕ КОМАНДЫ =====");
    Serial.println("help                     - показать эту справку");
    Serial.println("show                     - показать текущие настройки");
    Serial.println("save                     - сохранить настройки в flash");
    Serial.println("mode run/calib           - переключить режим");
    Serial.println("");
    Serial.println("set pan_delta <60..220>  - чувствительность поворота PAN");
    Serial.println("set pan_step <5..120>    - длительность шага PAN (мс)");
    Serial.println("set pan_settle <20..200> - пауза после движения PAN (мс)");
    Serial.println("set cone <8..45>         - половина угла конуса (градусы)");
    Serial.println("set tilt_home <45..120>  - домашний угол TILT (градусы)");
    Serial.println("");
    Serial.println("tilt_range <min> <max>   - пределы движения TILT");
    Serial.println("tilt_speed <1..3>        - скорость TILT (1=медл, 3=быстр)");
    Serial.println("pan_speed slow/normal/fast - предустановки PAN");
    Serial.println("");
    Serial.println("sound on/off             - включить/выключить звук");
    Serial.println("sound_profile quiet/normal/loud - громкость");
    Serial.println("test sound bird/mouse    - тест звука");
    Serial.println("===============================\n");
}

void printConfig() {
    RuntimeConfig cfg;
    DeviceMode mode;

    xSemaphoreTake(gStateMutex, portMAX_DELAY);
    cfg = gCfg;
    mode = gState.mode;
    xSemaphoreGive(gStateMutex);

    Serial.println("\n===== ТЕКУЩАЯ КОНФИГУРАЦИЯ =====");
    Serial.print("Режим: ");
    Serial.println(mode == MODE_CALIB ? "CALIB (калибровка)" : "RUN (работа)");
    Serial.print("PAN delta (мкс): "); Serial.println(cfg.panDeltaUs);
    Serial.print("PAN шаг (мс): "); Serial.println(cfg.panStepMs);
    Serial.print("PAN пауза (мс): "); Serial.println(cfg.panSettleMs);
    Serial.print("Конус (градусы): "); Serial.println(cfg.coneHalfDeg);
    Serial.print("TILT home (градусы): "); Serial.println(cfg.tiltHomeDeg);
    Serial.print("TILT диапазон: "); Serial.print(cfg.tiltMinDeg);
    Serial.print(".."); Serial.println(cfg.tiltMaxDeg);
    Serial.print("TILT скорость (1-3): "); Serial.println(cfg.tiltSpeed);
    Serial.print("Звук: "); Serial.println(cfg.soundEnabled ? "ВКЛ" : "ВЫКЛ");
    Serial.print("Громкость: ");
    if(cfg.soundLevel == 1) Serial.println("ТИХО");
    else if(cfg.soundLevel == 2) Serial.println("НОРМАЛЬНО");
    else Serial.println("ГРОМКО");
    Serial.println("===============================\n");
}

void handleSerialCommand(const String& lineRaw) {
    String line = lineRaw;
    line.trim();

    if(line.length() == 0) return;

    // Преобразуем в нижний регистр для удобства (но с сохранением аргументов)
    String lowerLine = line;
    lowerLine.toLowerCase();

    // ----- help -----
    if(lowerLine == "help") {
        printHelp();
        return;
    }

    // ----- show -----
    if(lowerLine == "show") {
        printConfig();
        return;
    }

    // ----- save -----
    if(lowerLine == "save") {
        saveConfig();
        Serial.println("Настройки сохранены в flash");
        return;
    }

    // ----- mode run / mode calib -----
    if(lowerLine == "mode run") {
        xSemaphoreTake(gStateMutex, portMAX_DELAY);
        gState.mode = MODE_RUN;
        xSemaphoreGive(gStateMutex);
        Serial.println("Режим: RUN (рабочий)");
        return;
    }

    if(lowerLine == "mode calib") {
        xSemaphoreTake(gStateMutex, portMAX_DELAY);
        gState.mode = MODE_CALIB;
        xSemaphoreGive(gStateMutex);
        Serial.println("Режим: CALIB (калибровка)");
        return;
    }

    // ----- sound on / off -----
    if(lowerLine == "sound on") {
        xSemaphoreTake(gStateMutex, portMAX_DELAY);
        gCfg.soundEnabled = true;
        xSemaphoreGive(gStateMutex);
        Serial.println("Звук ВКЛЮЧЁН");
        printConfig();
        return;
    }

    if(lowerLine == "sound off") {
        xSemaphoreTake(gStateMutex, portMAX_DELAY);
        gCfg.soundEnabled = false;
        xSemaphoreGive(gStateMutex);
        ledcWriteTone(SPEAKER_PIN, 0);
        Serial.println("Звук ВЫКЛЮЧЁН");
        printConfig();
        return;
    }

    // ----- sound_profile quiet/normal/loud -----
    if(lowerLine == "sound_profile quiet") {
        xSemaphoreTake(gStateMutex, portMAX_DELAY);
        gCfg.soundLevel = 1;
        xSemaphoreGive(gStateMutex);
        Serial.println("Громкость: ТИХО");
        printConfig();
        return;
    }

    if(lowerLine == "sound_profile normal") {
        xSemaphoreTake(gStateMutex, portMAX_DELAY);
        gCfg.soundLevel = 2;
        xSemaphoreGive(gStateMutex);
        Serial.println("Громкость: НОРМАЛЬНО");
        printConfig();
        return;
    }

    if(lowerLine == "sound_profile loud") {
        xSemaphoreTake(gStateMutex, portMAX_DELAY);
        gCfg.soundLevel = 3;
        xSemaphoreGive(gStateMutex);
        Serial.println("Громкость: ГРОМКО");
        printConfig();
        return;
    }

    // ----- test sound bird / mouse -----
    if(lowerLine == "test sound bird") {
        Serial.println("Тест: звук птички");
        soundBirdOriginal();
        return;
    }

    if(lowerLine == "test sound mouse") {
        Serial.println("Тест: звук мышки");
        soundMouse();
        return;
    }

    // ----- pan_speed slow/normal/fast -----
    if(lowerLine == "pan_speed slow") {
        xSemaphoreTake(gStateMutex, portMAX_DELAY);
        gCfg.panDeltaUs = 90;
        gCfg.panStepMs = 18;
        gCfg.panSettleMs = 140;
        xSemaphoreGive(gStateMutex);
        Serial.println("PAN: МЕДЛЕННО (дельта=90, шаг=18, пауза=140)");
        printConfig();
        return;
    }

    if(lowerLine == "pan_speed normal") {
        xSemaphoreTake(gStateMutex, portMAX_DELAY);
        gCfg.panDeltaUs = 120;
        gCfg.panStepMs = 28;
        gCfg.panSettleMs = 70;
        xSemaphoreGive(gStateMutex);
        Serial.println("PAN: НОРМАЛЬНО (дельта=120, шаг=28, пауза=70)");
        printConfig();
        return;
    }

    if(lowerLine == "pan_speed fast") {
        xSemaphoreTake(gStateMutex, portMAX_DELAY);
        gCfg.panDeltaUs = 160;
        gCfg.panStepMs = 32;
        gCfg.panSettleMs = 45;
        xSemaphoreGive(gStateMutex);
        Serial.println("PAN: БЫСТРО (дельта=160, шаг=32, пауза=45)");
        printConfig();
        return;
    }

    // ----- tilt_range min max -----
    if(lowerLine.startsWith("tilt_range ")) {
        int sp = line.indexOf(' ', 11);
        if(sp > 0) {
            int minD = line.substring(11, sp).toInt();
            int maxD = line.substring(sp + 1).toInt();
            setTiltRange(minD, maxD);
            printConfig();
        }
        return;
    }

    // ----- tilt_speed 1/2/3 -----
    if(lowerLine.startsWith("tilt_speed ")) {
        int spd = line.substring(11).toInt();
        if(spd >= 1 && spd <= 3) {
            xSemaphoreTake(gStateMutex, portMAX_DELAY);
            gCfg.tiltSpeed = spd;
            xSemaphoreGive(gStateMutex);
            Serial.printf("Скорость TILT: %d\n", spd);
            printConfig();
        }
        return;
    }

    // ----- set param value -----
    if(lowerLine.startsWith("set ")) {
        if(parseSetCommand(line)) {
            Serial.println("Настройка применена");
            printConfig();
        } else {
            Serial.println("Ошибка: неверный параметр. Доступны: pan_delta, pan_step, pan_settle, cone, tilt_home");
        }
        return;
    }

    Serial.println("Неизвестная команда. Введите 'help'");
}

void processSerial() {
    while(Serial.available() > 0) {
        char c = (char)Serial.read();

        if(c == '\n' || c == '\r') {
            if(gSerialLine.length() > 0) {
                handleSerialCommand(gSerialLine);
                gSerialLine = "";
            }
        } else {
            gSerialLine += c;
            if(gSerialLine.length() > 120) {
                gSerialLine = "";
            }
        }
    }
}