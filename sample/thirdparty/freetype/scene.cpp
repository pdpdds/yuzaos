#include "scene.hpp"

#include "menu_scene.hpp"

eastl::unique_ptr<Scene> Scene::currentScene = eastl::make_unique<MenuScene>();

void Scene::TickCurrent(const Context &context) {
	if (currentScene == nullptr)
		return;
	currentScene->Tick(context);
}