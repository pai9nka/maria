#include "SoundPlayer.h"

// Инициализация переменных через список инициализации
SoundPlayer::SoundPlayer(uint8_t pin)
  : _pin(pin), _activeType(SOUND_OFF), _step(0), _totalSteps(0), _nextEventTime(0), _currentFreq(0) {}

void SoundPlayer::begin() {
    ledcAttach(_pin, 1000, 8); // Настройка ШИМ для ESP32
}

void SoundPlayer::_playTone(unsigned int freq, int vol) {
    if (vol <= 0 || freq == 0) {
        ledcWrite(_pin, 0);
    } else {
        // Ограничиваем громкость до 50% скважности (127) для безопасности
        int duty = map(vol, 0, 100, 0, 127);
        ledcWriteTone(_pin, freq);
        ledcWrite(_pin, duty);
    }
}

void SoundPlayer::play(SoundType type) {
    if (_activeType != SOUND_OFF) return; // Не прерывать текущий звук
    _activeType = type;
    _step = 0;
    _nextEventTime = millis();

    // Настройка параметров для разных типов
    if (type == SOUND_MOUSE) _totalSteps = random(2, 5);
    else if (type == SOUND_BIRD) _totalSteps = 20;
    else if (type == SOUND_CLICK) _totalSteps = random(5, 15);
}

void SoundPlayer::update(int volume) {
    if (_activeType == SOUND_OFF) return;
    if (millis() < _nextEventTime) return;

    switch (_activeType) {
        case SOUND_MOUSE:
            _playTone(random(5000, 9000), volume);
            _nextEventTime = millis() + 60;
            _activeType = SOUND_PAUSE; // Переходим в паузу между писками
            break;

        case SOUND_PAUSE:
            _playTone(0, 0);
            _nextEventTime = millis() + random(50, 120);
            _activeType = SOUND_MOUSE; // Возвращаемся к писку
            _step++;
            if (_step >= _totalSteps) _activeType = SOUND_OFF;
            break;

        case SOUND_BIRD:
            if (_step == 0) _currentFreq = random(6000, 9000);
            if (_step < _totalSteps) {
                _playTone(_currentFreq, volume);
                _currentFreq -= 150;
                _nextEventTime = millis() + 15;
                _step++;
            } else { _activeType = SOUND_OFF; }
            break;

        case SOUND_CLICK:
            if (_step < _totalSteps) {
                _playTone(random(3000, 5000), volume);
                _nextEventTime = millis() + random(30, 70);
                _step++;
            } else { _activeType = SOUND_OFF; }
            break;

        default:
            _activeType = SOUND_OFF;
            break;
    }

    if (_activeType == SOUND_OFF) _playTone(0, 0);
}

bool SoundPlayer::isPlaying() { return _activeType != SOUND_OFF; }

