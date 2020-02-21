// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#ifndef MENU_H
#define MENU_H

#include <stdint.h>
#include <stdarg.h>
#include "lcd_screen.h"
#include "settings.h"
#include <Encoder.h>

#define MAX_MENU_ITEMS 255
#define MAX_NAME_LENGTH 20

#define MENU_ITEM_PADDING 2 //px


enum MenuItemType {
  MISC,
  MENU,
  BUTTON,
  BUTTON_BACK,
};

class Menu;

class MenuItemBase {
  public:
    MenuItemBase(const char* nameInpt, bool showNum = true);

    // virtual methods
    // override both of these if your item has a custom appearance
    virtual void renderItem(LCDScreen* screen, int16_t yPos, bool selected = false);
    virtual uint16_t getHeight(LCDScreen*);

    virtual void dispatchPress() { /*do nothing*/ };
    virtual MenuItemType getItemType() { return MISC; };
    virtual bool getSelectable() { return true; };

    virtual void setParentMenu(Menu* menu) {/* do nothing*/};
    virtual bool requiresUpdate() {return false;};

    // getters and setters:
    const char* getName() {return name; };
    void setIndex(uint8_t newIndex) { index = newIndex; };
    uint8_t getIndex() { return index; };
    bool getShowIndex() { return showIndex; };

  protected:
    const char* name;
    uint8_t index = 0;
    bool showIndex = true;
};

class MenuItem : public MenuItemBase {
  public:
    MenuItem(const char* nameInpt, bool showIndex = true, void (*onPressCb)() = NULL);
    MenuItem(const char* nameInpt, void (*onPressCb)());

    bool getIsPressable();
    void dispatchPress();
    virtual MenuItemType getItemType() { return BUTTON; };
  private:
    // callback function
    void (*onPress)();
};

class BackButton: public MenuItemBase {
  public:
    BackButton(void (*onPressCB)() = NULL):MenuItemBase(STR_BACK_BUTTON, /*showIndex =*/false) { onPress = onPressCB; };
    virtual MenuItemType getItemType() { return BUTTON_BACK; };
    virtual void dispatchPress() { if (onPress != NULL) onPress(); };
  private:
    void (*onPress)();
};

class Divider: public MenuItemBase {
  public:
    Divider():MenuItemBase("-", /*showIndex =*/false) { /* empty*/ };
    virtual bool getSelectable() { return false; };
    virtual MenuItemType getItemType() { return MISC; };
    virtual void renderItem(LCDScreen* screen, int16_t yPos, bool selected = false);
    virtual uint16_t getHeight(LCDScreen*);
};

class HeaderItem: public MenuItemBase {
  public:
    HeaderItem(const char* nameInpt, const char* actionButtonStr, void (*onPressCb)() = NULL, bool isBackButton_in = false);
    virtual bool getSelectable() { return true; };

    virtual void renderItem(LCDScreen* screen, int16_t yPos, bool selected = false);
    virtual uint16_t getHeight(LCDScreen*);
    virtual MenuItemType getItemType() { return isBackButton?BUTTON_BACK:MISC; };
    bool getIsPressable();
    void dispatchPress();

  private:
    static const int extraPadding = 12;
    bool isBackButton = false;
    const char* actionButton;
    // callback function
    void (*onPress)();
};

class Menu: public MenuItemBase {
  public:
    //Menu(const char* nameInpt, uint8_t count, ...);
    Menu(const char* nameInpt, uint8_t count, MenuItemBase** itemsIn);

    Menu(const char* nameInpt, uint8_t count, ...)
    : Menu(count, nameInpt, (va_start(ap_, count), ap_)) { va_end(ap_); };

    //Menu(const char* nameInpt, uint8_t count ...);

    void bindToScreen(LCDScreen* scrn);

    virtual MenuItemType getItemType() { return MENU; };
    virtual MenuItemBase* getSelectedItem() { return items[selectedIndex]; };
    virtual void dispatchPress() { /*do nothing*/ };
    virtual void renderItem(LCDScreen* screen, int16_t yPos, bool selected = false);
    virtual void selectIndex(uint8_t index);
    virtual void selectIndex();
    void selectNext();
    void selectPrev();
    virtual void setParentMenu(Menu* menu);
    Menu* getParentMenu();
    virtual bool requiresUpdate();

  protected:
    // this is protected and has the argument order switched so that
    // menu constructions with just one argument, don't choose this constructor
    Menu(uint8_t count, const char* nameInpt, va_list ap);
    MenuItemBase** items;
    int selectedIndex = -1;
    LCDScreen* screen;
    uint16_t length = 0;
  private:
    bool updateRequired = false;
    va_list ap_;
    Menu* parentMenu = NULL;
    uint16_t scrollPos = 0;
    void drawAtPos(int16_t scrollPos);
};

enum ScreenPositionX {
  LEFT,
  CENTER,
  RIGHT
};

enum ScreenPositionY {
  TOP,
  MIDDLE,
  BOTTOM
};

struct ScreenPosition {
  ScreenPositionX x;
  ScreenPositionY y;
};

class FullScreenElement: public MenuItemBase {
  public:
    FullScreenElement(const char* nameInpt, 
                      ScreenPosition position_in, 
                      bool selectable = false,
                      void (*getItemNameFn)(const char*) = NULL,
                      MenuItemBase* child_in = NULL);
    MenuItemBase* getChild() { return child; };
    virtual MenuItemType getItemType();
    virtual void dispatchPress();
    virtual void renderItem(LCDScreen* screen, int16_t yPos, bool selected = false);
    virtual void setParentMenu(Menu* menu);
    virtual bool getSelectable() { return canSelect; };
  protected:
    MenuItemBase* child;
  private:
    void (*getItemName)(const char*);
    ScreenPosition position;
    bool canSelect = false;
};


class FullScreenDisplay: public Menu {
public:

  FullScreenDisplay(const char* nameInpt,int defaultSelection, uint8_t count, ...)
    : Menu(count, nameInpt, (va_start(ap_, count), ap_)) { va_end(ap_); selectedIndex = defaultSelection; };

  virtual void dispatchPress() { /*do nothing*/ };
  virtual MenuItemBase* getSelectedItem();
  virtual void renderItem(LCDScreen* screen, int16_t yPos, bool selected = false) { MenuItemBase::renderItem(screen, yPos, selected); };
  virtual void selectIndex(uint8_t index);
  virtual void selectIndex() { selectIndex(selectedIndex); };
  virtual bool requiresUpdate();

private:
  va_list ap_;
};

class SettingChanger: public FullScreenElement {
public:
  SettingChanger::SettingChanger(Setting* setting_in, Encoder* encoder_in);
  virtual void renderItem(LCDScreen* screen, int16_t yPos, bool selected = false);
  virtual bool requiresUpdate();
  virtual void dispatchPress();
private:
  long offset = 0;
  bool offsetValid = false;
  Setting* setting;
  Encoder* encoder;
};

class NavigationController {
  public:
    NavigationController(LCDScreen* scrn, Menu* topMenu);
    void dispatchPress();

    void selectNext();
    void selectPrev();

    void goHome();

    void display();

    void tick();
  private:
    LCDScreen* screen;
    Menu* currentMenu;
};

#endif
