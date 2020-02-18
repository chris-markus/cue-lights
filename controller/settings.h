// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#ifndef SETTINGS_H
#define SETTINGS_H

#define DEFAULT_NUM_SETTINGS 10

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
};

class Settings {
private:
    static Settings *instance;
    Setting** settings;
    // Private constructor so that no objects can be created.
    Settings() { /*do nothing*/ }

    // we don't want to delete the setting objects because they may be used elsewhere
    ~Settings() { delete settings; delete instance; }

    int count = 0;
    int maxSettings = DEFAULT_NUM_SETTINGS;
public:
    static Settings *alloc(int num_settings = 0);
    static Settings *getInstance();
    bool add(Setting* s);
    void save(Setting* s);
    Setting* getSettingWithName(const char* name);
};


#endif