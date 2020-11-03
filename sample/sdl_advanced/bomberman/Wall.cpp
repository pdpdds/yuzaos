//#include "stdafx.h"
#include "Wall.h"
#include "Game.h"
#include "InputHandler.h"
#include "TileLayer.h"
#include "SoundManager.h"
#include "Camera.h"
#include "TextureManager.h"
#include "Explosion.h"

Wall::Wall()
{
	
}


Wall::~Wall()
{
}


void Wall::Draw()
{
	TheTextureManager::Instance()->drawFrame(m_textureID, (Uint32)m_position.getX() - TheCamera::Instance()->getPosition().m_x, (Uint32)m_position.getY() - TheCamera::Instance()->getPosition().m_y,
		m_width, m_height, m_currentRow, m_currentFrame, TheGame::Instance()->getRenderer(), m_angle, m_alpha);
}

void Wall::Update()
{
	
}

void Wall::Clean()
{
	PlatformerObject::Clean();
}

void Wall::Collision()
{

}

void Wall::Load(LoaderParams& params)
{
	// inherited load function
	PlatformerObject::Load(params);
}