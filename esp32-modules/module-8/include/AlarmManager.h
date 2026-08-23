#ifndef ALARM_MANAGER_H
#define ALARM_MANAGER_H

#include "ModuleManager.h"
#include "CommandHandler.h"
#include "AlarmSystem.h"
#include "BuzzerController.h"

class AlarmManager {
private:
  ModuleManager* moduleManager;
  BuzzerController buzzer;
  AlarmSystem alarmSystem;
  CommandHandler commandHandler;

  void publishStatus();

public:
  AlarmManager(ModuleManager* moduleMgr);

  void begin();
  void loop();

  void handleForceUpdate();
  CommandHandler& getCommandHandler() { return commandHandler; }
  AlarmSystem& getAlarmSystem() { return alarmSystem; }
  BuzzerController& getBuzzer() { return buzzer; }
};

#endif
