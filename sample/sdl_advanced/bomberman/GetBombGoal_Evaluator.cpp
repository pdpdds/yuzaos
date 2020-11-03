#include "GetBombGoal_Evaluator.h"
#include "Goal_Think.h"
#include "Goal_Types.h"
#include "Level.h"
#include "TriggerSystem.h"

//---------------------- CalculateDesirability -------------------------------------
//-----------------------------------------------------------------------------
double GetBombGoal_Evaluator::CalculateDesirability(Player* pBot)
{
	double Desirability = 0.05;

	Desirability *= m_dCharacterBias;

	Level* pLevel = pBot->GetOwner();
	TriggerSystem<Trigger<GameObject>>* pTriggerSystem = pLevel->GetTriggerSystem();
	const TriggerSystem<Trigger<GameObject>>::TriggerList& triggerList = pTriggerSystem->GetTriggers();
	if (triggerList.size() == 0)
		Desirability *= 0.5;
	else Desirability *= 1.4;

	return Desirability;
}



//----------------------------- SetGoal ---------------------------------------
//-----------------------------------------------------------------------------
void GetBombGoal_Evaluator::SetGoal(Player* pBot)
{
	pBot->GetBrain()->AddGoal_GetItem(type_bomb);
}

//-------------------------- RenderInfo ---------------------------------------
//-----------------------------------------------------------------------------
void GetBombGoal_Evaluator::RenderInfo(Vector2D Position, Player* pBot)
{
  /*gdi->TextAtPos(Position, "H: " + ttos(CalculateDesirability(pBot), 2));
  return;
  
  std::string s = ttos(1-Raven_Feature::Health(pBot)) + ", " + ttos(Raven_Feature::DistanceToItem(pBot, type_health));
  gdi->TextAtPos(Position+Vector2D(0,15), s);*/
}