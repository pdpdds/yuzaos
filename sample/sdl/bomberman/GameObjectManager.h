#pragma once
#include <map>

class GameObject;

class GameObjectManager
{
	typedef std::map<int, GameObject*> GameObjectMap;
public:
	GameObjectManager();
	virtual ~GameObjectManager();

	static GameObjectManager* Instance();

	void            RegisterGameObject(GameObject* pGameObject);

	GameObject* GetGameObjectFromID(int id)const;

	void            RemoveGameObject(GameObject* pGameObject);
	
	void            Reset(){ m_GameObjectMap.clear(); }

private:
	GameObjectMap m_GameObjectMap;

	GameObjectManager(const GameObjectManager&);
	GameObjectManager& operator=(const GameObjectManager&);

	
};

