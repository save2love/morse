#ifndef MENU_H
#define MENU_H

#include <String.h>

class MenuItem {
    friend class Menu;

  public:
    MenuItem(const char* itemName ) : name(itemName) {
      next = prev = sub = 0;
    }

    const char* getName() const {
      return name;
    }

    void setNext(MenuItem &m) {
      next = &m;
    }
    
    void setPrevious(MenuItem &m) {
      prev = &m;
    }
    
    void setSubMenu(MenuItem &m) {
      sub = &m;
    }
    
    void setBack(MenuItem &m) {
      back = &m;
    }

    MenuItem* getNext() const {
      return next;
    }
    
    MenuItem* getPrevious() const {
      return prev;
    }
    
    MenuItem* getSubItem() const {
      return sub;
    }
    
    MenuItem* getBack() const {
      return back;
    }

    MenuItem *moveBack() {
      return back;
    }

    MenuItem *moveNext() {
      /*if (next) {
        next->back = this;
      }*/
      return next;
    }

    MenuItem *movePrevious() {
      /*if (prev) {
        prev->back = this;
      }*/
      return prev;
    }

    MenuItem *moveSubItem() {
      /*if (sub) {
        sub->back = this;
      }*/
      return sub;
    }

    MenuItem &add(MenuItem &mi) {
      return addNext(mi);
    }

    MenuItem &addNext(MenuItem &mi) {
      mi.prev = this;
      mi.next = next;
      mi.back = this->getBack();
      next = &mi;
      return mi;
    }

    MenuItem &addPrevious(MenuItem &mi) {
      mi.prev = prev;
      mi.next = this;
      mi.back = this->getBack();
      prev = &mi;
      return mi;
    }

    MenuItem &addSubItem(MenuItem &mi) {
      mi.back = this;
      mi.next = &mi;
      mi.prev = &mi;
      sub = &mi;
      return mi;
    }

  public:
    bool operator==(const char* test) const {
      return String(getName()) == test;
    }

    bool operator==(const String& test) const {
      return String(getName()) == test;
    }

    bool operator==(const MenuItem &rhs) const {
      return String(getName()) == rhs.getName();
    }

  protected:
    const char* name;

    MenuItem *prev;
    MenuItem *next;
    MenuItem *sub;

    MenuItem *back;
};

struct MenuChangeEvent {
  const MenuItem &from;
  const MenuItem &to;
};

struct MenuUseEvent {
  const MenuItem &item;
};

typedef void (*cb_change)(MenuChangeEvent);
typedef void (*cb_use)(MenuUseEvent);
typedef MenuItem& MenuItemRef;

class Menu {
  public:
    Menu(const MenuItem &r, cb_use menuUse, cb_change menuChange = 0): root(r) {
      root.next = &root;
      root.prev = &root;
      root.back = &root;
      current = &root;

      cb_menuChange = menuChange;
      cb_menuUse = menuUse;
    }

    MenuItemRef getRoot() {
      return root;
    }

    MenuItemRef getCurrent() {
      return *current;
    }

    void moveBack() {
      setCurrent(current->getBack());
    }

    void moveNext() {
      setCurrent(current->moveNext());
    }

    void movePrevious() {
      setCurrent(current->movePrevious());
    }

    void moveSubItem() {
      setCurrent(current->moveSubItem());
    }

    void use() {
      if (cb_menuUse) {
        MenuUseEvent mue = { *current };
        cb_menuUse(mue);
      }
    }

    void toRoot() {
      setCurrent(&getRoot());
    }

  private:
    void setCurrent( MenuItem *next ) {
      if (next) {
        if (cb_menuChange) {
          MenuChangeEvent mce = { *current, *next };
          (*cb_menuChange)(mce);
        }
        current = next;
      }
    }

    MenuItem root;
    MenuItem *current;

    cb_change cb_menuChange;
    cb_use cb_menuUse;
};

#endif
