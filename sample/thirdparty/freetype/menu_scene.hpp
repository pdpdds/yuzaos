#ifndef MENU_SCENE_HPP
#define MENU_SCENE_HPP

#include "scene.hpp"
#include <SDL.h>

class MenuScene : public Scene {
public:
  virtual bool Init(const Context &context);
  virtual void Tick(const Context &context);
  virtual void Cleanup(const Context &context);
};

#endif