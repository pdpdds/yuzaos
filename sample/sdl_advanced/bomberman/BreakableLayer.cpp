//#include "stdafx.h"
#include "BreakableLayer.h"

BreakableLayer::BreakableLayer(int tileSize, int mapWidth, int mapHeight, const std::vector<Tileset>& tilesets)
: TileLayer(tileSize, mapWidth, mapHeight, tilesets)
{
}


BreakableLayer::~BreakableLayer()
{
}

void BreakableLayer::Update(Level* pLevel)
{
	/*Player* pPlayer = pLevel->getPlayer();

	Vector2D layerPos = getPosition();

	int x, y, tileColumn, tileRow, tileid = 0;

	x = layerPos.getX() / getTileSize();
	y = layerPos.getY() / getTileSize();

	Vector2D startPos = pPlayer->getPosition();

	Vector2D endPos(pPlayer->getPosition().m_x + (pPlayer->getWidth() + 2), (pPlayer->getPosition().m_y) + pPlayer->getHeight() + 2);

	for (int i = startPos.m_x; i < endPos.m_x; i++)
	{
		for (int j = startPos.m_y; j < endPos.m_y; j++)
		{
			tileColumn = i / getTileSize();
			tileRow = j / getTileSize();

			tileid = m_tileIDs[tileRow + y][tileColumn + x];

			if (tileid != 0)
			{
				m_tileIDs[tileRow + y][tileColumn + x] = 0;
				return;
			}
		}
	}*/

}



bool BreakableLayer::ProcessTileBreak(int tileRow, int tileColumn)
{
	m_tileIDs[tileRow][tileColumn] = 0;

	return true;
}
