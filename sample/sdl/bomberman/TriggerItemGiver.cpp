//#include "stdafx.h"
#include "TriggerItemGiver.h"
#include "TextureManager.h"
#include "Game.h"
#include "Camera.h"
#include "Player.h"
#include "Explosion.h"

TriggerItemGiver::TriggerItemGiver(int itemType)
: TriggerLimitedLifeTime(2800)
, m_itemType(itemType)
{
	m_invincible = true;
}


TriggerItemGiver::~TriggerItemGiver()
{
}


void  TriggerItemGiver::Try(GameObject* pGameObject)
{	
	if (pGameObject->type().compare("Player") == 0)
	{
		Vector2D temp;
		temp.m_x = 16;
		temp.m_y = 16;
		if (isActive() && isTouchingTrigger(pGameObject->getPosition() + temp, (pGameObject->getWidth() / 2)-1))
		{
			Player* pPlayer = (Player*)pGameObject;

			if (m_itemType == TRIGGER_BOMB)
				pPlayer->IncreaseBombCount();
			else if (m_itemType == TRIGGER_BOMBPOWER)
				pPlayer->IncreaseBombPower();
			else if (m_itemType == TRIGGER_SPEEDUP)
				pPlayer->SetSpeed(pPlayer->GetSpeed()+0.2f);
			else if (m_itemType == TRIGGER_TNT)
				pPlayer->SetBombType(BOMB_TNT);			

			SetToBeRemovedFromGame();
		}
	}
	else if (pGameObject->type().compare("Explosion") == 0)
	{
		Explosion* pExplosion = (Explosion*)pGameObject;
		for (std::vector<FlameInfo>::iterator it = pExplosion->m_flameList.begin(); it != pExplosion->m_flameList.end(); ++it)// < m_gameObjects.size(); i++)
		{
			Vector2D temp;
			temp.m_x = (*it).offsetX + 16;
			temp.m_y = (*it).offsetY + 16;


			if (isActive() && isTouchingTrigger(pGameObject->getPosition() + temp, (pGameObject->getWidth() / 2) - 1))
			{
				if (m_invincible)
					m_invincible = false;
				else
					SetToBeRemovedFromGame();
			}


		}

		
	}
}

void TriggerItemGiver::Draw()
{
	TextureManager::Instance()->drawFrame(m_textureID, (Uint32)m_position.getX() - TheCamera::Instance()->getPosition().m_x, (Uint32)m_position.getY() - TheCamera::Instance()->getPosition().m_y,
		m_width, m_height, m_currentRow, m_currentFrame, TheGame::Instance()->getRenderer(), m_angle, m_alpha);
}

void TriggerItemGiver::Load(LoaderParams& params)
{
	// get position
	m_position = Vector2D(params.getX(), params.getY());

	// get drawing variables
	m_width = params.getWidth();
	m_height = params.getHeight();
	m_textureID = params.getTextureID();
	m_numFrames = params.getNumFrames();
}