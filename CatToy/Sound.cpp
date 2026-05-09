#include "Sound.h"
#include "SharedState.h"
#include <math.h>

extern RuntimeConfig gCfg;
extern SemaphoreHandle_t gStateMutex;

void initSound() {
    pinMode(SPEAKER_PIN, OUTPUT);
    Serial.println("Звуковой модуль инициализирован (tone)");
}

// Базовый писк
void beep(uint16_t freq, uint16_t durMs) {
    RuntimeConfig cfg;
    xSemaphoreTake(gStateMutex, portMAX_DELAY);
    cfg = gCfg;
    xSemaphoreGive(gStateMutex);
    
    if (!cfg.soundEnabled) return;
    
    tone(SPEAKER_PIN, freq, durMs);
    vTaskDelay(pdMS_TO_TICKS(durMs + 50));
}

// Случайный мышиный писк
void soundMouse() {

    RuntimeConfig cfg;
    xSemaphoreTake(gStateMutex, portMAX_DELAY);
    cfg = gCfg;
    xSemaphoreGive(gStateMutex);
    if (!cfg.soundEnabled) return;
    
    int count = random(2, 5);
    for (int i = 0; i < count; i++) {
        int freq = random(4000, 9000);
        int duration = random(50,180);
        tone(SPEAKER_PIN, freq, duration);
        vTaskDelay(duration + 50);
        //delay(random(50, 150));
    }
}

// Птичка (свип вниз)
void soundBirdOriginal() {
    RuntimeConfig cfg;
    xSemaphoreTake(gStateMutex, portMAX_DELAY);
    cfg = gCfg;
    xSemaphoreGive(gStateMutex);
    if (!cfg.soundEnabled) return;
    
    int startF = random(6000, 9000);
    int endF = startF - random(2000, 3000);
    for (int f = startF; f > endF; f -= 150) {
        int duration = 10;
        tone(SPEAKER_PIN, f, duration);
        vTaskDelay(duration + 50);
        //delay(7);
    }
}

// Стрекотание
void soundTrill() {
    RuntimeConfig cfg;
    xSemaphoreTake(gStateMutex, portMAX_DELAY);
    cfg = gCfg;
    xSemaphoreGive(gStateMutex);
    if (!cfg.soundEnabled) return;
    
    int clicks = random(5, 15);
    for (int i = 0; i < clicks; i++) {
        int duration = random(10,20);
        tone(SPEAKER_PIN, random(3000, 5000), duration);
        vTaskDelay(duration + 50);
        //delay(random(20, 60));
    }
}

// Активная игра — случайный выбор
void soundActivePlay() {
    int r = random(100);
    if (r < 33) soundMouse();
    else if (r < 66) soundBirdOriginal();
    else soundTrill();
}

// Спокойная игра — только мышь
void soundCalm() {
    //soundMouse();
    soundBirdOriginal();

}

// Остальные функции-заглушки (если нужны)
void beepSweep(int startFreq, int endFreq, int durMs, int steps) {}
void soundBirdVariant(int v) {}


void taskSound(void* pv) {
    while(!systemReady) vTaskDelay(pdMS_TO_TICKS(10));
    randomSeed(analogRead(0));
    
    uint32_t lastSoundTime = 0;  // <--- ЗАЩИТА ОТ ЧАСТЫХ ВЫЗОВОВ
    bool lastActivePlay = false;
    bool lastCatDetected = false;
    
    for(;;) {
        SharedState st;
        xSemaphoreTake(gStateMutex, portMAX_DELAY);
        st = gState;
        xSemaphoreGive(gStateMutex);
        
        if(st.mode == MODE_CALIB || st.powerSave || !st.catDetected) {
            noTone(SPEAKER_PIN);
            vTaskDelay(pdMS_TO_TICKS(500));
            lastActivePlay = false;
            lastCatDetected = false;
            continue;
        }
        
        uint32_t now = millis();
        
        // Звук ТОЛЬКО при смене состояния И не чаще раза в 3 секунды
        if(st.activePlay && !lastActivePlay && (now - lastSoundTime > 3000)) {
            soundActivePlay();
            lastSoundTime = now;
            vTaskDelay(pdMS_TO_TICKS(100));
        } 
        else if(st.catDetected && !lastCatDetected && !st.activePlay && (now - lastSoundTime > 3000)) {
            soundCalm();
            lastSoundTime = now;
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        
        lastActivePlay = st.activePlay;
        lastCatDetected = st.catDetected;
        
        vTaskDelay(pdMS_TO_TICKS(500));  // Проверяем состояние чаще
    }
}