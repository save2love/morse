#ifndef CONFIG_H
#define CONFIG_H

#include <EEPROM.h>
#include <Arduino.h>

#define DEBUG        true
#define MemoryBase   0    // Where to store your config data in EEPROM
#define CONFIG_SIGN "XXX"

struct config_t
{
  char Sign[4];      // Mere detection of configuration in memory (XXX)
  bool IsBeep;       // Sound enabled or not (default is false)
  bool IsCapitalize; // Capitalise letters or not (default is true)
  uint8_t Language;  // 0 - English
} Config = {
  CONFIG_SIGN,
  false,
  true
};

template <class T> int ReadConfig(int ee, T& value)
{
  byte* p = (byte*)(void*)&value;
  unsigned int i;
  for (i = 0; i < sizeof(value); i++)
    *p++ = EEPROM.read(ee++);
  return i;
}

template <class T> int SaveConfig(int ee, const T& value)
{
  const byte* p = (const byte*)(const void*)&value;
  unsigned int i;
  for (i = 0; i < sizeof(value); i++)
    EEPROM.write(ee++, *p++);
  return i;
}

void printConfig() {
#ifdef DEBUG
  Serial.println(F("[CONFIG]"));
  Serial.print(F("  Is beep........: "));
  Serial.println(Config.IsBeep);
  Serial.print(F("  Is capitalize..: "));
  Serial.println(Config.IsCapitalize);
  Serial.print(F("  Language.......: "));
  switch (Config.Language) {
    case 0: Serial.println("English"); break;
    case 1: Serial.println("Russian"); break;
    default: Serial.println(); break;
  }
#endif
}

void LoadCurrentConfig() {
  config_t temp;
  ReadConfig(MemoryBase, temp);
  if (strcmp(temp.Sign, CONFIG_SIGN) == 0)
    Config = temp;
  printConfig();
}

void SaveCurrentConfig() {
  SaveConfig(MemoryBase, Config);
#ifdef DEBUG
  Serial.println(F("Config has been saved"));
  printConfig();
#endif
}

#endif
