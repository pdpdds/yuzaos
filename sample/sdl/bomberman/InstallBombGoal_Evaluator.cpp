#include "InstallBombGoal_Evaluator.h"
#include "Goal_Think.h"
#include "Goal_Types.h"
#include "Level.h"
#include "TriggerSystem.h"

//---------------------- CalculateDesirability -------------------------------------
//-----------------------------------------------------------------------------
double InstallBombGoal_Evaluator::CalculateDesirability(Player* pBot)
{
	double Desirability = 0.05;

	Desirability *= m_dCharacterBias;

	if(pBot->GetPlayerStat().currentBombCount == 0)	
		Desirability *= 1.2;
	else
		Desirability *= 0.9;

	return Desirability;
}



//----------------------------- SetGoal ---------------------------------------
//-----------------------------------------------------------------------------
void InstallBombGoal_Evaluator::SetGoal(Player* pBot)
{
	pBot->GetBrain()->AddGoal_InstallBomb();
}

//-------------------------- RenderInfo ---------------------------------------
//-----------------------------------------------------------------------------
void InstallBombGoal_Evaluator::RenderInfo(Vector2D Position, Player* pBot)
{
	/*gdi->TextAtPos(Position, "H: " + ttos(CalculateDesirability(pBot), 2));
	return;

	std::string s = ttos(1-Raven_Feature::Health(pBot)) + ", " + ttos(Raven_Feature::DistanceToItem(pBot, type_health));
	gdi->TextAtPos(Position+Vector2D(0,15), s);*/
}