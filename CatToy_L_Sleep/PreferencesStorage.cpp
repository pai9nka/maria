/*
  ============================================================================
   ФАЙЛ: PreferencesStorage.cpp
   НАЗНАЧЕНИЕ: Реализация функций сохранения настроек
  ============================================================================
*/

#include "PreferencesStorage.h"
#include "SharedState.h"
#include "Config.h"

// Внешние переменные
extern RuntimeConfig gCfg;
extern SemaphoreHandle_t gStateMutex;

Preferences gPrefs;   // Хранилище настроек в flash

void initPreferences() {
    Serial.print("Загрузка настроек из flash... ");

    // Открываем раздел "cattoy" (false = режим чтения/записи)
    gPrefs.begin("cattoy", false);

    // Загружаем значения (если есть) или оставляем стандартные
    xSemaphoreTake(gStateMutex, portMAX_DELAY);

    gCfg.panDeltaUs   = gPrefs.getInt("pan_delta", PAN_DELTA_US_DEF);
    gCfg.panStepMs    = gPrefs.getInt("pan_step", PAN_STEP_MS_DEF);
    gCfg.panSettleMs  = gPrefs.getInt("pan_settle", PAN_SETTLE_MS_DEF);
    gCfg.coneHalfDeg  = gPrefs.getInt("cone", CONE_HALF_DEG_DEF);
    gCfg.tiltHomeDeg  = gPrefs.getInt("tilt_home", TILT_HOME_DEG_DEF);
    gCfg.tiltMinDeg   = gPrefs.getInt("tilt_min", TILT_MIN_DEG);
    gCfg.tiltMaxDeg   = gPrefs.getInt("tilt_max", TILT_MAX_DEG);
    gCfg.tiltSpeed    = gPrefs.getInt("tilt_speed", 2);
    gCfg.soundEnabled = gPrefs.getBool("sound_en", true);
    gCfg.soundLevel   = (uint8_t)constrain(gPrefs.getInt("sound_lvl", 2), 1, 3);

    xSemaphoreGive(gStateMutex);

    Serial.println("загружено");
}

void saveConfig() {
    // Копируем текущую конфигурацию под мьютексом
    RuntimeConfig cfg;
    xSemaphoreTake(gStateMutex, portMAX_DELAY);
    cfg = gCfg;
    xSemaphoreGive(gStateMutex);

    // Сохраняем в flash
    gPrefs.putInt("pan_delta", cfg.panDeltaUs);
    gPrefs.putInt("pan_step", cfg.panStepMs);
    gPrefs.putInt("pan_settle", cfg.panSettleMs);
    gPrefs.putInt("cone", cfg.coneHalfDeg);
    gPrefs.putInt("tilt_home", cfg.tiltHomeDeg);
    gPrefs.putInt("tilt_min", cfg.tiltMinDeg);
    gPrefs.putInt("tilt_max", cfg.tiltMaxDeg);
    gPrefs.putInt("tilt_speed", cfg.tiltSpeed);
    gPrefs.putBool("sound_en", cfg.soundEnabled);
    gPrefs.putInt("sound_lvl", cfg.soundLevel);

    Serial.println("Настройки сохранены");
}