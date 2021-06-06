#include <BuildOption.h>
#include "PIT.h"
#include "intrinsic.h"

volatile unsigned int _pitTicks = 0;

//타이머를 시작
void StartPITCounter(uint32_t freq, uint8_t counter, uint8_t mode) 
{
	if (freq == 0)
		return;

#if SKY_EMULATOR
	extern bool StartWin32Timer();
	StartWin32Timer();
#else
	uint16_t divisor = uint16_t(1193181 / (uint16_t)freq);

	//커맨드 전송
	uint8_t ocw = 0;
	ocw = (ocw & ~I86_PIT_OCW_MASK_MODE) | mode;
	ocw = (ocw & ~I86_PIT_OCW_MASK_RL) | I86_PIT_OCW_RL_DATA;
	ocw = (ocw & ~I86_PIT_OCW_MASK_COUNTER) | counter;
	SendPITCommand(ocw);

	//프리퀀시 비율 설정
	SendPITData(divisor & 0xff, 0);
	SendPITData((divisor >> 8) & 0xff, 0);

	//타이머 틱 카운트 리셋
	_pitTicks = 0;
#endif
}

void SendPITCommand(uint8_t cmd) 
{
	OutPortByte(I86_PIT_REG_COMMAND, cmd);
}

void SendPITData(uint16_t data, uint8_t counter) {

	uint8_t	port = (counter == I86_PIT_OCW_COUNTER_0) ? I86_PIT_REG_COUNTER0 :
		((counter == I86_PIT_OCW_COUNTER_1) ? I86_PIT_REG_COUNTER1 : I86_PIT_REG_COUNTER2);

	OutPortByte(port, (uint8_t)data);
}