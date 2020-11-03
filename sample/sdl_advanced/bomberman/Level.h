#ifndef __LEVEL_H__
#define __LEVEL_H__

#include <iostream>
#include <vector>
#include "Layer.h"
#include "LevelParser.h"
#include "Player.h"
#include "CollisionManager.h"
#include "PathManager.h"
#include "TriggerGenerator.h"

class TileLayer;
class GameObject;
class Level;
class Player;
class Explosion;
class PathPlanner;

struct Tileset
{
    int firstGridID;
    int tileWidth;
    int tileHeight;
    int spacing;
    int margin;
    int width;
    int height;
    int numColumns;
    std::string name;

	Tileset(){
		firstGridID = 0;
		tileWidth = 0;
		tileHeight = 0;
		spacing = 0;
		margin = 0;
		width = 0;
		height = 0;
		numColumns = 0;
	}
};

class Level
{
public:
	Level(int width, int height);
	~Level();
    
	void Update();
    void Render();
    
    std::vector<Tileset>* getTilesets() { return &m_tilesets; }
    std::vector<Layer*>* getLayers() { return &m_layers; }    
    std::vector<TileLayer*>* getCollisionLayers() { return &m_collisionLayers; }

//로컬, 네트워크 플레이어를 추가하거나 제거한다.
	bool AddPlayers(Player* pPlayer);
	bool RemovePlayers(Player* pPlayer);
	std::vector<Player*>& GetPlayers(){ return m_players; }
	
//아이디로 부터 게임 오브젝트를 얻어낸다.
	GameObject* GetGameObjectFromID(int id);

//플레이어를 업데이트한다.
	void UpdatePlayers();

//게임 오브젝트를 업데이트한다.
	void UpdateObject();

//네비게이션 그래프를 생성한다.
	void GenerateNavigationGraph();

//네비게이션 그래프에 노드를 추가한다. 벽돌을 깨서 길이 이어질 경우 호출된다.
	bool AddNavigationNode(int x, int y);

//네비게이션 그래프에서 노드를 제거한다. 아직 구현되지 않음
	bool RemoveNavigationNode(int x, int y){ return false; }

	std::vector<GameObject*>* getGameObjects() { return &m_gameObjects; }

	bool AddGameObject(GameObject* pObject);
	bool AddReserveObject(GameObject* pObject);

//해당 타일에 아이템을 생성하는 로직을 수행
	void GenerateItem(int tileRow, int tileColumn);

//레벨에 트리거를 추가한다.
	void AddTrigger(int x, int y, std::string& triggerType);

//트리거 시스템
	TriggerSystem<Trigger<GameObject>>* GetTriggerSystem(){ return &m_TriggerSystem; }
	
//패스 매니저
	PathManager<PathPlanner>* const    GetPathManager(){ return m_pPathManager; }	

//플레이어와 게임 오브젝트와의 충돌을 체크하고 각 게임오브젝트 종류에 따른 처리를 수행한다.
	void CheckPlayerCollision();

//해당 게임 오브젝트의 위치에 주어진 변수로 폭발을 일으킨다.
	void CreateExplosion(GameObject* pGameObject, int bombPower, int bombType);

//폭탄 폭발시 레벨에 있는 아이템 및 벽돌과 폭발의 충돌을 체크
	bool CheckExplosionEffect(Explosion* pExplosion);

	GameMap* const GetMap(){ return m_pMap; }

//맘에 안드는 메소드
	Vector2D GetRandomItemPos(int iItemToGet);
	Vector2D GetCloseWallPos(Vector2D& source);
	
	bool isPathObstructed(Vector2D A, Vector2D B, double    BoundingRadius)const;
	
	Vector2D GetRandomSpawnPoint(){ return m_SpawnPoints[RandInt(0, m_SpawnPoints.size() - 1)]; }
	
private:
	std::vector<Player*> m_players;
    
    std::vector<Layer*> m_layers;
    std::vector<Tileset> m_tilesets;
    std::vector<TileLayer*> m_collisionLayers;

	std::vector<GameObject*> m_gameObjects;
	std::vector<GameObject*> m_reserveAddObjects;

	int m_mapWidth;
	int m_mapHeight;

	TriggerSystem<Trigger<GameObject>> m_TriggerSystem;
	TriggerGenerator m_TriggerGenerator;

	std::vector<Vector2D>  m_SpawnPoints;
	PathManager<PathPlanner>*  m_pPathManager;
	GameMap*  m_pMap;
};

#endif 
