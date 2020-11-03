//#include "stdafx.h"
#include "Bomb.h"
#include "Game.h"
#include "InputHandler.h"
#include "TileLayer.h"
#include "SoundManager.h"
#include "Camera.h"
#include "TextureManager.h"
#include "Explosion.h"
#include "MessageDispatcher.h"
#include "Messages.h"

Bomb::Bomb(int ownerId, int explosionInterval)
: m_ownerId(ownerId)
, m_explosionInterval(explosionInterval)
, m_lifeTime(0)
, m_bombPower(1)
, m_bChainedExplosion(false)
, m_bombType(BOMB_NORMAL)
{
	m_generatedTime = SDL_GetTicks();
}


Bomb::~Bomb()
{
}


void Bomb::Draw()
{	
	TheTextureManager::Instance()->drawFrame(m_textureID, (Uint32)m_position.getX() - TheCamera::Instance()->getPosition().m_x, (Uint32)m_position.getY() - TheCamera::Instance()->getPosition().m_y,
			m_width, m_height, m_currentRow, m_currentFrame, TheGame::Instance()->getRenderer(), m_angle, m_alpha);	
}

void Bomb::Update()
{
	m_currentFrame = int(((SDL_GetTicks() / (200)) % m_numFrames));

	m_lifeTime = SDL_GetTicks() - m_generatedTime;

	if (m_bDead == false)
	{
		if (m_lifeTime > m_explosionInterval || m_bChainedExplosion)
		{
			m_bDead = true;
			Level* pLevel = GetOwner();

			pLevel->CreateExplosion(this, m_bombPower, m_bombType);		

			Dispatcher->DispatchMsg(SEND_MSG_IMMEDIATELY,
				ID(),
				m_ownerId,
				Msg_BombExplosion,
				NO_ADDITIONAL_INFO);
		}
	}
}

void Bomb::Clean()
{
	PlatformerObject::Clean();
}

void Bomb::Collision()
{

}

void Bomb::Load(LoaderParams& params)
{
	// inherited load function
	PlatformerObject::Load(params);	
}

void Bomb::ProcessDeadAction()
{
	m_bChainedExplosion = true;
}