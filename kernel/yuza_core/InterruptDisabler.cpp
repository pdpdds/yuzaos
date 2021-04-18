#include "InterruptDisabler.h"
#include <intrinsic.h>

InterruptDisabler::InterruptDisabler()
{
	m_flags = DisableInterrupts();
}

InterruptDisabler::~InterruptDisabler()
{
	RestoreInterrupts(m_flags);
}