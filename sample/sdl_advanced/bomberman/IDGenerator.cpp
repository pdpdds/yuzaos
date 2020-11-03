//#include "stdafx.h"
#include "IDGenerator.h"


IDGenerator::IDGenerator()
: m_id(0)
{
}


IDGenerator::~IDGenerator()
{
}

IDGenerator* IDGenerator::Instance()
{
	static IDGenerator instance;

	return &instance;
}