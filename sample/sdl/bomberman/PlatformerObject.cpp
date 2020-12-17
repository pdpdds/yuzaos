//
//  SDLGameObject.cpp
//  SDL Game Programming Book
//
//  Created by shaun mitchell on 19/01/2013.
//  Copyright (c) 2013 shaun mitchell. All rights reserved.
//

#include "PlatformerObject.h"
#include "TextureManager.h"
#include "Game.h"
#include "TileLayer.h"
#include "Level.h"
#include "Collision.h"

PlatformerObject::PlatformerObject() :    GameObject(),
                                    m_bulletFiringSpeed(0),
                                    m_bulletCounter(0),
                                    m_moveSpeed(0),
                                    m_dyingTime(0),
                                    m_dyingCounter(0),
                                    m_bPlayedDeathSound(false),
                                    m_bFlipped(false),                                    
                                    m_bRunning(false),
                                    m_lastSafePos(0,0)
{
}

void PlatformerObject::Load(LoaderParams& params)
{
    // get position
	m_position = Vector2D(params.getX(), params.getY());
	
    // get drawing variables
	m_width = params.getWidth();
	m_height = params.getHeight();
	m_textureID = params.getTextureID();
	m_numFrames = params.getNumFrames();
}

// draw the object to the screen
void PlatformerObject::Draw()
{
    TextureManager::Instance()->drawFrame(m_textureID, (Uint32)m_position.getX(), (Uint32)m_position.getY(),
                                          m_width, m_height, m_currentRow, m_currentFrame, TheGame::Instance()->getRenderer(), m_angle, m_alpha);
}

// apply velocity to current position
void PlatformerObject::Update()
{
    m_position += m_velocity;
    m_currentFrame = int(((SDL_GetTicks() / (1000 / 3)) % m_numFrames));
}

void PlatformerObject::doDyingAnimation()
{
    m_currentFrame = int(((SDL_GetTicks() / (1000/ 10)) % m_numFrames));
    
    if(m_dyingCounter == m_dyingTime)
    {
        m_bDead = true;
    }
    m_dyingCounter++;
}

//2 : 깰수 없는 타일, 1 : 깰 수 있는 타일, 0 : 충돌없음
int PlatformerObject::CheckCollideTileType(Vector2D& newPos)
{
	TileLayer* pTileLayer = NULL;
	std::vector<std::vector<int>> tiles;
	for (std::vector<TileLayer*>::iterator it = m_pCollisionLayers->begin(); it != m_pCollisionLayers->end(); ++it)
	{
		pTileLayer  = (*it);

		tiles = pTileLayer->getTileIDs();


		int tileColumn, tileRow, tileid = 0;

		tileColumn = newPos.m_x / pTileLayer->getTileSize();
		tileRow = newPos.m_y / pTileLayer->getTileSize();

		if (tileRow >= pTileLayer->getRows() || tileRow < 0)
			return 2;

		if (tileColumn >= pTileLayer->getCols() || tileColumn < 0)
			return 2;

		tileid = tiles[tileRow][tileColumn];

		if (tileid != 0)
		{
			
			if (pTileLayer->IsBreakTile())
				return 1;

			return 2;
		}		
	}

	return 0;
}

bool PlatformerObject::BreakTile(Vector2D newPos)
{

	for (std::vector<TileLayer*>::iterator it = m_pCollisionLayers->begin(); it != m_pCollisionLayers->end(); ++it)
	{
		TileLayer* pTileLayer = (*it);

		std::vector<std::vector<int>> tiles = pTileLayer->getTileIDs();


		int tileColumn, tileRow, tileid = 0;

		tileColumn = newPos.m_x / pTileLayer->getTileSize();
		tileRow = newPos.m_y / pTileLayer->getTileSize();

		if (tileRow >= pTileLayer->getRows())
			return false;

		if (tileColumn >= pTileLayer->getCols())
			return false;

		tileid = tiles[tileRow][tileColumn];

		if (tileid != 0)
		{
			if (pTileLayer->ProcessTileBreak(tileRow, tileColumn))
			{
				GetOwner()->GenerateItem(tileRow, tileColumn);

				GetOwner()->AddNavigationNode(tileRow, tileColumn);
				return true;
			}

			return false;
		}
	}

	return false;
}

bool PlatformerObject::checkCollideTile(Vector2D newPos)
{
	//if (newPos.m_y + m_height >= TheGame::Instance()->getGameHeight() - 32)
	if (newPos.m_y  < 32 || newPos.m_x < 32)
	{
		return true;
	}
	else
	{
		for (std::vector<TileLayer*>::iterator it = m_pCollisionLayers->begin(); it != m_pCollisionLayers->end(); ++it)
		{
			TileLayer* pTileLayer = (*it);

			std::vector<std::vector<int>> tiles = pTileLayer->getTileIDs();

			Vector2D layerPos = pTileLayer->getPosition();

			int x, y, tileColumn, tileRow, tileid = 0;

			x = layerPos.getX() / pTileLayer->getTileSize();
			y = layerPos.getY() / pTileLayer->getTileSize();

			Vector2D startPos = newPos;

			Vector2D endPos(newPos.m_x + m_width, (newPos.m_y) + m_height);

			for (int i = startPos.m_x; i < endPos.m_x; i++)
			{
				for (int j = startPos.m_y; j < endPos.m_y; j++)
				{
					tileColumn = i / pTileLayer->getTileSize();
					tileRow = j / pTileLayer->getTileSize();

					if (tileRow >= pTileLayer->getRows())
						return true;

					tileid = tiles[tileRow + y][tileColumn + x];

					if (tileid != 0)
					{
						return true;
					}
				}
			}
		}

		return false;

	}
}

bool PlatformerObject::CheckCollision(GameObject* gameObject)
{
	SDL_Rect rect1;
	rect1.x = gameObject->getPosition().getX();
	rect1.y = gameObject->getPosition().getY();
	rect1.w = gameObject->getWidth();
	rect1.h = gameObject->getHeight();

	SDL_Rect rect2;

	rect2.x = getPosition().getX();
	rect2.y = getPosition().getY();
	rect2.w = getWidth();
	rect2.h = getHeight();

	if (RectRect(&rect1, &rect2))
	{
		return true;
	}

	return false;
}
