#ifndef SOUND_PLAYER_H
#define SOUND_PLAYER_H

#include <Arduino.h>

// Список доступных звуков
enum SoundType {
  SOUND_OFF = -1,
  SOUND_MOUSE = 0,
  SOUND_BIRD = 1,
  SOUND_CLICK = 2,
  SOUND_PAUSE = 3 // Внутреннее состояние паузы
};

class SoundPlayer {
  public:
    SoundPlayer(uint8_t pin); // Конструктор
    void begin();             // Инициализация (вызывать в setup)
    void play(SoundType type); // Запуск звука
    void update(int volume);  // Обработчик (вызывать в loop)
    bool isPlaying();         // Проверка: играет ли сейчас что-то

  private:
    uint8_t _pin;           // Номер GPIO пина, к которому подключен транзистор 2N2222
    SoundType _activeType;  // Текущий проигрываемый звук (или SOUND_OFF, если тишина)
    int _step;              // Счетчик текущего шага внутри серии (например, какой по счету писк)
    int _totalSteps;        // Общее количество шагов/повторений, запланированное для звука
    unsigned long _nextEventTime; // Время (в ms), когда нужно изменить частоту или выключить звук
    unsigned int _currentFreq;    // Текущая рабочая частота (используется для эффектов типа "свип")

    // Внутренний метод для прямой работы с ШИМ (инкапсулирует логику громкости)
    void _playTone(unsigned int freq, int vol);
};

#endif

