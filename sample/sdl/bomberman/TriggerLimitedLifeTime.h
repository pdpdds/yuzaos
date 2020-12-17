#ifndef TRIGGERLIMITEDLIFETIME_H
#define TRIGGERLIMITEDLIFETIME_H
#pragma warning (disable:4786)
//-----------------------------------------------------------------------------
//
//  Name:     Trigger_LimitedLifetime.h
//
//  Author:   Mat Buckland
//
//  Desc:     defines a trigger that only remains in the game for a specified
//            number of update steps
//
//-----------------------------------------------------------------------------
#include "Trigger.h"


template <class entity_type>
class TriggerLimitedLifeTime : public Trigger<entity_type>
{
protected:

  //the lifetime of this trigger in update-steps
  int m_iLifetime;

public:

	TriggerLimitedLifeTime(int lifetime) :Trigger<entity_type>(1),
                                        m_iLifetime(lifetime)
  {}

	virtual ~TriggerLimitedLifeTime(){}

  //children of this class should always make sure this is called from within
  //their own update method
  virtual void Update()
  {
    //if the lifetime counter expires set this trigger to be removed from
    //the game
    if (--m_iLifetime <= 0)
    {
      SetToBeRemovedFromGame();
    }
  }

  //to be implemented by child classes
  virtual void  Try(entity_type*) = 0;
};


#endif