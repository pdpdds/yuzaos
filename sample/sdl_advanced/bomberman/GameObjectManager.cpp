//#include "stdafx.h"
#include "GameObjectManager.h"
#include "GameObject.h"

GameObjectManager::GameObjectManager()
{
}

GameObjectManager::~GameObjectManager()
{
}

GameObjectManager* GameObjectManager::Instance()
{
	static GameObjectManager instance;

	return &instance;
}

GameObject* GameObjectManager::GetGameObjectFromID(int id)const
{
	GameObjectMap::const_iterator ent = m_GameObjectMap.find(id);

	if (ent != m_GameObjectMap.end())	
		return ent->second;

	return NULL;
}

void GameObjectManager::RemoveGameObject(GameObject* pGameObject)
{
	m_GameObjectMap.erase(m_GameObjectMap.find(pGameObject->ID()));
}

void GameObjectManager::RegisterGameObject(GameObject* pGameObject)
{
	//m_GameObjectMap.insert(std::make_pair(pGameObject->ID(), pGameObject));
	m_GameObjectMap[pGameObject->ID()] = pGameObject;
}