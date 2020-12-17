#include "Goal_GetItem.h"
#include "PathPlanner.h"

#include "Telegram.h"
#include "Messages.h"

#include "Goal_Wander.h"
#include "Goal_FollowPath.h"
#include "Level.h"


int ItemTypeToGoalType(int gt)
{
  switch(gt)
  {
  case type_bomb:

	  return goal_get_bomb;

  default: 
      assert(0);
      //throw std::runtime_error("Goal_GetItem cannot determine item type");

  }//end switch
}

//------------------------------- Activate ------------------------------------
//-----------------------------------------------------------------------------
void Goal_GetItem::Activate()
{
  m_iStatus = active;
  
  m_pGiverTrigger = 0;
  
  Vector2D itemPos = m_pOwner->GetOwner()->GetRandomItemPos(m_iItemToGet);
  //request a path to the item
  //m_pOwner->GetPathPlanner()->RequestPathToItem(m_iItemToGet);
  m_pOwner->GetPathPlanner()->RequestPathToPosition(itemPos);

  //the bot may have to wait a few update cycles before a path is calculated
  //so for appearances sake it just wanders
  AddSubgoal(new Goal_Wander(m_pOwner));
}

//-------------------------- Process ------------------------------------------
//-----------------------------------------------------------------------------
int Goal_GetItem::Process()
{
  ActivateIfInactive();

  if (hasItemBeenStolen())
  {
    Terminate();
  }

  else
  {
    //process the subgoals
    m_iStatus = ProcessSubgoals();
  }

  return m_iStatus;
}
//---------------------------- HandleMessage ----------------------------------
//-----------------------------------------------------------------------------
bool Goal_GetItem::HandleMessage(const Telegram& msg)
{
  //first, pass the message down the goal hierarchy
  bool bHandled = ForwardMessageToFrontMostSubgoal(msg);

  //if the msg was not handled, test to see if this goal can handle it
  if (bHandled == false)
  {
    switch(msg.Msg)
    {
    case Msg_PathReady:

      //clear any existing goals
      RemoveAllSubgoals();

      AddSubgoal(new Goal_FollowPath(m_pOwner,
                                     m_pOwner->GetPathPlanner()->GetPath()));

      //get the pointer to the item
	  m_pGiverTrigger = static_cast<Trigger<GameObject>*>(msg.ExtraInfo);

      return true; //msg handled


    case Msg_NoPathAvailable:

      m_iStatus = failed;

      return true; //msg handled

    default: return false;
    }
  }

  //handled by subgoals
  return true;
}

//---------------------------- hasItemBeenStolen ------------------------------
//
//  returns true if the bot sees that the item it is heading for has been
//  picked up by an opponent
//-----------------------------------------------------------------------------
bool Goal_GetItem::hasItemBeenStolen()const
{
 /* if (m_pGiverTrigger &&
      !m_pGiverTrigger->isActive() &&
      m_pOwner->hasLOSto(m_pGiverTrigger->Pos()) )
  {
    return true;
  }*/

  return false;
}