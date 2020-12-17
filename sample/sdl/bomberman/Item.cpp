//#include "stdafx.h"
#include "Item.h"
#include "Game.h"
#include "InputHandler.h"
#include "TileLayer.h"
#include "SoundManager.h"
#include "Camera.h"
#include "TextureManager.h"
#include "Explosion.h"
#include "MessageDispatcher.h"
#include "Messages.h"
#include "Collision.h"

Item::Item(int itemType)
	: m_itemType(itemType)
{
	m_generatedTime = SDL_GetTicks();
}


Item::~Item()
{
}

void Item::Draw()
{
	TheTextureManager::Instance()->drawFrame(m_textureID, (Uint32)m_position.getX() - TheCamera::Instance()->getPosition().m_x, (Uint32)m_position.getY() - TheCamera::Instance()->getPosition().m_y,
		m_width, m_height, m_currentRow, m_currentFrame, TheGame::Instance()->getRenderer(), m_angle, m_alpha);
}

void Item::Update()
{
	m_currentFrame = int(((SDL_GetTicks() / (200)) % m_numFrames));

	m_lifeTime = SDL_GetTicks() - m_generatedTime;

	if (m_bDead == false)
	{
		if (m_lifeTime > 5000000)
		{
			m_bDead = true;			
		}
	}
}

void Item::Clean()
{
	PlatformerObject::Clean();
}

void Item::Collision()
{

}

void Item::Load(LoaderParams& params)
{
	// inherited load function
	PlatformerObject::Load(params);
}