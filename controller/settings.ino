#include "settings.h"
#include "constants.h"
#include <CueLightsCommon.h>
#include "strings.h"
#include <EEPROM.h>

Setting panelBrightness = {SETTING_PANEL_BRIGHTNESS, 100, 100, 10, VALUE};
Setting flashOnStandby = {SETTING_FLASH_STANDBY, 1, 1, 0, BOOL};
Setting standbyColor[3] = {
  {STR_RED, 255, 255, 0, VALUE},
  {STR_GREEN, 0, 255, 0, VALUE},
  {STR_BLUE, 0, 255, 0, VALUE}
};
Setting goColor[3] = {
  {STR_RED, 0, 255, 0, VALUE},
  {STR_GREEN, 255, 255, 0, VALUE},
  {STR_BLUE, 0, 255, 0, VALUE}
};
Setting stationBrightness[MAX_STATIONS];

//Setting settings[NUM_SETTINGS];

SettingsEEPROMData defaultSettings;

void InitSettings() {
  CreateSettingGroup(stationBrightness, MAX_STATIONS, SETTING_STATION_BRIGHTNESS, 100, 100, 10, VALUE);

  defaultSettings = settingsToEEEPROMDataHelper();
  recallSavedSettings();
}

static Setting* CreateSettingGroup(Setting* group, int count, const char* baseName, uint8_t defaultValue, uint8_t max, uint8_t min, SettingType type) {
  int len = 0;
  for (uint8_t i=0; i<count; i++) {
    char* nameBuf = new char[21];
    snprintf(nameBuf, 21, baseName, (i+1));
    group[i] = Setting {nameBuf, defaultValue, max, min, type};
  }
  return group;
}

SettingsEEPROMData settingsToEEEPROMDataHelper() {
  SettingsEEPROMData s;
  s.flashOnStandby = flashOnStandby.value;
  s.panelBrightness = panelBrightness.value;
  for (int i=0; i<3; i++) {
    s.standbyColor[i] = standbyColor[i].value;
    s.goColor[i] = goColor[i].value;
  }
  for (int i=0; i<MAX_STATIONS; i++) {
    s.stationBrightness[i] = stationBrightness[i].value;
  }
  return s;
}

void settingsFromEEPROMDataHelper(SettingsEEPROMData s) {
  if (s.flashOnStandby >= flashOnStandby.min && s.flashOnStandby <= flashOnStandby.max)
    flashOnStandby.value = s.flashOnStandby;
  if (s.panelBrightness >= panelBrightness.min && s.panelBrightness <= panelBrightness.max)
    panelBrightness.value = s.panelBrightness;
  for (int i=0; i<3; i++) {
    if (s.standbyColor[i] >= standbyColor[i].min && s.standbyColor[i] <= standbyColor[i].max)
      standbyColor[i].value = s.standbyColor[i];
    
    if (s.goColor[i] >= goColor[i].min && s.goColor[i] <= goColor[i].max)
      goColor[i].value = s.goColor[i];
  }
  for (int i=0; i<MAX_STATIONS; i++) {
    if (s.stationBrightness[i] >= stationBrightness[i].min && s.stationBrightness[i] <= stationBrightness[i].max)
      stationBrightness[i].value = s.stationBrightness[i];
  }
}

void resetDefaultSettings() {
  settingsFromEEPROMDataHelper(defaultSettings);
  saveSettings();
}

// EEPROM.put() uses update so we will only update settings that have changed
void saveSettings() {
  SettingsEEPROMData s = settingsToEEEPROMDataHelper();
  EEPROM.put(EEPROM_SETTINGS_ADDRESS, s);
  unsigned long crc = eeprom_crc();
  #ifdef DEBUG
    Serial.print("Saved settings to eeprom... CRC: ");
    Serial.println(crc);
  #endif
  EEPROM.put(EEPROM_CRC_ADDRESS, crc);
}

void recallSavedSettings() {
  SettingsEEPROMData s;
  EEPROM.get(EEPROM_SETTINGS_ADDRESS,s);
  unsigned long storedCrc = ~0L;
  EEPROM.get(EEPROM_CRC_ADDRESS, storedCrc);
  unsigned long calculatedCrc = eeprom_crc();
  #ifdef DEBUG
    Serial.print("Recalled settings from eeprom... stored CRC: ");
    Serial.print(storedCrc);
    Serial.print(", calculated CRC: ");
    Serial.println(calculatedCrc);
  #endif
  if (s.iteration == SETTINGS_ITERATION && storedCrc == calculatedCrc) {
    settingsFromEEPROMDataHelper(s);
  }
  else {
    // settings will be defaults at this point so just save them back over
    saveSettings();
  }
}

unsigned long eeprom_crc() {
  const unsigned long crc_table[16] = {
    0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
    0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
    0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
    0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
  };

  unsigned long crc = ~0L;

  for (int index = 0; index < sizeof(SettingsEEPROMData); ++index) {
    crc = crc_table[(crc ^ EEPROM[index]) & 0x0f] ^ (crc >> 4);
    crc = crc_table[(crc ^ (EEPROM[index] >> 4)) & 0x0f] ^ (crc >> 4);
    crc = ~crc;
  }
  return crc;
}