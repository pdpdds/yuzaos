//#include "stdafx.h"
#include "Explosion.h"
#include "Game.h"
#include "InputHandler.h"
#include "TileLayer.h"
#include "SoundManager.h"
#include "Camera.h"
#include "TextureManager.h"
#include "Collision.h"

Explosion::Explosion()
	: m_lifeTime(0)
{
	m_explosionLenth = 1;
	m_generatedTime = SDL_GetTicks();
}


void Explosion::MakeTNTFlame()
{
	FlameInfo info;
	info.frameindex = 2;
	Vector2D pos;
	pos.m_x = m_position.getX() - 32 * m_explosionLenth;
	pos.m_y = m_position.getY() - 32 * m_explosionLenth;

	//가로+
	for (int i = 0; i <= m_explosionLenth*2; i++)
		for (int j = 0; j <= m_explosionLenth*2; j++)
	{
		bool bBlocked = false;
		Vector2D tempPos = pos;
		tempPos.m_x += 32 * i;
		tempPos.m_y += 32 * j;

		int result = CheckCollideTileType(tempPos); // 이 메소드 내부에서 부술 수 있는 벽돌이면 부숴 버린다.
		if (result == 2) // 부술 수 없는 벽돌
			continue;

		if (result == 1) // 부술 수 있는 벽돌
		{			
			BreakTile(tempPos);
		}

		info.offsetX = tempPos.m_x - m_position.m_x;
		info.offsetY = tempPos.m_y - m_position.m_y;

		SDL_Rect rect;
		rect.x = tempPos.m_x;
		rect.y = tempPos.m_y;
		rect.h = 32;
		rect.w = 32;

		m_flameList.push_back(info);		
	}

	GetOwner()->CheckExplosionEffect(this);
}

void Explosion::MakeFlame(int bombPower, int bombType)
{
	m_explosionLenth = bombPower;

	if (bombType == BOMB_TNT)
	{
		MakeTNTFlame();
		return;
	}

	FlameInfo info;
	info.frameindex = 2;
	info.offsetX = 0;
	info.offsetY = 0;
	m_flameList.push_back(info);
	Vector2D pos;
	pos.m_x = m_position.getX();
	pos.m_y = m_position.getY();

	//가로+
	for (int i = 1; i <= m_explosionLenth; i++)
	{
		bool bBlocked = false;
		Vector2D tempPos = pos;
		tempPos.m_x += 32 * i;
		tempPos.m_y += 0;

		int result = CheckCollideTileType(tempPos); // 이 메소드 내부에서 부술 수 있는 벽돌이면 부숴 버린다.
		if (result == 2) // 부술 수 없는 벽돌
			break;

		if (result == 1) // 부술 수 있는 벽돌
		{
			bBlocked = true;
			BreakTile(tempPos);
		}

		if (m_explosionLenth == i || bBlocked == true)
			info.frameindex = 4;
		else
			info.frameindex = 3;
		info.offsetX = 32 * i;
		info.offsetY = 0;

		SDL_Rect rect;
		rect.x = tempPos.m_x;
		rect.y = tempPos.m_y;
		rect.h = 32;
		rect.w = 32;		

		m_flameList.push_back(info);

		if (bBlocked)
			break;		
	}

	//가로-
	for (int i = 1; i <= m_explosionLenth; i++)
	{
		bool bBlocked = false;
		Vector2D tempPos = pos;
		tempPos.m_x += -32 * i;
		tempPos.m_y += 0;

		int result = CheckCollideTileType(tempPos); // 이 메소드 내부에서 부술 수 있는 벽돌이면 부숴 버린다.
		if (result == 2) // 부술 수 없는 벽돌
			break;

		if (result == 1) // 부술 수 있는 벽돌
		{
			bBlocked = true;
			BreakTile(tempPos);
		}
		if (m_explosionLenth == i || bBlocked == true)
			info.frameindex = 0;
		else
			info.frameindex = 1;

		info.offsetX = -32 * i;
		info.offsetY = 0;

		m_flameList.push_back(info);

		SDL_Rect rect;
		rect.x = tempPos.m_x;
		rect.y = tempPos.m_y;
		rect.h = 32;
		rect.w = 32;
		

		if (bBlocked)
			break;
	}

	//세로+
	for (int i = 1; i <= m_explosionLenth; i++)
	{
		bool bBlocked = false;
		Vector2D tempPos = pos;
		tempPos.m_x += 0;
		tempPos.m_y += 32 * i;

		int result = CheckCollideTileType(tempPos); // 이 메소드 내부에서 부술 수 있는 벽돌이면 부숴 버린다.
		if (result == 2) // 부술 수 없는 벽돌
			break;

		if (result == 1) // 부술 수 있는 벽돌
		{
			bBlocked = true;
			BreakTile(tempPos);
		}
		if (m_explosionLenth == i || bBlocked == true)
			info.frameindex = 7;
		else
			info.frameindex = 6;
		info.offsetX = 0;
		info.offsetY = 32 * i;

		m_flameList.push_back(info);

		SDL_Rect rect;
		rect.x = tempPos.m_x;
		rect.y = tempPos.m_y;
		rect.h = 32;
		rect.w = 32;
		

		if (bBlocked)
			break;
	}

	//세로-
	for (int i = 1; i <= m_explosionLenth; i++)
	{
		bool bBlocked = false;
		Vector2D tempPos = pos;
		tempPos.m_x += 0;
		tempPos.m_y += -32 * i;

		int result = CheckCollideTileType(tempPos); // 이 메소드 내부에서 부술 수 있는 벽돌이면 부숴 버린다.
		if (result == 2) // 부술 수 없는 벽돌
			break;

		if (result == 1) // 부술 수 있는 벽돌
		{
			bBlocked = true;
			BreakTile(tempPos);
		}
		if (m_explosionLenth == i || bBlocked == true)
			info.frameindex = 5;
		else
			info.frameindex = 6;

		info.offsetX = 0;
		info.offsetY = -32 * i;

		SDL_Rect rect;
		rect.x = tempPos.m_x;
		rect.y = tempPos.m_y;
		rect.h = 32;
		rect.w = 32;

		m_flameList.push_back(info);

		if (bBlocked)
			break;		
	}

	GetOwner()->CheckExplosionEffect(this);
}

Explosion::~Explosion()
{

	m_flameList.clear();
}

void Explosion::Draw()
{
	for (std::vector<FlameInfo>::iterator it = m_flameList.begin(); it != m_flameList.end(); ++it)// < m_gameObjects.size(); i++)
	{
		FlameInfo& info = *it;
		DrawFlame(info.frameindex, info.offsetX, info.offsetY);
	}
	/*DrawCenter();

	for (int i = 1; i < m_explosionLenth; i++)
	{
	DrawFlame(3, 32 * i, 0);//가로+
	DrawFlame(1, -32 * i, 0);//가로-
	DrawFlame(6, 0 * i, -32 * i);//세로-
	DrawFlame(6, 0 * i, 32 * i);//세로+
	}

	DrawFlame(4, 32 * m_explosionLenth, 0);
	DrawFlame(0, -32 * m_explosionLenth, 0);
	DrawFlame(5, 0, -32 * m_explosionLenth);
	DrawFlame(7, 0, 32 * m_explosionLenth);*/
}

void Explosion::DrawCenter()
{
	TheTextureManager::Instance()->drawFrame(m_textureID, (Uint32)m_position.getX() - TheCamera::Instance()->getPosition().m_x, (Uint32)m_position.getY() - TheCamera::Instance()->getPosition().m_y,
		m_width, m_height, m_currentRow, 2, TheGame::Instance()->getRenderer(), m_angle, m_alpha);
}

void Explosion::DrawFlame(int frameIndex, int xOffset, int yOffset)
{
	Vector2D pos;
	pos.m_x = m_position.getX() - TheCamera::Instance()->getPosition().m_x + xOffset;
	pos.m_y = m_position.getY() + yOffset - TheCamera::Instance()->getPosition().m_y;

	TheTextureManager::Instance()->drawFrame(m_textureID, pos.getX(), pos.getY(), m_width, m_height, m_currentRow, frameIndex, TheGame::Instance()->getRenderer(), m_angle, m_alpha);
}

void Explosion::Update()
{
	m_lifeTime = SDL_GetTicks() - m_generatedTime;

	m_currentRow = m_lifeTime / 200;

	if (m_currentRow >= MAX_EXPLOSION_ROW)
	{
		m_currentRow = MAX_EXPLOSION_ROW - 1;
		m_bDead = true;
	}
	else
	{

	}
}

void Explosion::Clean()
{
	PlatformerObject::Clean();
}

void Explosion::Collision()
{

}

void Explosion::Load(LoaderParams& params)
{
	// inherited load function
	PlatformerObject::Load(params);
}

bool Explosion::InRange(GameObject* pObject)
{
	SDL_Rect rect1;
	rect1.x = pObject->getPosition().getX();
	rect1.y = pObject->getPosition().getY();
	rect1.w = pObject->getWidth();
	rect1.h = pObject->getHeight();

	for (std::vector<FlameInfo>::iterator it = m_flameList.begin(); it != m_flameList.end(); ++it)// < m_gameObjects.size(); i++)
	{
		FlameInfo& info = *it;

		SDL_Rect rect2;
		rect2.x = getPosition().getX() + info.offsetX;
		rect2.y = getPosition().getY() + info.offsetY;
		rect2.w = 32;
		rect2.h = 32;

		if (RectRect(&rect1, &rect2))
		{
			return true;
		}
	}

	return false;
}

bool Explosion::CheckCollision(GameObject* gameObject)
{
	int explsionLenth = GetExplosionLength();

	if (true == InRange(gameObject))
	{
		if (!dead() && !dying())
		{
			return true;
		}
	}

	return false;
}