// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#ifndef MENU_H
#define MENU_H

#include <stdint.h>
#include "lcd_screen.h"

#define MAX_MENU_ITEMS 255
#define MAX_NAME_LENGTH 20

#define MENU_ITEM_PADDING 2 //px


enum MenuItemType {
  MISC,
  MENU,
  BUTTON,
  BUTTON_BACK,
};

class MenuItemBase {
  public:
    MenuItemBase(const char* nameInpt, bool showNum = true);

    // virtual methods
    virtual void renderItem(LCDScreen* screen, int16_t yPos, bool selected = false);
    virtual void dispatchPress() { /*do nothing*/ };
    virtual MenuItemType getItemType() { return MISC; };
    virtual bool getSelectable() { return true; };
    virtual uint16_t getHeight(LCDScreen*);

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
    BackButton():MenuItemBase(STR_BACK_BUTTON, /*showIndex =*/false) { /* empty*/ };
    virtual MenuItemType getItemType() { return BUTTON_BACK; };
};

class HeaderItem: public MenuItemBase {
  public:
    HeaderItem(const char* nameInpt):MenuItemBase(nameInpt, /*showIndex =*/false){ /* empty */ };
    virtual bool getSelectable() { return false; };

    virtual void renderItem(LCDScreen* screen, int16_t yPos, bool selected = false);
    virtual uint16_t getHeight(LCDScreen*);
};

class Menu: public MenuItemBase {
  public:
    Menu(const char* nameInpt, uint8_t count, ...);
    Menu(const char* nameInpt, uint8_t count, MenuItemBase** itemsIn);

    void bindToScreen(LCDScreen* scrn);

    virtual MenuItemType getItemType() { return MENU; };
    MenuItemBase* getSelectedItem() { return items[selectedIndex]; };
    virtual void dispatchPress() { /*do nothing*/ };

    virtual void renderItem(LCDScreen* screen, int16_t yPos, bool selected = false);
    
    void selectIndex(uint8_t index);
    void selectIndex();
    void selectNext();
    void selectPrev();
    void setParentMenu(Menu* menu);
    Menu* getParentMenu();
  private:
    Menu* parentMenu = NULL;
    void commonInit();
    LCDScreen* screen;
    uint16_t length = 0;
    uint8_t selectedIndex = 0;
    uint16_t scrollPos = 0;
    void drawAtPos(int16_t scrollPos);
    MenuItemBase* items[MAX_MENU_ITEMS];
};

class NavigationController {
  public:
    NavigationController(LCDScreen* scrn, Menu* topMenu);
    void dispatchPress();

    void selectNext();
    void selectPrev();

    void goHome();
  private:
    LCDScreen* screen;
    Menu* currentMenu;
};

#endif
