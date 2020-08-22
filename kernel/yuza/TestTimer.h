#pragma once
#include <Timer.h>

class TestTimer : public Timer
{
public:
	TestTimer();
	~TestTimer();

	virtual InterruptStatus HandleTimeout() override;
};

