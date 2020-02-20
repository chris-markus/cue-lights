// =====================================================
// Copyright (C) 2020 Christopher Markus 
// www.chrismarkus.me
// This software is licensed under the GNU GPLv3 License
// =====================================================

#include "menu.h"
#include <stdarg.h>

#include "Arduino.h"

// =====================================================
// MenuItemBase
// =====================================================
MenuItemBase::MenuItemBase(const char* nameInpt, bool showNum) {
  name = nameInpt;
  showIndex = showNum;
}

void MenuItemBase::renderItem(LCDScreen* screen, int16_t yPos, bool selected) {
  int16_t yStart = yPos;
  
  yPos += MENU_ITEM_PADDING + 1;
  int16_t x,y;
  uint16_t w,h;
  screen->getTextBounds(name,MENU_ITEM_PADDING,0,&x,&y,&w,&h);
  screen->setCursor(MENU_ITEM_PADDING,yPos);
  
  uint16_t textColor = WHITE;
  if (selected){
    textColor = BLACK;
    uint16_t height = 2*MENU_ITEM_PADDING + 1 + h;
    screen->fillRect(0, yStart, screen->width(), height, WHITE);
  }
  else {
    screen->drawFastHLine(0, yStart, screen->width(), WHITE);
  }
  
  screen->setTextColor(textColor);
  if (showIndex) {
    char tmp[MAX_NAME_LENGTH];
    sprintf(tmp, "%i. %s", index, name);
    screen->print(tmp);
  }
  else {
    screen->print(name);
  }
  
  yPos += h + MENU_ITEM_PADDING;
  
  if (!selected) {
    screen->drawFastHLine(0,yPos,screen->width(),WHITE);
  }
}

uint16_t MenuItemBase::getHeight(LCDScreen* screen) {
  /* 
  *  height calculation:
  *  top border
  *  padding
  *  text
  *  padding
  */
  int16_t x,y;
  uint16_t w,h;
  screen->getTextBounds(name,MENU_ITEM_PADDING,0,&x,&y,&w,&h);
  h += MENU_ITEM_PADDING * 2 + 1;
  return h;
}

// =====================================================
// MenuItem
// =====================================================

MenuItem::MenuItem(const char* nameInpt, bool showIndex, void (*onPressCb)())
  : MenuItemBase(nameInpt, showIndex) {
  onPress = onPressCb;
}

MenuItem::MenuItem(const char* nameInpt, void (*onPressCb)())
  : MenuItemBase(nameInpt) {
  onPress = onPressCb;
}

bool MenuItem::getIsPressable() {
  return onPress != NULL;
}

void MenuItem::dispatchPress() {
  if (onPress != NULL) {
    onPress();
  }
}

// =====================================================
// HeaderItem
// =====================================================

HeaderItem::HeaderItem(const char* nameInpt, const char* actionButtonStr, void (*onPressCb)(), bool isBackButton_in)
: MenuItemBase(nameInpt, /*showIndex = */false) {
  actionButton = actionButtonStr;
  isBackButton = isBackButton_in;
  onPress = onPressCb;
}

void HeaderItem::renderItem(LCDScreen* screen, int16_t yPos, bool select = false) {
  screen->setTextColor(WHITE);
  int16_t x,y;
  uint16_t w,h;
  screen->setCursor(MENU_ITEM_PADDING, yPos + MENU_ITEM_PADDING);
  screen->getTextBounds(actionButton, 
                        MENU_ITEM_PADDING, 
                        yPos + MENU_ITEM_PADDING, &x, &y, &w, &h);
  if (select) {
    screen->setTextColor(BLACK);
    screen->fillRect(0, yPos, w + MENU_ITEM_PADDING + 1, h + MENU_ITEM_PADDING + 1, WHITE);
  }
  screen->print(actionButton);
  screen->setTextColor(WHITE);
  yPos += MENU_ITEM_PADDING + extraPadding;
  screen->getTextBounds(name,MENU_ITEM_PADDING,0,&x,&y,&w,&h);
  screen->printCentered(name, false, yPos);
  yPos += h + MENU_ITEM_PADDING;
  screen->drawFastHLine(0,yPos,screen->width(),WHITE);
}

uint16_t HeaderItem::getHeight(LCDScreen* screen) {
  int16_t x,y;
  uint16_t w,h;
  screen->getTextBounds(name,MENU_ITEM_PADDING,0,&x,&y,&w,&h);
  h += MENU_ITEM_PADDING * 2 + extraPadding;
  return h;
}

bool HeaderItem::getIsPressable() {
  return onPress != NULL;
}

void HeaderItem::dispatchPress() {
  if (onPress != NULL) {
    onPress();
  }
}

// =====================================================
// Divider
// =====================================================

void Divider::renderItem(LCDScreen* screen, int16_t yPos, bool select = false) {
  screen->drawFastHLine(0,yPos,screen->width(),WHITE);
  screen->drawFastHLine(0,yPos+MENU_ITEM_PADDING*2,screen->width(),WHITE);
}

uint16_t Divider::getHeight(LCDScreen* screen) {
  return 2*MENU_ITEM_PADDING;
}

// =====================================================
// Menu
// =====================================================

Menu::Menu(uint8_t count, const char* nameInpt, va_list ap)
: MenuItemBase(nameInpt) {
  int currentIndex = 0;
  items = new MenuItemBase*[count];
  for (uint8_t i=0; i < count; i++) {
      MenuItemBase* tempPtr = va_arg(ap, MenuItemBase*);
      // doing it this way protects us a little more
      if (tempPtr != NULL) {
        items[length] = tempPtr;

        if (items[length]->getShowIndex()) {
          currentIndex++;
          items[length]->setIndex(currentIndex);
        }
        if (selectedIndex != -1 && items[length]->getSelectable()) {
          selectedIndex = length;
        }

        // if the item is a menu, set this menu as its parent
        if (items[length]->getItemType() == MENU) {
          ((Menu*)(items[length]))->setParentMenu(this);
        }

        length++;
      }
      else {
          break;
      }
  }
  // only as a last resort
  if (selectedIndex == -1) selectedIndex = 0;
}

void Menu::bindToScreen(LCDScreen* scrn) {
  screen = scrn;
}

// add a right-justified triangle to indicate a submenu
void Menu::renderItem(LCDScreen* screen, int16_t yPos, bool selected) {
  const int16_t triangleSize = 6;
  // 8 is the text height
  int16_t padding = MENU_ITEM_PADDING + (8 - triangleSize)/2;
  int16_t startPos = yPos;
  MenuItemBase::renderItem(screen, yPos, selected);
  uint16_t color = WHITE;
  if (selected)
    color = BLACK;
  screen->fillTriangle(screen->width() - triangleSize/2 - padding, startPos + padding,
                        screen->width() - padding , startPos + padding + triangleSize/2,
                        screen->width() - triangleSize/2 - padding, startPos + padding + triangleSize,
                        color);
}

// show it in the middle of the screen if not at the top or bottom
// TODO: could do this without calculating the total offset each time
void Menu::selectIndex(uint8_t index) {
  // if the item isn't selectable go to the next or last depending on the context
  if (!items[index]->getSelectable()) {
    if (selectedIndex > index) {
      if (index > 0)
        index--;
      else if(index < length-1)
        index++;
    }
    else if (selectedIndex <= index) {
      if (index < length-1)
        index++;
      else if (index > 0)
        index--;
    }
  }
  if (index >= 0 && index < length) {
    selectedIndex = index;
    int16_t offset = 0;
    for (int i=0; i<selectedIndex; i++) {
      offset -= items[i]->getHeight(screen);
    }
    int16_t offsetTot = offset;
    for (int i=selectedIndex; i<length; i++) {
      offsetTot-= items[i]->getHeight(screen);
    }
    // if we are at the top or bottom of the list don't scroll any farther
    int16_t thisHeight = items[selectedIndex]->getHeight(screen);
    if (offset*-1 < screen->height()/2 - thisHeight/2) {
      offset = 0;
    }
    else if (offsetTot*-1 > screen->height() 
             && offset < offsetTot + screen->height()/2 + thisHeight/2) {
      offset = offsetTot + screen->height();
    }
    else {
      offset += screen->height()/2 - items[selectedIndex]->getHeight(screen)/2;
    }
    drawAtPos(offset);
  }
}

void Menu::selectIndex() {
  selectIndex(selectedIndex);
}

void Menu::selectNext() {
  // this is controlled for in the selectIndex function too but this avoids an overflow
  if (selectedIndex < length - 1) {
    selectIndex(selectedIndex+1);
  }
}

void Menu::selectPrev() {
  if (selectedIndex > 0) {
    selectIndex(selectedIndex-1);
  }
}

void Menu::setParentMenu(Menu* parent) {
  parentMenu = parent;
  if (parentMenu != NULL && length > 0 
      && length < MAX_MENU_ITEMS - 1 
      && items[0]->getItemType() != BUTTON_BACK)
  {
    for (int i=length; i>0;i--) {
      items[i] = items[i-1];
    }
    length++;
    items[0] = new HeaderItem(name, STR_BACK_BUTTON ,NULL, true);//new BackButton();
  }
}

Menu* Menu::getParentMenu() {
  return parentMenu;
}

bool Menu::requiresUpdate() {
  if (updateRequired) {
    updateRequired = false;
    return true;
  }
  return false;
}

// --------------------------------
// Private:

void Menu::drawAtPos(int16_t scrollPos) {
  screen->clear();
  int16_t yOffset = scrollPos;
  for (int i=0; i<length; i++) {
    uint16_t itemHeight = items[i]->getHeight(screen);
    if (yOffset + itemHeight >= 0 && yOffset <= screen->height()) {
      bool selected = selectedIndex == i;
      items[i]->renderItem(screen,yOffset,selected);
      // if any onscreen items require an update, set the flag
      updateRequired = updateRequired | items[i]->requiresUpdate();
    }
    yOffset += itemHeight;
  }
  screen->display();
}

// =====================================================
// FullScreenDisplay
// =====================================================

void FullScreenDisplay::selectIndex(uint8_t index) {
  screen->clear();
  if (items[index]->getSelectable()) {
    selectedIndex = index;
  }
  for (int i=0; i<length; i++) {
    items[i]->renderItem(screen, 0, i == selectedIndex);
  }
  screen->display();
}

// only update once every 200ms
bool FullScreenDisplay::requiresUpdate() {
  static bool didUpdate = false;
  if (millis()%200 == 0) {
    if (!didUpdate) {
      didUpdate = true;
      return true;
    }
  }
  else {
    didUpdate = false;
  }
  return false;
}

// =====================================================
// ScreenElement
// =====================================================

FullScreenElement::FullScreenElement(const char* nameInpt, 
                                     ScreenPosition position_in, 
                                     MenuItemBase* child_in, 
                                     bool selectable,
                                     void (*getItemNameFn)(const char*))
: MenuItemBase(nameInpt, /*showNum =*/false) {
  canSelect = selectable;
  getItemName = getItemNameFn;
  child = child_in;
  position = position_in;
}

void FullScreenElement::renderItem(LCDScreen* screen, int16_t yPos, bool selected) {
  int16_t x,y;
  uint16_t w,h;
  x = 0;
  y = 0;

  char tempName[40];
  if (getItemName == NULL) {
    strcpy(tempName, name);
  }
  else {
    getItemName(tempName);
  }

  screen->getTextBounds(tempName,0,0,&x,&y,&w,&h);

  switch (position.x) {
    case LEFT:
      x = MENU_ITEM_PADDING;
      break;
    case RIGHT:
      x = screen->width() - w - MENU_ITEM_PADDING;
      break;
    case CENTER:
      x = (screen->width() - w)/2;
  }

  switch(position.y) {
    case TOP:
      y = MENU_ITEM_PADDING;
      break;
    case BOTTOM:
      y = screen->height() - h - MENU_ITEM_PADDING;
      break;
    case MIDDLE:
      y = (screen->height() - h)/2;
  }

  if(selected) {
    screen->fillRect(x - MENU_ITEM_PADDING, 
                     y - MENU_ITEM_PADDING, 
                     w+2*MENU_ITEM_PADDING, 
                     h+2*MENU_ITEM_PADDING, 
                     WHITE);
    screen->setTextColor(BLACK);
  }
  else {
    screen->setTextColor(WHITE);
  }
  screen->setCursor(x,y);
  screen->print(tempName);
}

void FullScreenElement::setParentMenu(Menu* menu) {
  if (child->getItemType()==MENU) {
    ((Menu*)(child))->setParentMenu(menu);
  }
}


// =====================================================
// NavigationController
// =====================================================

NavigationController::NavigationController(LCDScreen* scrn, Menu* topMenu) {
  screen = scrn;
  currentMenu = topMenu;
  currentMenu->bindToScreen(scrn);
  currentMenu->selectIndex();
}

void NavigationController::dispatchPress() {
  MenuItemBase* selectedItem = currentMenu->getSelectedItem();
  switch (selectedItem->getItemType()) {
    case MISC:
    case BUTTON:
      selectedItem->dispatchPress();
      break;
    case BUTTON_BACK:
      if (currentMenu->getParentMenu() != NULL) {
        currentMenu = currentMenu->getParentMenu();
        currentMenu->bindToScreen(screen);
        currentMenu->selectIndex();
      }
      break;
    case MENU:
      currentMenu = (Menu*)(selectedItem);
      currentMenu->bindToScreen(screen);
      currentMenu->selectIndex();
      break;
  }
}

void NavigationController::selectNext() {
  currentMenu->selectNext();
}

void NavigationController::selectPrev() {
  currentMenu->selectPrev();
}

void NavigationController::goHome() {
  while(currentMenu->getParentMenu() != NULL) {
    currentMenu = currentMenu->getParentMenu();
  }
  currentMenu->selectIndex();
}

void NavigationController::display() {
  currentMenu->selectIndex();
}

void NavigationController::tick() {
  // redraw if needed
  if (currentMenu->requiresUpdate()) {
    currentMenu->selectIndex();
  }
}