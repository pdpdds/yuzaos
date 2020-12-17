#include "ExploreGoal_Evaluator.h"
#include "PathPlanner.h"
//#include "../Raven_ObjectEnumerations.h"
//#include "../lua/Raven_Scriptor.h"
//#include "misc/Stream_Utility_Functions.h"
//#include "Raven_Feature.h"

#include "Goal_Think.h"
#include "Goal_Types.h"




//---------------- CalculateDesirability -------------------------------------
//-----------------------------------------------------------------------------
double ExploreGoal_Evaluator::CalculateDesirability(Player* pBot)
{
  double Desirability = 0.05;

  Desirability *= m_dCharacterBias;

  return Desirability;
}

//----------------------------- SetGoal ---------------------------------------
//-----------------------------------------------------------------------------
void ExploreGoal_Evaluator::SetGoal(Player* pBot)
{
  pBot->GetBrain()->AddGoal_Explore();
}

//-------------------------- RenderInfo ---------------------------------------
//-----------------------------------------------------------------------------
void ExploreGoal_Evaluator::RenderInfo(Vector2D Position, Player* pBot)
{
 // gdi->TextAtPos(Position, "EX: " + ttos(CalculateDesirability(pBot), 2));
}