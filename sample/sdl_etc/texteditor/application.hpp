#pragma once
#include "deliting_object.hpp"
#include <SDL.h>
#include <vector.h>

class Widget;

class Application
{
  friend class Widget;
public:
  Application(int &argc, char **argv);
  ~Application();
  template <typename T>
  void queueDelete(T *obj)
  {
    deletingObjects_.push_back(new DeletingObject<T>(obj));
  }
  int exec();
  static Application *instance();
private:
  static Application *instance_;
  eastl::vector<Widget *> widgetList_;
  Widget *focusWidget_;
  Widget *needUpdateWithoutRedraw_;
  eastl::vector<BaseDeletingObject *> deletingObjects_;
  Uint32 lastUpdate_;
  void addWidget(Widget *);
  void removeWidget(Widget *);
  Widget *widgetByWindowId(Uint32);
  void setFocusWidget(Widget *);
  Widget *focusWidget() const;
  void clearFocus();
};
