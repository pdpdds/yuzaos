#ifndef CRUDETIMER_H
#define CRUDETIMER_H


#define Clock CrudeTimer::Instance()

class CrudeTimer
{
private:
  

  //set to the time (in seconds) when class is instantiated
  double m_dStartTime;

  //set the start time
  CrudeTimer();

  //copy ctor and assignment should be private
  CrudeTimer(const CrudeTimer&);
  CrudeTimer& operator=(const CrudeTimer&);
  
public:

  static CrudeTimer* Instance();

  //returns how much time has elapsed since the timer was started
  double GetCurTime();
  

};







#endif