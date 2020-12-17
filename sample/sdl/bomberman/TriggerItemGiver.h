#pragma once
#include "GameObject.h"
#include "TriggerLimitedLifeTime.h"

class TriggerItemGiver : public TriggerLimitedLifeTime<GameObject>
{
	RTTI_DECLARATIONS(TriggerItemGiver, TriggerLimitedLifeTime<GameObject>)
public:
	TriggerItemGiver(int itemType);
	virtual ~TriggerItemGiver();

	virtual void Load(LoaderParams& params);

	// draw the object
	virtual void Draw();

	// remove anything that needs to be deleted
	virtual void Clean(){}

	// object has collided, handle accordingly
	virtual void Collision(){}

	// get the type of the object
	virtual std::string type(){ return "TriggerItemGiver"; }

	virtual void  Try(GameObject* pGameObject);

private:
	int m_itemType;	
	bool m_invincible;

};

