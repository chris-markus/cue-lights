// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#ifndef SETTINGS_H
#define SETTINGS_H

#define SETTING_PANEL_BRIGHTNESS "Panel Brightness"
#define SETTING_FLASH_STANDBY "Flash on Standby"

enum SettingType {
    BOOL,
    VALUE
};

/*class Setting {
public:
    Setting(const char* name_in, int defaultVal) { name = name_in; value = defaultVal; };
    Setting* getNext() {return next; };
    virtual int getVal() { return value; };
    virtual void setVal(int newVal) { value = newVal; };
    void adjust();
private:
    Setting* next = NULL;
    const char* name;
    int value;
};*/
struct Setting {
    const char* name;
    int value;
    int max;
    int min;
    SettingType type;
    Setting* next;
};

class Settings {
private:
    static Settings *instance;
    Setting* firstSetting;
    // Private constructor so that no objects can be created.
    Settings() { /*do nothing*/ }

    Setting* getLast();
    int count = 0;
public:
    static Settings *getInstance();
    void add(Setting* s);
    void save(Setting* s);
    Setting* getSettingWithName(const char* name);
};


#endif