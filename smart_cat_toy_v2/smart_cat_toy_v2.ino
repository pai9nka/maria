
/*
  Проект: "Умная игрушка для кота" на ESP32
  Версия: v2 (рабочая копия для доработок), с комментариями на русском

  Примечание:
  Этот файл создан как копия v1, чтобы изменения не затирали зафиксированную версию.
  
  Что делает скетч:
  1) Считывает расстояние с VL53L0X.
  2) Управляет 2 сервоприводами:
     - PAN: SG90 360 (не позиционный, а непрерывного вращения)
     - TILT: SG90 180 (позиционный)
  3) Включает/выключает лазер через транзисторный ключ.
  4) Выводит состояние на OLED SSD1306 (SPI).
  5) Использует FreeRTOS-задачи.
  6) Имеет простой "экономичный" режим при бездействии.

  ВАЖНО ПО ЖЕЛЕЗУ:
  - Серво питать от отдельной 5V линии (не от 3.3V ESP32).
  - Общая земля обязательна: GND ESP32 = GND серв = GND датчика = GND ключа лазера.
  - Лазер управлять через транзистор (GPIO -> резистор -> база/затвор).
*/

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_VL53L0X.h>
#include <ESP32Servo.h>
#include <Preferences.h>

// =========================
// ПИНЫ (с учетом OLED SPI)
// =========================
// OLED SSD1306 SPI
#define OLED_MOSI   23
#define OLED_CLK    18
#define OLED_DC      2
#define OLED_CS      5
#define OLED_RESET   4

// Остальные устройства
#define SERVO_PAN_PIN    19   // SG90 360 (непрерывное вращение)
#define SERVO_TILT_PIN   27   // SG90 180 (позиционный)
#define LASER_CTRL_PIN   26   // Управление лазером через транзистор
#define SPEAKER_PIN      25   // Пищалка/динамик (через PWM)

// I2C для VL53L0X
#define I2C_SDA_PIN      21
#define I2C_SCL_PIN      22

// =========================
// НАСТРОЙКИ ПОВЕДЕНИЯ
// =========================
static const uint16_t CAT_DETECT_MM        = 1200;   // Если ближе — считаем "кот в зоне"
static const uint16_t CAT_ACTIVE_MM        = 900;    // Активный режим игры
static const uint32_t INACTIVE_TIMEOUT_MS  = 60000;  // Через 60 сек без кота -> эконом режим

// Для SG90-360 часто нужна заметная "дельта" от стопа, иначе мотор не трогается.
// SG90 360: управление импульсом
// 1500 мкс ~ стоп, >1500 вращение в одну сторону, <1500 в другую
static const int PAN_STOP_US        = 1500;
static const int PAN_DELTA_US_DEF   = 120;
static const int PAN_STEP_MS_DEF    = 28;
static const int PAN_SETTLE_MS_DEF  = 70;
static const int PAN_NUDGE_DEG      = 4;

static const int CONE_HALF_DEG_DEF = 22;

// SG90 180: углы
static const int TILT_MIN_DEG  = 40;
static const int TILT_MAX_DEG  = 130;
static const int TILT_HOME_DEG_DEF = 85;

// =========================
// ГЛОБАЛЬНЫЕ ОБЪЕКТЫ
// =========================
Adafruit_SSD1306 display(128, 64, &SPI, OLED_DC, OLED_RESET, OLED_CS);
Adafruit_VL53L0X lox;
Servo servoPan;
Servo servoTilt;

// Для звука (LEDC)
const int BUZZ_CH = 0;
const int BUZZ_RES_BITS = 8;
const int BUZZ_BASE_FREQ = 2000;

struct RuntimeConfig {
  int panDeltaUs = PAN_DELTA_US_DEF;
  int panStepMs = PAN_STEP_MS_DEF;
  int panSettleMs = PAN_SETTLE_MS_DEF;
  int coneHalfDeg = CONE_HALF_DEG_DEF;
  int tiltHomeDeg = TILT_HOME_DEG_DEF;
  bool soundEnabled = true;
  uint8_t soundLevel = 2; // 1=quiet, 2=normal, 3=loud
};

RuntimeConfig gCfg;
Preferences gPrefs;

enum DeviceMode : uint8_t {
  MODE_RUN = 0,
  MODE_CALIB = 1
};

// =========================
// ОБЩЕЕ СОСТОЯНИЕ (между задачами)
// =========================
struct SharedState {
  uint16_t distanceMM = 8190;   // Текущая дистанция
  bool sensorOk = false;
  bool catDetected = false;     // Кот в зоне
  bool activePlay = false;      // Активная игра
  bool powerSave = false;       // Экономичный режим
  bool laserOn = false;
  uint32_t lastCatSeenMs = 0;
  int tiltDeg = TILT_HOME_DEG_DEF;
  int panUs = PAN_STOP_US;
  int panRelDeg = 0;
  DeviceMode mode = MODE_RUN;
};

SharedState gState;
SemaphoreHandle_t gStateMutex;
String gSerialLine;

int panLeftUs(const RuntimeConfig& cfg) {
  return PAN_STOP_US - cfg.panDeltaUs;
}

int panRightUs(const RuntimeConfig& cfg) {
  return PAN_STOP_US + cfg.panDeltaUs;
}

int tiltConeMin(const RuntimeConfig& cfg) {
  return cfg.tiltHomeDeg - cfg.coneHalfDeg;
}

int tiltConeMax(const RuntimeConfig& cfg) {
  return cfg.tiltHomeDeg + cfg.coneHalfDeg;
}

// =========================
// ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ
// =========================
void beep(uint16_t freq, uint16_t durMs) {
  RuntimeConfig cfg;
  xSemaphoreTake(gStateMutex, portMAX_DELAY);
  cfg = gCfg;
  xSemaphoreGive(gStateMutex);
  if (!cfg.soundEnabled) return;

  uint16_t scaledFreq = freq;
  if (cfg.soundLevel == 1) {
    scaledFreq = max<uint16_t>(200, freq * 70 / 100);
  } else if (cfg.soundLevel >= 3) {
    scaledFreq = min<uint16_t>(8000, freq * 130 / 100);
  }

#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
  ledcWriteTone(SPEAKER_PIN, scaledFreq);
#else
  ledcWriteTone(BUZZ_CH, scaledFreq);
#endif
  vTaskDelay(pdMS_TO_TICKS(durMs));
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
  ledcWriteTone(SPEAKER_PIN, 0);
#else
  ledcWriteTone(BUZZ_CH, 0);
#endif
}

// Простая "птичка" — 2 тона
void soundBird() {
  beep(2800, 80);
  vTaskDelay(pdMS_TO_TICKS(30));
  beep(3400, 90);
}

// "Мышка" — короткие низкие щелчки
void soundMouse() {
  beep(1200, 50);
  vTaskDelay(pdMS_TO_TICKS(20));
  beep(1000, 50);
}

// Безопасно обновляем состояние лазера
void setLaser(bool on) {
  digitalWrite(LASER_CTRL_PIN, on ? HIGH : LOW);
  xSemaphoreTake(gStateMutex, portMAX_DELAY);
  gState.laserOn = on;
  xSemaphoreGive(gStateMutex);
}

void panNudge(bool toRight, const RuntimeConfig& cfg) {
  servoPan.writeMicroseconds(toRight ? panRightUs(cfg) : panLeftUs(cfg));
  vTaskDelay(pdMS_TO_TICKS(cfg.panStepMs));
  servoPan.writeMicroseconds(PAN_STOP_US);
  vTaskDelay(pdMS_TO_TICKS(cfg.panSettleMs));
}

// =========================
// ЗАДАЧА 1: датчик дистанции
// =========================
void taskSensor(void* pv) {
  VL53L0X_RangingMeasurementData_t measure;
  for (;;) {
    bool ok = false;
    uint16_t mm = 8190;
    lox.rangingTest(&measure, false);
    if (measure.RangeStatus != 4) {  // 4 = out of range
      mm = measure.RangeMilliMeter;
      ok = true;
    }

    xSemaphoreTake(gStateMutex, portMAX_DELAY);
    gState.sensorOk = ok;
    gState.distanceMM = mm;
    if (ok && mm <= CAT_DETECT_MM) {
      gState.catDetected = true;
      gState.lastCatSeenMs = millis();
      gState.activePlay = (mm <= CAT_ACTIVE_MM);
    } else {
      gState.catDetected = false;
      gState.activePlay = false;
    }

    // Включение экономичного режима при длительном отсутствии кота
    uint32_t now = millis();
    gState.powerSave = (now - gState.lastCatSeenMs) > INACTIVE_TIMEOUT_MS;
    xSemaphoreGive(gStateMutex);

    vTaskDelay(pdMS_TO_TICKS(70));   // ~14 Гц
  }
}

// =========================
// ЗАДАЧА 2: движение и лазер
// =========================
void taskMotion(void* pv) {
  RuntimeConfig cfg = gCfg;
  int tilt = cfg.tiltHomeDeg;
  bool panDirRight = true;
  int panEstimateDeg = 0;
  int8_t orbitPhase = 1;

  for (;;) {
    SharedState st;
    xSemaphoreTake(gStateMutex, portMAX_DELAY);
    st = gState;
    cfg = gCfg;
    xSemaphoreGive(gStateMutex);

    const int coneHalf = cfg.coneHalfDeg;
    const int tiltMin = tiltConeMin(cfg);
    const int tiltMax = tiltConeMax(cfg);

    if (st.mode == MODE_CALIB) {
      // Экономичный режим: лазер выкл, серво в спокойное положение
      setLaser(false);
      servoPan.writeMicroseconds(PAN_STOP_US);
      servoTilt.write(cfg.tiltHomeDeg);
      xSemaphoreTake(gStateMutex, portMAX_DELAY);
      gState.panUs = PAN_STOP_US;
      gState.tiltDeg = cfg.tiltHomeDeg;
      xSemaphoreGive(gStateMutex);
      vTaskDelay(pdMS_TO_TICKS(120));
      continue;
    }

    if (st.powerSave) {
      setLaser(false);
      servoPan.writeMicroseconds(PAN_STOP_US);
      servoTilt.write(cfg.tiltHomeDeg);
      tilt = cfg.tiltHomeDeg;
      panDirRight = true;
      panEstimateDeg = 0;
      xSemaphoreTake(gStateMutex, portMAX_DELAY);
      gState.panUs = PAN_STOP_US;
      gState.tiltDeg = cfg.tiltHomeDeg;
      gState.panRelDeg = panEstimateDeg;
      xSemaphoreGive(gStateMutex);
      vTaskDelay(pdMS_TO_TICKS(500));
      continue;
    }

    if (!st.catDetected) {
      // Кота не видно: мягкий поиск (редкие движения, лазер выкл)
      setLaser(false);

      // Панорама короткими "тычками"
      panNudge(panDirRight, cfg);
      panEstimateDeg += panDirRight ? PAN_NUDGE_DEG : -PAN_NUDGE_DEG;
      if (panEstimateDeg >= coneHalf) {
        panEstimateDeg = coneHalf;
        panDirRight = false;
      } else if (panEstimateDeg <= -coneHalf) {
        panEstimateDeg = -coneHalf;
        panDirRight = true;
      }
      xSemaphoreTake(gStateMutex, portMAX_DELAY);
      gState.panUs = PAN_STOP_US;
      gState.panRelDeg = panEstimateDeg;
      xSemaphoreGive(gStateMutex);
      vTaskDelay(pdMS_TO_TICKS(350));
    } else if (st.activePlay) {
      // Активная игра: лазер вкл, более динамичная траектория
      setLaser(true);
      // "Зайчик по окружности": pan и tilt двигаются синхронно, но со сдвигом фазы.
      panNudge(panDirRight, cfg);
      panEstimateDeg += panDirRight ? PAN_NUDGE_DEG : -PAN_NUDGE_DEG;
      if (panEstimateDeg >= coneHalf) {
        panEstimateDeg = coneHalf;
        panDirRight = false;
      } else if (panEstimateDeg <= -coneHalf) {
        panEstimateDeg = -coneHalf;
        panDirRight = true;
      }
      tilt += orbitPhase;
      if (tilt >= tiltMax) {
        tilt = tiltMax;
        orbitPhase = -1;
      } else if (tilt <= tiltMin) {
        tilt = tiltMin;
        orbitPhase = 1;
      }
      servoTilt.write(tilt);
      xSemaphoreTake(gStateMutex, portMAX_DELAY);
      gState.panUs = PAN_STOP_US;
      gState.tiltDeg = tilt;
      gState.panRelDeg = panEstimateDeg;
      xSemaphoreGive(gStateMutex);
      vTaskDelay(pdMS_TO_TICKS(120));
    } else {
      // Кот в зоне, но далеко: спокойный режим игры
      setLaser(true);
      servoPan.writeMicroseconds(PAN_STOP_US);

      // Небольшие покачивания по tilt
      static bool dir = true;
      tilt += dir ? 1 : -1;
      if (tilt >= cfg.tiltHomeDeg + 10) dir = false;
      if (tilt <= cfg.tiltHomeDeg - 10) dir = true;
      servoTilt.write(tilt);
      xSemaphoreTake(gStateMutex, portMAX_DELAY);
      gState.panUs = PAN_STOP_US;
      gState.tiltDeg = tilt;
      gState.panRelDeg = panEstimateDeg;
      xSemaphoreGive(gStateMutex);
      vTaskDelay(pdMS_TO_TICKS(180));
    }
  }
}

// =========================
// ЗАДАЧА 3: звук
// =========================
void taskSound(void* pv) {
  for (;;) {
    SharedState st;
    xSemaphoreTake(gStateMutex, portMAX_DELAY);
    st = gState;
    xSemaphoreGive(gStateMutex);
    if (st.mode == MODE_CALIB || st.powerSave) {
      vTaskDelay(pdMS_TO_TICKS(1200));
      continue;
    }
    if (st.activePlay) {
      soundBird();
      vTaskDelay(pdMS_TO_TICKS(500));
    } else if (st.catDetected) {
      soundMouse();
      vTaskDelay(pdMS_TO_TICKS(900));
    } else {
      vTaskDelay(pdMS_TO_TICKS(1200));
    }
  }
}

// =========================
// ЗАДАЧА 4: интерфейс OLED
// =========================
void taskUI(void* pv) {
  for (;;) {
    SharedState st;
    xSemaphoreTake(gStateMutex, portMAX_DELAY);
    st = gState;
    xSemaphoreGive(gStateMutex);

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0); display.println("Smart Cat Toy");
    display.setCursor(0, 12); display.print("Sensor: "); display.println(st.sensorOk ? "OK" : "ERR");
    display.setCursor(0, 22); display.print("Dist(mm): "); if (st.sensorOk) display.println(st.distanceMM); else display.println("---");
    display.setCursor(0, 32); display.print("Cat: "); display.println(st.catDetected ? "YES" : "NO");
    display.setCursor(0, 42); display.print("Mode: ");
    if (st.mode == MODE_CALIB) display.println("CALIB");
    else if (st.powerSave) display.println("POWER SAVE");
    else if (st.activePlay) display.println("ACTIVE PLAY");
    else if (st.catDetected) display.println("CALM PLAY");
    else display.println("SEARCH");
    display.setCursor(0, 54); display.print("L:"); display.print(st.laserOn ? "ON " : "OFF"); display.print(" T:"); display.print(st.tiltDeg); display.print(" H:"); display.print(st.panRelDeg);
    display.display();
    vTaskDelay(pdMS_TO_TICKS(250));
  }
}

void printHelp() {
  Serial.println("Commands:");
  Serial.println("  help");
  Serial.println("  show");
  Serial.println("  mode run|calib");
  Serial.println("  set pan_delta <60..220>");
  Serial.println("  set pan_step <5..120>");
  Serial.println("  set pan_settle <20..200>");
  Serial.println("  set cone <8..45>");
  Serial.println("  set tilt_home <45..120>");
  Serial.println("  pan_speed slow|normal|fast");
  Serial.println("  sound on|off");
  Serial.println("  sound_profile quiet|normal|loud");
  Serial.println("  test sound bird|mouse");
  Serial.println("  save");
}

void printConfig() {
  RuntimeConfig cfg;
  DeviceMode mode;
  xSemaphoreTake(gStateMutex, portMAX_DELAY);
  cfg = gCfg;
  mode = gState.mode;
  xSemaphoreGive(gStateMutex);

  Serial.print("mode="); Serial.println(mode == MODE_CALIB ? "calib" : "run");
  Serial.print("pan_delta="); Serial.println(cfg.panDeltaUs);
  Serial.print("pan_step="); Serial.println(cfg.panStepMs);
  Serial.print("pan_settle="); Serial.println(cfg.panSettleMs);
  Serial.print("cone="); Serial.println(cfg.coneHalfDeg);
  Serial.print("tilt_home="); Serial.println(cfg.tiltHomeDeg);
  Serial.print("sound="); Serial.println(cfg.soundEnabled ? "on" : "off");
  Serial.print("sound_profile=");
  if (cfg.soundLevel <= 1) Serial.println("quiet");
  else if (cfg.soundLevel >= 3) Serial.println("loud");
  else Serial.println("normal");
}

bool parseSetCommand(const String& cmd) {
  int sp1 = cmd.indexOf(' ');
  int sp2 = cmd.indexOf(' ', sp1 + 1);
  if (sp1 < 0) return false;
  String key;
  String valStr;
  if (sp2 < 0) {
    String tail = cmd.substring(sp1 + 1);
    int eq = tail.indexOf('=');
    if (eq < 0) return false;
    key = tail.substring(0, eq);
    valStr = tail.substring(eq + 1);
  } else {
    key = cmd.substring(sp1 + 1, sp2);
    valStr = cmd.substring(sp2 + 1);
  }
  key.trim();
  valStr.trim();
  if (key.length() == 0 || valStr.length() == 0) return false;
  int val = valStr.toInt();

  xSemaphoreTake(gStateMutex, portMAX_DELAY);
  if (key == "pan_delta") gCfg.panDeltaUs = constrain(val, 60, 220);
  else if (key == "pan_step") gCfg.panStepMs = constrain(val, 5, 120);
  else if (key == "pan_settle") gCfg.panSettleMs = constrain(val, 20, 200);
  else if (key == "cone") gCfg.coneHalfDeg = constrain(val, 8, 45);
  else if (key == "tilt_home") gCfg.tiltHomeDeg = constrain(val, 45, 120);
  else {
    xSemaphoreGive(gStateMutex);
    return false;
  }
  xSemaphoreGive(gStateMutex);
  return true;
}

void saveConfig() {
  RuntimeConfig cfg;
  xSemaphoreTake(gStateMutex, portMAX_DELAY);
  cfg = gCfg;
  xSemaphoreGive(gStateMutex);

  gPrefs.putInt("pan_delta", cfg.panDeltaUs);
  gPrefs.putInt("pan_step", cfg.panStepMs);
  gPrefs.putInt("pan_settle", cfg.panSettleMs);
  gPrefs.putInt("cone", cfg.coneHalfDeg);
  gPrefs.putInt("tilt_home", cfg.tiltHomeDeg);
  gPrefs.putBool("sound_en", cfg.soundEnabled);
  gPrefs.putInt("sound_lvl", cfg.soundLevel);
}

void handleSerialCommand(const String& lineRaw) {
  String line = lineRaw;
  line.trim();
  if (line.length() == 0) return;

  if (line == "help") {
    printHelp();
    return;
  }
  if (line == "show") {
    printConfig();
    return;
  }
  if (line == "save") {
    saveConfig();
    Serial.println("Saved.");
    return;
  }
  if (line == "mode run") {
    xSemaphoreTake(gStateMutex, portMAX_DELAY);
    gState.mode = MODE_RUN;
    xSemaphoreGive(gStateMutex);
    Serial.println("Mode: RUN");
    return;
  }
  if (line == "mode calib") {
    xSemaphoreTake(gStateMutex, portMAX_DELAY);
    gState.mode = MODE_CALIB;
    xSemaphoreGive(gStateMutex);
    Serial.println("Mode: CALIB");
    return;
  }
  if (line == "sound on") {
    xSemaphoreTake(gStateMutex, portMAX_DELAY);
    gCfg.soundEnabled = true;
    xSemaphoreGive(gStateMutex);
    Serial.println("Sound: ON");
    printConfig();
    return;
  }
  if (line == "sound off") {
    xSemaphoreTake(gStateMutex, portMAX_DELAY);
    gCfg.soundEnabled = false;
    xSemaphoreGive(gStateMutex);
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
    ledcWriteTone(SPEAKER_PIN, 0);
#else
    ledcWriteTone(BUZZ_CH, 0);
#endif
    Serial.println("Sound: OFF");
    printConfig();
    return;
  }
  if (line == "sound_profile quiet") {
    xSemaphoreTake(gStateMutex, portMAX_DELAY);
    gCfg.soundLevel = 1;
    xSemaphoreGive(gStateMutex);
    Serial.println("Sound profile: quiet");
    printConfig();
    return;
  }
  if (line == "sound_profile normal") {
    xSemaphoreTake(gStateMutex, portMAX_DELAY);
    gCfg.soundLevel = 2;
    xSemaphoreGive(gStateMutex);
    Serial.println("Sound profile: normal");
    printConfig();
    return;
  }
  if (line == "sound_profile loud") {
    xSemaphoreTake(gStateMutex, portMAX_DELAY);
    gCfg.soundLevel = 3;
    xSemaphoreGive(gStateMutex);
    Serial.println("Sound profile: loud");
    printConfig();
    return;
  }
  if (line == "test sound bird") {
    soundBird();
    Serial.println("Test: bird");
    return;
  }
  if (line == "test sound mouse") {
    soundMouse();
    Serial.println("Test: mouse");
    return;
  }
  if (line.startsWith("set ")) {
    if (parseSetCommand(line)) {
      Serial.println("OK");
      printConfig();
    } else {
      Serial.println("Bad set command. Try: set pan_delta 130");
    }
    return;
  }
  if (line == "pan_speed slow") {
    xSemaphoreTake(gStateMutex, portMAX_DELAY);
    gCfg.panDeltaUs = 90;
    gCfg.panStepMs = 18;
    gCfg.panSettleMs = 140;
    xSemaphoreGive(gStateMutex);
    Serial.println("PAN profile: slow");
    printConfig();
    return;
  }
  if (line == "pan_speed normal") {
    xSemaphoreTake(gStateMutex, portMAX_DELAY);
    gCfg.panDeltaUs = 120;
    gCfg.panStepMs = 28;
    gCfg.panSettleMs = 70;
    xSemaphoreGive(gStateMutex);
    Serial.println("PAN profile: normal");
    printConfig();
    return;
  }
  if (line == "pan_speed fast") {
    xSemaphoreTake(gStateMutex, portMAX_DELAY);
    gCfg.panDeltaUs = 145;
    gCfg.panStepMs = 32;
    gCfg.panSettleMs = 45;
    xSemaphoreGive(gStateMutex);
    Serial.println("PAN profile: fast");
    printConfig();
    return;
  }

  Serial.println("Unknown command. Type: help");
}

void processSerial() {
  while (Serial.available() > 0) {
    char c = (char)Serial.read();
    if (c == '\n' || c == '\r') {
      if (gSerialLine.length() > 0) {
        handleSerialCommand(gSerialLine);
        gSerialLine = "";
      }
    } else {
      gSerialLine += c;
      if (gSerialLine.length() > 120) gSerialLine = "";
    }
  }
}

// =========================
// SETUP / LOOP
// =========================
void setup() {
  Serial.begin(115200);
  delay(300);

  // Mutex для общего состояния
  gStateMutex = xSemaphoreCreateMutex();
  if (!gStateMutex) {
    Serial.println("ERROR: mutex create failed");
    while (true) delay(1000);
  }

  gPrefs.begin("cattoy", false);
  gCfg.panDeltaUs = gPrefs.getInt("pan_delta", PAN_DELTA_US_DEF);
  gCfg.panStepMs = gPrefs.getInt("pan_step", PAN_STEP_MS_DEF);
  gCfg.panSettleMs = gPrefs.getInt("pan_settle", PAN_SETTLE_MS_DEF);
  gCfg.coneHalfDeg = gPrefs.getInt("cone", CONE_HALF_DEG_DEF);
  gCfg.tiltHomeDeg = gPrefs.getInt("tilt_home", TILT_HOME_DEG_DEF);
  gCfg.soundEnabled = gPrefs.getBool("sound_en", true);
  gCfg.soundLevel = (uint8_t)constrain(gPrefs.getInt("sound_lvl", 2), 1, 3);

  xSemaphoreTake(gStateMutex, portMAX_DELAY);
  gState.mode = MODE_RUN;
  gState.tiltDeg = gCfg.tiltHomeDeg;
  xSemaphoreGive(gStateMutex);

  // GPIO
  pinMode(LASER_CTRL_PIN, OUTPUT);
  digitalWrite(LASER_CTRL_PIN, LOW);

  // I2C + VL53L0X
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  if (!lox.begin()) {
    Serial.println("ERROR: VL53L0X not found!");
    // Продолжаем, но в UI будет Sensor:ERR
  } else {
    Serial.println("VL53L0X OK");
    xSemaphoreTake(gStateMutex, portMAX_DELAY);
    gState.sensorOk = true;
    gState.lastCatSeenMs = millis();
    xSemaphoreGive(gStateMutex);
  }

  // SPI + OLED
  SPI.begin(OLED_CLK, -1, OLED_MOSI, OLED_CS);
  if (!display.begin(SSD1306_SWITCHCAPVCC)) {
    Serial.println("ERROR: SSD1306 init failed");
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("OLED OK");
    display.display();
  }

  // Серво
  servoPan.setPeriodHertz(50);
  servoTilt.setPeriodHertz(50);

  // Диапазон импульсов SG90 типично 500..2400 мкс
  servoPan.attach(SERVO_PAN_PIN, 500, 2400);
  servoTilt.attach(SERVO_TILT_PIN, 500, 2400);
  servoPan.writeMicroseconds(PAN_STOP_US);
  servoTilt.write(gCfg.tiltHomeDeg);
  delay(400);

#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
  // Новый API ESP32 core 3.x: канал назначается автоматически.
  ledcAttach(SPEAKER_PIN, BUZZ_BASE_FREQ, BUZZ_RES_BITS);
  ledcWriteTone(SPEAKER_PIN, 0);
#else
  // Старый API ESP32 core 2.x.
  ledcSetup(BUZZ_CH, BUZZ_BASE_FREQ, BUZZ_RES_BITS);
  ledcAttachPin(SPEAKER_PIN, BUZZ_CH);
  ledcWriteTone(BUZZ_CH, 0);
#endif

  // Создание задач FreeRTOS
  xTaskCreatePinnedToCore(taskSensor, "SensorTask", 4096, nullptr, 2, nullptr, 1);
  xTaskCreatePinnedToCore(taskMotion, "MotionTask", 4096, nullptr, 2, nullptr, 1);
  xTaskCreatePinnedToCore(taskSound, "SoundTask", 3072, nullptr, 1, nullptr, 0);
  xTaskCreatePinnedToCore(taskUI, "UITask", 4096, nullptr, 1, nullptr, 0);

  Serial.println("System started. Type 'help' in Serial Monitor.");
  printConfig();
}

void loop() {
  // Не используем loop, логика в задачах FreeRTOS
  processSerial();
  vTaskDelay(pdMS_TO_TICKS(30));
}
/*

Не дал отладить нормальный звук :( - хочет денег...

*/