//
//  StateParser.cpp
//  SDL Game Programming Book
//
//  Created by shaun mitchell on 24/02/2013.
//  Copyright (c) 2013 shaun mitchell. All rights reserved.
//

#include "StateParser.h"
#include "TextureManager.h"
#include "Game.h"
#include "GameObjectFactory.h"

using namespace std;

bool StateParser::parseState(const char *stateFile, string stateID, vector<GameObject *> *pObjects, std::vector<std::string> *pTextureIDs)
{
#ifndef WIN32
	char* file_contents = NULL;

	if (read_text(stateFile, &file_contents) != true)
	{
		return false;
	}
#endif

    // create the XML document
    TiXmlDocument xmlDoc;
    
    // load the state file
#ifndef WIN32
	if(!xmlDoc.Parse(file_contents))
#else
	if (!xmlDoc.LoadFile(stateFile))
#endif
    {
        //cerr << xmlDoc.ErrorDesc() << "\n";
        printf("%s\n", xmlDoc.ErrorDesc());
#ifndef WIN32
		delete[] file_contents;
#endif
        return false;
    }

#ifndef WIN32
	delete[] file_contents;
#endif    
    // get the root element
    TiXmlElement* pRoot = xmlDoc.RootElement();
    
    // pre declare the states root node
    TiXmlElement* pStateRoot = 0;
    // get this states root node and assing it to pStateRoot
    for(TiXmlElement* e = pRoot->FirstChildElement(); e != NULL; e = e->NextSiblingElement())
    {
        if(e->Value() == stateID)
        {
            pStateRoot = e;
			break;
        }
    }
    
    // pre declare the texture root
    TiXmlElement* pTextureRoot = 0;
    
    // get the root of the texture elements
    for(TiXmlElement* e = pStateRoot->FirstChildElement(); e != NULL; e = e->NextSiblingElement())
    {
        if(e->Value() == string("TEXTURES"))
        {
            pTextureRoot = e;
			break;
        }
    }
    
    // now parse the textures
    parseTextures(pTextureRoot, pTextureIDs);
    
    // pre declare the object root node
    TiXmlElement* pObjectRoot = 0;
    
    // get the root node and assign it to pObjectRoot
    for(TiXmlElement* e = pStateRoot->FirstChildElement(); e != NULL; e = e->NextSiblingElement())
    {
        if(e->Value() == string("OBJECTS"))
        {
            pObjectRoot = e;
			break;
        }
    }

    // now parse the objects
    parseObjects(pObjectRoot, pObjects);
    
    return true;
}

bool StateParser::read_text(const char* source_file, char** destination)
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

void StateParser::parseTextures(TiXmlElement* pStateRoot, std::vector<std::string> *pTextureIDs)
{
    // for each texture we get the filename and the desired ID and add them to the texture manager
    for(TiXmlElement* e = pStateRoot->FirstChildElement(); e != NULL; e = e->NextSiblingElement())
    {
        string filenameAttribute = e->Attribute("filename");
        string idAttribute = e->Attribute("ID");
        
        pTextureIDs->push_back(idAttribute); // push the id into the list
        
        TheTextureManager::Instance()->load(filenameAttribute, idAttribute, TheGame::Instance()->getRenderer());
    }
}

void StateParser::parseObjects(TiXmlElement *pStateRoot, std::vector<GameObject *> *pObjects)
{
    for(TiXmlElement* e = pStateRoot->FirstChildElement(); e != NULL; e = e->NextSiblingElement())
    {
        int x, y, width, height, numFrames, callbackID, animSpeed;
        string textureID;
        
        e->Attribute("x", &x);
        e->Attribute("y", &y);
        e->Attribute("width",&width);
        e->Attribute("height", &height);
        e->Attribute("numFrames", &numFrames);
        e->Attribute("callbackID", &callbackID);
        e->Attribute("animSpeed", &animSpeed);
        
        textureID = e->Attribute("textureID");
        //int x, int y, int width, int height, std::string textureID, int numFrames, void()
         GameObject* pGameObject = TheGameObjectFactory::Instance()->create(e->Attribute("type"));
		 //pGameObject->SetOwner(GetOwner());

		LoaderParams param = LoaderParams(x, y, width, height, textureID, numFrames, callbackID, animSpeed);
		pGameObject->Load(param);
       
        pObjects->push_back(pGameObject);
    }
}