#include "TileLayer.h"

class BreakableLayer : public TileLayer
{
public:
	BreakableLayer(int tileSize, int mapWidth, int mapHeight, const std::vector<Tileset>& tilesets);
	virtual ~BreakableLayer();

	virtual void Update(Level* pLevel);
	virtual bool ProcessTileBreak(int tileRow, int tileColumn);
	virtual bool IsBreakTile(){ return true; }
};

