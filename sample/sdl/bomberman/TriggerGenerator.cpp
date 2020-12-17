//#include "stdafx.h"
#include "TriggerGenerator.h"
#include "TriggerItemGiver.h"
#include "GameObject.h"

TriggerGenerator::TriggerGenerator()
{
	m_totalTileNum = 100;
	m_generatedItemCount = 0;
	m_maxGeneratedItemCount = 100;
	m_probability = 100;
}


TriggerGenerator::~TriggerGenerator()
{
}

Trigger<GameObject>* TriggerGenerator::GenerateItemTrigger(int tileRow, int tileColumn)
{
	int range = rand() % 100;

	if (range > m_probability)
		return 0;

	if (m_generatedItemCount >= m_maxGeneratedItemCount)
		return 0;

	m_generatedItemCount++;

	int determinedItem = rand() % MAX_TRIGGER_TYPE;

	std::string triggerType;

	switch (determinedItem)
	{
	case TRIGGER_BOMB:
	{
		triggerType = "data/image/item/bombamount.png";
	}
	break;
	case TRIGGER_BOMBPOWER:
	{
		triggerType = "data/image/item/bombpower.png";
	}
	break;
	case TRIGGER_SPEEDUP:
	{
		triggerType = "data/image/item/speedup.png";
	}
	break;
	case TRIGGER_TNT:
	{
		triggerType = "data/image/item/powerup_special_big.png";
	}
	break;
	case TRIGGER_COIN10:
	{
		triggerType = "data/image/item/coins10.png";
	}
	break;
	case TRIGGER_COIN20:
	{
		triggerType = "data/image/item/coins20.png";
	}
	break;
	case TRIGGER_COIN30:
	{
		triggerType = "data/image/item/coins30.png";
	}
	break;
	}


	TriggerItemGiver* pTriggerItemGiver = new TriggerItemGiver(determinedItem);

	Vector2D leftTop(tileColumn * 32, tileRow * 32);
	Vector2D bottomRight(tileColumn * 32 + 32, tileRow * 32 + 32);

	pTriggerItemGiver->SetActive();
	pTriggerItemGiver->AddRectangularTriggerRegion(leftTop, bottomRight);
	LoaderParams param = LoaderParams(tileColumn * 32, tileRow * 32, 32, 32, triggerType, 1, 0, 0);
	pTriggerItemGiver->Load(param);

	return pTriggerItemGiver;
}