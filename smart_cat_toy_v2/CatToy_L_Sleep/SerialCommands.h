/*
  ============================================================================
   ФАЙЛ: SerialCommands.h
   НАЗНАЧЕНИЕ: Обработка команд из Serial Monitor
  ============================================================================

   ПОДДЕРЖИВАЕМЫЕ КОМАНДЫ:
   ========================
   help              - показать список команд
   show              - показать текущие настройки
   save              - сохранить настройки в flash
   mode run/calib    - переключить режим
   set <параметр> <значение>
   pan_speed slow/normal/fast
   sound on/off
   sound_profile quiet/normal/loud
   test sound bird/mouse
   tilt_range <min> <max>
   tilt_speed <1..3>

  ============================================================================
*/

#ifndef SERIAL_COMMANDS_H
#define SERIAL_COMMANDS_H

#include <Arduino.h>
#include "SharedState.h"

// Внешние переменные
extern RuntimeConfig gCfg;
extern SharedState gState;
extern SemaphoreHandle_t gStateMutex;
extern String gSerialLine;

// ============================================================================
// ОСНОВНЫЕ ФУНКЦИИ
// ============================================================================

/**
 * @brief Вывести список всех доступных команд в Serial
 */
void printHelp();

/**
 * @brief Вывести текущую конфигурацию в Serial
 */
void printConfig();

/**
 * @brief Обработать одну команду (строку)
 * @param lineRaw строка с командой (без символов новой строки)
 */
void handleSerialCommand(const String& lineRaw);

/**
 * @brief Проверить наличие данных в Serial и обработать команды
 *
 * Вызывается из loop(). Накапливает входящие символы в буфер gSerialLine,
 * при получении \n или \r передаёт строку в handleSerialCommand().
 */
void processSerial();

#endif  // SERIAL_COMMANDS_H