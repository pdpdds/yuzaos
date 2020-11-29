#ifndef SCENE_HPP
#define SCENE_HPP

#include <SDL.h>
#include <eastl/unique_ptr.h>

#include "context.hpp"

class Scene {
public:
  virtual bool Init(const Context& context) = 0;
  virtual void Tick(const Context &context) = 0;
  virtual void Cleanup(const Context &context) = 0;

  static void TickCurrent(const Context &context);
  template <class T> static void ChangeScene(const Context &context);

private:
  static eastl::unique_ptr<Scene> currentScene;
};

template <class T> 
void Scene::ChangeScene(const Context &context) {
	auto newScene = eastl::make_unique<T>();
	
	if (!newScene->Init(context))
		return;

	currentScene->Cleanup(context);
	currentScene = eastl::move(newScene);
}

#endif
