#ifndef REGULATOR
#define REGULATOR
//------------------------------------------------------------------------
//
//  Name:   Regulator.h
//
//  Desc:   Use this class to regulate code flow (for an update function say)
//          Instantiate the class with the frequency you would like your code
//          section to flow (like 10 times per second) and then only allow 
//          the program flow to continue if Ready() returns true
//
//  Author: Mat Buckland 2003 (fup@ai-junkie.com)
//
//------------------------------------------------------------------------

class Regulator
{
public:
	Regulator(double NumUpdatesPerSecondRqd);
	bool isReady();

private:
	double m_dUpdatePeriod;	
	unsigned int m_dwNextUpdateTime;
};

#endif