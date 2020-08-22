#pragma once
class InterruptDisabler
{
public:
	InterruptDisabler();
	~InterruptDisabler();

private:
	int m_flags;
};

