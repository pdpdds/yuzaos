#pragma once
#include "Trigger.h"

class GameObject;

class TriggerGenerator
{
public:
	TriggerGenerator();
	virtual ~TriggerGenerator();

	Trigger<GameObject>* GenerateItemTrigger(int tileX, int tileY);

private:
	int m_totalTileNum;
	int m_generatedItemCount;
	int m_maxGeneratedItemCount;
	int m_probability;
};

