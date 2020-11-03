//
//  TileLayer.h
//  SDL Game Programming Book
//
//  Created by shaun mitchell on 10/03/2013.
//  Copyright (c) 2013 shaun mitchell. All rights reserved.
//

#ifndef __SDL_Game_Programming_Book__TileLayer__
#define __SDL_Game_Programming_Book__TileLayer__

#include <iostream>
#include <vector>
#include "Layer.h"
#include "Level.h"
#include "Vector2D.h"

class TileLayer : public Layer
{
public:
    
    TileLayer(int tileSize, int mapWidth, int mapHeight, const std::vector<Tileset>& tilesets);
    
    virtual ~TileLayer() {}
    
	virtual void Update(Level* pLevel);
    virtual void Render();
	virtual bool ProcessTileBreak(int tileRow, int tileColumn){ return false; }
	virtual bool IsBreakTile(){ return false; }
    
    void setTileIDs(const std::vector<std::vector<int> >& data) { m_tileIDs = data; }
    void setTileSize(int tileSize) { m_tileSize = tileSize; }
    void setMapWidth(int mapWidth) { m_mapWidth = mapWidth; }
    int getMapWidth() { return m_mapWidth; }
	void setMapHeight(int mapHeight) { m_mapHeight = mapHeight; }
	int getMapHeight() { return m_mapHeight; }
    
    int getTileSize() { return m_tileSize; }
    
    const std::vector<std::vector<int> >& getTileIDs() { return m_tileIDs; }
    
    Tileset getTilesetByID(int tileID);
    
    const Vector2D getPosition() { return m_position; }
    
    void setPosition(Vector2D position) { m_position = position; }

	int getRows(){ return m_numRows; }
	int getCols(){ return m_numColumns; }
    
    
private:
    
    int m_numColumns;
    int m_numRows;
    int m_tileSize;
    
    int m_mapWidth;
	int m_mapHeight;
    
    Vector2D m_position;
    Vector2D m_velocity;
    Vector2D m_acceleration;
    
    float diff;
    
    const std::vector<Tileset>& m_tilesets;
    
protected:
    std::vector<std::vector<int> > m_tileIDs;
};

#endif /* defined(__SDL_Game_Programming_Book__TileLayer__) */
