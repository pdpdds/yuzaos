//
//  LevelParser.cpp
//  SDL Game Programming Book
//
//  Created by shaun mitchell on 10/03/2013.
//  Copyright (c) 2013 shaun mitchell. All rights reserved.
//

#include <string>
#include "LevelParser.h"
#include "TextureManager.h"
#include "Game.h"
#include "ObjectLayer.h"
#include "TileLayer.h"
#include "GameObjectFactory.h"
#include "base64.h"
#include "zlib.h"
#include "Level.h"
#include "BreakableLayer.h"
#include "Wall.h"
#include "MessageDispatcher.h"

bool LevelParser::read_text(const char* source_file, char** destination)
{
	// Open the file
	SDL_RWops *file;
	file = SDL_RWFromFile(source_file, "r");

	if (file == 0)
		return false;

	size_t file_length = SDL_RWseek(file, 0, SEEK_END);
	(*destination) = new char[file_length + 1];
	// Reset seek to beginning of file and read text
	SDL_RWseek(file, 0, SEEK_SET);
	int n_blocks = SDL_RWread(file, (*destination), 1, file_length);
	if (n_blocks <= 0)
	{

	}
	SDL_RWclose(file);

	(*destination)[file_length] = '\0';

	return true;
}

Level* LevelParser::parseLevel(const char *levelFile)
{
    
#ifndef WIN32
	char* file_contents = NULL;

	if (read_text(levelFile, &file_contents) != true)
	{
		return 0;
	}
#endif

	// create a tinyXML document and load the map xml
	TiXmlDocument levelDocument;

	// load the state file
#ifndef WIN32
	levelDocument.Parse(file_contents);
#else
	levelDocument.LoadFile(levelFile);
#endif

#ifndef WIN32
	delete[] file_contents;
#endif  
        
    // get the root node and display some values
    TiXmlElement* pRoot = levelDocument.RootElement();
    
    std::cout << "Loading level:\n" << "Version: " << pRoot->Attribute("version") << "\n";
    std::cout << "Width:" << pRoot->Attribute("width") << " - Height:" << pRoot->Attribute("height") << "\n";
    std::cout << "Tile Width:" << pRoot->Attribute("tilewidth") << " - Tile Height:" << pRoot->Attribute("tileheight") << "\n";
    
    pRoot->Attribute("tilewidth", &m_tileSize);
    pRoot->Attribute("width", &m_width);
    pRoot->Attribute("height", &m_height);

	// create the level object
	Level* pLevel = new Level(m_width*m_tileSize, m_height*m_tileSize);
	Dispatcher->SetLevel(pLevel);
	
    //we know that properties is the first child of the root
    TiXmlElement* pProperties = pRoot->FirstChildElement();
    
    // we must parse the textures needed for this level, which have been added to properties
    for(TiXmlElement* e = pProperties->FirstChildElement(); e != NULL; e = e->NextSiblingElement())
    {
        if(e->Value() == std::string("property"))
        {
            parseTextures(e);
        }
    }
    
    // we must now parse the tilesets
    for(TiXmlElement* e = pRoot->FirstChildElement(); e != NULL; e = e->NextSiblingElement())
    {
        if(e->Value() == std::string("tileset"))
        {
            parseTilesets(e, pLevel->getTilesets());
        }
    }
    
    // parse any object layers
    for(TiXmlElement* e = pRoot->FirstChildElement(); e != NULL; e = e->NextSiblingElement())
    {
        if(e->Value() == std::string("objectgroup") || e->Value() == std::string("layer"))
        {
			std::string aa = e->FirstChildElement()->Value();
            if(e->FirstChildElement()->Value() == std::string("object"))
            {
                parseObjectLayer(e, pLevel->getLayers(), pLevel);
            }
            else if(e->FirstChildElement()->Value() == std::string("data") ||
                    (e->FirstChildElement()->NextSiblingElement() != 0 && e->FirstChildElement()->NextSiblingElement()->Value() == std::string("data")))
            {
                parseTileLayer(e, pLevel->getLayers(), pLevel->getTilesets(), pLevel->getCollisionLayers());
            }
			else if 
				(e->FirstChildElement()->NextSiblingElement() != 0 && e->FirstChildElement()->NextSiblingElement()->Value() == std::string("object"))
			{
				parseObjectLayer(e, pLevel->getLayers(), pLevel);
			}
        }
    }
    
	pLevel->GenerateNavigationGraph();
	
    return pLevel;
}


void LevelParser::parseTextures(TiXmlElement* pTextureRoot)
{
    std::cout << "adding texture " << pTextureRoot->Attribute("value") << " with ID " << pTextureRoot->Attribute("name") << std::endl;
    TheTextureManager::Instance()->load(pTextureRoot->Attribute("value"), pTextureRoot->Attribute("name"), TheGame::Instance()->getRenderer());
}

void LevelParser::parseTilesets(TiXmlElement* pTilesetRoot, std::vector<Tileset>* pTilesets)
{
	std::string assetsTag = "assets/";
    // first add the tileset to texture manager
    std::cout << "adding texture " << pTilesetRoot->FirstChildElement()->Attribute("source") << " with ID " << pTilesetRoot->Attribute("name") << std::endl;
    TheTextureManager::Instance()->load(assetsTag.append(pTilesetRoot->FirstChildElement()->Attribute("source")), pTilesetRoot->Attribute("name"), TheGame::Instance()->getRenderer());
    
    // create a tileset object
    Tileset tileset;
	//memset(&tileset, 0, sizeof(Tileset));
    pTilesetRoot->FirstChildElement()->Attribute("width", &tileset.width);
    pTilesetRoot->FirstChildElement()->Attribute("height", &tileset.height);
    pTilesetRoot->Attribute("firstgid", &tileset.firstGridID);
    pTilesetRoot->Attribute("tilewidth", &tileset.tileWidth);
    pTilesetRoot->Attribute("tileheight", &tileset.tileHeight);
    pTilesetRoot->Attribute("spacing", &tileset.spacing);
    pTilesetRoot->Attribute("margin", &tileset.margin);

    tileset.name = pTilesetRoot->Attribute("name");
    
    tileset.numColumns = tileset.width / (tileset.tileWidth + tileset.spacing);
    
    pTilesets->push_back(tileset);
}

void LevelParser::parseObjectLayer(TiXmlElement* pObjectElement, std::vector<Layer*> *pLayers, Level* pLevel)
{
    // create an object layer
    //ObjectLayer* pObjectLayer = new ObjectLayer();
    
    for(TiXmlElement* e = pObjectElement->FirstChildElement(); e != NULL; e = e->NextSiblingElement())
    {
        if(e->Value() == std::string("object"))
        {
            int x, y, width, height, numFrames, callbackID = 0, animSpeed = 0;
            std::string textureID;
			std::string triggerType;
            std::string type;
            
            // get the initial node values type, x and y
            e->Attribute("x", &x);
            e->Attribute("y", &y);
            
            type = e->Attribute("type");
            
			if (type == "Player")
			{

				// get the property values
				for (TiXmlElement* properties = e->FirstChildElement(); properties != NULL; properties = properties->NextSiblingElement())
				{
					if (properties->Value() == std::string("properties"))
					{
						for (TiXmlElement* property = properties->FirstChildElement(); property != NULL; property = property->NextSiblingElement())
						{
							if (property->Value() == std::string("property"))
							{
								if (property->Attribute("name") == std::string("numFrames"))
								{
									property->Attribute("value", &numFrames);
								}
								else if (property->Attribute("name") == std::string("textureHeight"))
								{
									property->Attribute("value", &height);
								}
								else if (property->Attribute("name") == std::string("textureID"))
								{
									textureID = property->Attribute("value");
								}
								else if (property->Attribute("name") == std::string("textureWidth"))
								{
									property->Attribute("value", &width);
								}
								else if (property->Attribute("name") == std::string("callbackID"))
								{
									property->Attribute("value", &callbackID);
								}
								else if (e->Attribute("name") == std::string("animSpeed"))
								{
									property->Attribute("value", &animSpeed);
								}
							}
						}
					}
				}

				Player* pPlayer = new Player(pLevel);
				

				// load the object
				LoaderParams param = LoaderParams(x, y, width, height, textureID, numFrames, callbackID, animSpeed);
				pPlayer->Load(param);

				pPlayer->setCollisionLayers(pLevel->getCollisionLayers());
				pLevel->AddPlayers((static_cast<Player*>(pPlayer)));
			}
			else if (type == "Trigger")
			{
				// get the property values
				for (TiXmlElement* properties = e->FirstChildElement(); properties != NULL; properties = properties->NextSiblingElement())
				{
					if (properties->Value() == std::string("properties"))
					{
						for (TiXmlElement* property = properties->FirstChildElement(); property != NULL; property = property->NextSiblingElement())
						{
							if (property->Value() == std::string("property"))
							{
								if (property->Attribute("name") == std::string("triggerType"))
								{
									triggerType = property->Attribute("value");
								}								
							}
						}
					}
				}
			
				pLevel->AddTrigger(x,y, triggerType);
			}
        }
    }    
}

void LevelParser::parseTileLayer(TiXmlElement* pTileElement, std::vector<Layer*> *pLayers, const std::vector<Tileset>* pTilesets, std::vector<TileLayer*> *pCollisionLayers)
{
	TileLayer* pTileLayer = 0;
	
    bool collidable = false;
	bool breakable = false;
    
    // tile data
    std::vector<std::vector<int>> data;
    
    std::string decodedIDs;
    TiXmlElement* pDataNode = 0;
    
    for(TiXmlElement* e = pTileElement->FirstChildElement(); e != NULL; e = e->NextSiblingElement())
    {
        if(e->Value() == std::string("properties"))
        {
            for(TiXmlElement* property = e->FirstChildElement(); property != NULL; property = property->NextSiblingElement())
            {
                if(property->Value() == std::string("property"))
                {
                    if(property->Attribute("name") == std::string("collidable"))
                    {
                        collidable = true;
                    }
					if (property->Attribute("name") == std::string("breakable"))
					{
						breakable = true;
					}
                }
            }
        }
        
        if(e->Value() == std::string("data"))
        {
            pDataNode = e;
        }
    }
    
    for(TiXmlNode* e = pDataNode->FirstChild(); e != NULL; e = e->NextSibling())
    {
        TiXmlText* text = e->ToText();
        std::string t = text->Value();
        decodedIDs = base64_decode(t);
    }
    
    // uncompress zlib compression
    uLongf sizeofids = m_width * m_height * sizeof(int);
    std::vector<int> ids(m_width * m_height);
    uncompress((Bytef*)&ids[0], &sizeofids,(const Bytef*)decodedIDs.c_str(), decodedIDs.size());
    

	if (breakable == true)
	{
		pTileLayer = new BreakableLayer(m_tileSize, m_width, m_height, *pTilesets);
	}
	else
	{
		pTileLayer = new TileLayer(m_tileSize, m_width, m_height, *pTilesets);
	}
    
	std::vector<int> layerRow(m_width);
    
    for(int j = 0; j < m_height; j++)
    {
        data.push_back(layerRow);
    }
    
    for(int rows = 0; rows < m_height; rows++)
    {
        for(int cols = 0; cols < m_width; cols++)
        {
            data[rows][cols] = ids[rows * m_width + cols];
        }
    }
    
    pTileLayer->setTileIDs(data);
	
	/*if (breakable == true)
	{
		for (int rows = 0; rows < m_height; rows++)
		{
			for (int cols = 0; cols < m_width; cols++)
			{
				if (data[rows][cols])
				{
					Wall* pWall = static_cast<Wall*>(TheGameObjectFactory::Instance()->create("Wall"));
					LoaderParams param = LoaderParams(cols * 32, rows * 32, 32, 32, "data/image/object/tiles.bmp", 8, 0, 0);
					pWall->load(param);										
				}

				
			}
		}	

		delete pTileLayer;
	}
	else*/
	{ 
		if (collidable)
		{
			pCollisionLayers->push_back(pTileLayer);
		}

		pLayers->push_back(pTileLayer);
	}    
}

