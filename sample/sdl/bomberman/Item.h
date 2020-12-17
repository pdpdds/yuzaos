#pragma once
#include "PlatformerObject.h"

class Item : public PlatformerObject
{
public:
	Item(int itemType);
	virtual ~Item();

	virtual void Load(LoaderParams& params);

	virtual void Draw();
	virtual void Update();
	virtual void Clean();

	virtual void Collision();

	virtual std::string type() { return "Item"; }

	int GetItemType(){ return m_itemType; }

public:
	int m_lifeTime;
	int m_generatedTime;
	int m_itemType;
};

