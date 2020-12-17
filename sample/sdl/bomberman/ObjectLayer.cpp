//
//  ObjectLayer.cpp
//  SDL Game Programming Book
//
//  Created by shaun mitchell on 10/03/2013.
//  Copyright (c) 2013 shaun mitchell. All rights reserved.
//

#include "ObjectLayer.h"
#include "GameObject.h"
#include "Vector2D.h"
#include "Game.h"
#include "Camera.h"
#include "Level.h"

ObjectLayer::~ObjectLayer()
{
    for(std::vector<GameObject*>::iterator it = m_gameObjects.begin(); it != m_gameObjects.end(); ++it)// < m_gameObjects.size(); i++)
    {
        delete (*it);
    }
    m_gameObjects.clear();
}

void ObjectLayer::Update(Level* pLevel)
{
	std::vector<Player*>& players = pLevel->GetPlayers();
	for (int i = 0; i < players.size(); i++)
	{
		m_collisionManager.checkPlayerEnemyCollision(players[i], (const std::vector<GameObject*>&)m_gameObjects);

		// iterate through the objects
		if (!m_gameObjects.empty())
		{
			for (std::vector<GameObject*>::iterator it = m_gameObjects.begin(); it != m_gameObjects.end();)// < m_gameObjects.size(); i++)
			{
				if ((*it)->getPosition().getX() <= TheCamera::Instance()->getPosition().m_x + 640)
				{
					(*it)->setUpdating(true);
					(*it)->Update();
				}
				else
				{
					if ((*it)->type() != std::string("Player"))
					{
						(*it)->setUpdating(false);
					}
					else
					{
						(*it)->Update();
					}
				}

				// check if dead or off screen
				if ((*it)->dead() || (*it)->getPosition().m_y > 480)
				{
					std::cout << "deleting";
					delete * it;
					it = m_gameObjects.erase(it); // erase from vector and get new iterator
				}
				else
				{
					++it; // increment if all ok
				}

			}
		}}
}

void ObjectLayer::Render()
{
    for(int i = 0; i < m_gameObjects.size(); i++)
    {
    
        m_gameObjects[i]->Draw();

    }
}