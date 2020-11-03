#pragma once
class IDGenerator
{
public:
	static IDGenerator* Instance();

	int GetNextId(){ return m_id++;}

private:
	IDGenerator();
	virtual ~IDGenerator();

	int m_id;
};

