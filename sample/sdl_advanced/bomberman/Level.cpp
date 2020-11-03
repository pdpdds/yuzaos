#include "Level.h"
#include "TextureManager.h"
#include "Game.h"
#include "Layer.h"
#include "TileLayer.h"
#include <math.h>
#include <iostream>
#include "Camera.h"
#include "GameObject.h"
#include "Level.h"
#include "Collision.h"
#include "Explosion.h"
#include "Bomb.h"
#include "Item.h"
#include "Game.h"
#include "PathManager.h"
#include "BreakableLayer.h"
#include "MessageDispatcher.h"
#include "Messages.h"

Level::Level(int width, int height)
: m_mapWidth(width)
, m_mapHeight(height)
{
	m_pPathManager = new PathManager<PathPlanner>(30);
	m_pMap = new GameMap();
}

Level::~Level()
{
    for(int i = 0; i < m_layers.size(); i++)
    {
        delete m_layers[i];
    }
    
    m_layers.clear();

	for (int i = 0; i < m_players.size(); i++)
	{
		delete m_players[i];
	}

	m_players.clear();

	delete m_pPathManager;
	delete m_pMap;
}

void Level::Render()
{
    for(int i = 0; i < m_layers.size(); i++)
    {
        m_layers[i]->Render();
    }

	for (int i = 0; i < m_gameObjects.size(); i++)
	{
		m_gameObjects[i]->Draw();
	}

	m_TriggerSystem.Render();

	for (int i = 0; i < m_players.size(); i++)
	{
		m_players[i]->Draw();
	}
}

void Level::Update()
{    
    for(int i = 0; i < m_layers.size(); i++)
    {
		m_layers[i]->Update(this);
    }

	m_pPathManager->UpdateSearches();

	UpdatePlayers();

	//CheckPlayerCollision();

	UpdateObject();

	m_TriggerSystem.Update(GetPlayers());
}

bool Level::AddPlayers(Player* pPlayer)
{
	if (m_players.size() == 0)
	{
		pPlayer->SetPossessed(true);	
		Vector2D& pos = pPlayer->getPosition();
		TheCamera::Instance()->setTarget(&pos);
	}
	else
		pPlayer->SetPossessed(false);

	m_players.push_back(pPlayer);
	return true;
}

bool Level::RemovePlayers(Player* pPlayer)
{
	std::vector<Player*>::iterator iter = m_players.begin();

	for (; iter != m_players.end(); iter++)
	{
		if (*iter == pPlayer)
		{
			m_players.erase(iter);
			delete pPlayer;
			return true;
		}
	}

	return false;
}

GameObject* Level::GetGameObjectFromID(int id)
{
	std::vector<Player*>::iterator iter = m_players.begin();

	for (; iter != m_players.end(); iter++)
	{
		if ((*iter)->ID() == id)
		{
			return *iter;
		}
	}

	std::vector<GameObject*>::iterator iter2 = m_gameObjects.begin();

	for (; iter2 != m_gameObjects.end(); iter++)
	{
		if ((*iter2)->ID() == id)
		{
			return *iter2;
		}
	}

	return NULL;
}

void Level::UpdatePlayers()
{
	std::vector<Player*>::iterator iter = m_players.begin();

	for (; iter != m_players.end(); iter++)
	{
		(*iter)->Update();
	}
}

bool Level::AddGameObject(GameObject* pObject)
{
	m_gameObjects.push_back(pObject);
	return true;
}

bool Level::AddReserveObject(GameObject* pObject)
{
	m_reserveAddObjects.push_back(pObject);
	return true;
}

void Level::UpdateObject()
{
	if (!m_gameObjects.empty())
	{
		for (std::vector<GameObject*>::iterator it = m_gameObjects.begin(); it != m_gameObjects.end();)// < m_gameObjects.size(); i++)
		{
			(*it)->setUpdating(true);
			(*it)->Update();

			if ((*it)->dead())
			{
				std::cout << "deleting";
				delete * it;
				it = m_gameObjects.erase(it); 
			}
			else
			{
				++it;
			}
		}
	}

	if (!m_reserveAddObjects.empty())
	{
		for (std::vector<GameObject*>::iterator it = m_reserveAddObjects.begin(); it != m_reserveAddObjects.end(); ++it)// < m_gameObjects.size(); i++)
		{
			m_gameObjects.push_back(*it);
		}

		m_reserveAddObjects.clear();
	}	
}

void Level::CheckPlayerCollision()
{
	std::vector<Player*> players = GetPlayers();
	for (int index = 0; index < players.size(); index++)
	{
		players[index]->CheckCollision(m_gameObjects);
	}
}

bool Level::CheckExplosionEffect(Explosion* pExplosion)
{
	bool isOverlappedBomb = false;
	for (int i = 0; i < m_gameObjects.size(); i++)
	{
		if (m_gameObjects[i]->type() == std::string("Explosion"))
			continue;

		SDL_Rect rect2;

		rect2.x = m_gameObjects[i]->getPosition().getX();
		rect2.y = m_gameObjects[i]->getPosition().getY();
		rect2.w = m_gameObjects[i]->getWidth();
		rect2.h = m_gameObjects[i]->getHeight();

		if (pExplosion->CheckCollision(m_gameObjects[i]))
		{
			m_gameObjects[i]->ProcessDeadAction();			
		}
	}


	//불꽃의 영역에 있는 트리거를 모두 없앤다.
	//std::vector<Explosion*> explosions;
	//explosions.push_back(pExplosion);

	//m_TriggerSystem.Update(explosions);

	return false;
}

void Level::GenerateItem(int tileRow, int tileColumn)
{
	

	int range = rand() % 100;

	if (range > 100)
		return ;

	int determinedItem = rand() % MAX_TRIGGER_TYPE;

	std::string triggerType;

	switch (determinedItem)
	{
	case TRIGGER_BOMB:
	{
		triggerType = "data/image/item/bombamount.png";
	}
	break;
	case TRIGGER_BOMBPOWER:
	{
		triggerType = "data/image/item/bombpower.png";
	}
	break;
	case TRIGGER_SPEEDUP:
	{
		triggerType = "data/image/item/speedup.png";
	}
	break;
	case TRIGGER_TNT:
	{
		triggerType = "data/image/item/powerup_special_big.png";
	}
	break;
	case TRIGGER_COIN10:
	{
		triggerType = "data/image/item/coins10.png";
	}
	break;
	case TRIGGER_COIN20:
	{
		triggerType = "data/image/item/coins20.png";
	}
	break;
	case TRIGGER_COIN30:
	{
		triggerType = "data/image/item/coins30.png";
	}
	break;
	}

	Item* pItem = new Item(determinedItem);
	pItem->SetOwner(this);
	pItem->SetCollidable(false);

	LoaderParams param = LoaderParams(tileColumn * 32, tileRow * 32, 32, 32, triggerType, 1, 0, 0);	
	pItem->Load(param);
	pItem->setCollisionLayers(&m_collisionLayers);	

	AddReserveObject(pItem);
}

void Level::AddTrigger(int x, int y, std::string& triggerType)
{
	Trigger<GameObject>* pTrigger = m_TriggerGenerator.GenerateItemTrigger(x, y);

	if (pTrigger)
		m_TriggerSystem.Register(pTrigger);
}

bool Level::isPathObstructed(Vector2D A,
	Vector2D B,
	double    BoundingRadius)const
{
	Vector2D ToB = Vec2DNormalize(B - A);

	Vector2D curPos = A;

	while (Vec2DDistanceSq(curPos, B) > BoundingRadius*BoundingRadius)
	{
		//advance curPos one step
		curPos += ToB * 0.5 * BoundingRadius;

		//test all walls against the new position
		//if (doWallsIntersectCircle(m_pMap->GetWalls(), curPos, BoundingRadius))
		{
			return true;
		}
	}

	return false;
}

bool Level::AddNavigationNode(int y, int x)
{
	m_pMap->AddNavigationNode(y, x);

	return true;
}

void Level::GenerateNavigationGraph()
{
	std::vector<Vector2D> outVec;
	std::vector<std::vector<int> > naviIndex;

	int width = m_mapWidth / 32;
	int height = m_mapHeight / 32;

	std::vector<int> layerRow(width);
	for (int j = 0; j < height; j++)
	{
		naviIndex.push_back(layerRow);
	}

	std::vector<TileLayer*> layers = *getCollisionLayers();

	for (int q = 0; q < width; q++)
		for (int w = 0; w < height; w++)
			for (int i = 0; i <layers.size(); i++)
			{
				std::vector<std::vector<int> > tileIds = layers[i]->getTileIDs();

				if (naviIndex[w][q] == 0)
					naviIndex[w][q] = tileIds[w][q];
			}

	for (int q = 0; q < width; q++)
		for (int w = 0; w < height; w++)
		{
			if (naviIndex[w][q] == 0)
			{
				Vector2D vec;
				vec.m_x = q * 32 + 16;
				vec.m_y = w * 32 + 16;
				outVec.push_back(vec);
			}
		}

	m_pMap->CreateNaviGraph(outVec);
}

Vector2D Level::GetRandomItemPos(int iItemToGet)
{
	/*double ClosestSoFar = MaxDouble;
	const TriggerSystem<Trigger<GameObject>>::TriggerList& triggers = m_TriggerSystem.GetTriggers();

	GameMap::TriggerSystem::TriggerList::const_iterator it;
	for (it = triggers.begin(); it != triggers.end(); ++it)
	{
		//    if ( ((*it)->EntityType() == GiverType) && (*it)->isActive())
		if ((*it)->isActive())
		{
			return (*it)->Center();
		}
	}*/
	std::vector<GameObject*>::iterator iter = m_gameObjects.begin();

	for (; iter != m_gameObjects.end(); iter++)
	{
		if ((*iter)->type() == "Item")
		{
			return (*iter)->Center();
		}
	}

	return Vector2D(0, 0);
}

Vector2D Level::GetCloseWallPos(Vector2D& source)
{
	BreakableLayer* pLayer = NULL;
	for (int i = 0; i < m_collisionLayers.size(); i++)
	{
		if(m_collisionLayers[i]->IsBreakTile())
		{
			pLayer = static_cast<BreakableLayer*>(m_collisionLayers[i]);
			break;
		}
	}

	//if (pLayer == NULL)
		return Vector2D(0, 0);
}

void Level::CreateExplosion(GameObject* pGameObject, int bombPower, int bombType)
{
	Explosion* pExplosion = static_cast<Explosion*>(TheGameObjectFactory::Instance()->create("Explosion"));
	pExplosion->SetOwner(this);
	LoaderParams param = LoaderParams(pGameObject->getPosition().getX(), pGameObject->getPosition().getY(), 32, 32, "data/image/object/explosion.bmp", 8, 0, 0);
	pExplosion->Load(param);
	pExplosion->setCollisionLayers(&m_collisionLayers);
	pExplosion->MakeFlame(bombPower, bombType);

	AddReserveObject(pExplosion);

	for (int i = 0; i < m_players.size(); i++)
	{
		Dispatcher->DispatchMsg(SEND_MSG_IMMEDIATELY,
			SENDER_ID_IRRELEVANT,
			m_players[i]->ID(),
			Msg_NavigationChanged,
			NO_ADDITIONAL_INFO);
	}

	

	
}