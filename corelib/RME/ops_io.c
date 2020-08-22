/*
 * Realmode Emulator
 * - IO Operations
 * 
 */
#include "rme.h"
#include "rme_internal.h"
#include "opcode_prototypes.h"
#include "ops_io.h"

// === CODE ===
DEF_OPCODE_FCN(IN, ADx)
{
	DEBUG_S(" DX AL");
	return inB(State, State->DX.W, &State->AX.B.L);
}

DEF_OPCODE_FCN(IN, ADxX)
{
	if( State->Decoder.bOverrideOperand )
	{
		DEBUG_S(" DX EAX");
		return inD(State, State->DX.W, &State->AX.D);
	}
	else
	{
		DEBUG_S("DX AX");
		return inW(State, State->DX.W, &State->AX.W);
	}
}

DEF_OPCODE_FCN(IN, AI)
{
	uint8_t	port;
	READ_INSTR8(port);
	DEBUG_S(" 0x%02x AL", port);
	return inB(State, port, &State->AX.B.L);
}

DEF_OPCODE_FCN(IN, AIX)
{
	uint8_t	port;
	READ_INSTR8(port);
	DEBUG_S(" 0x%02x", port);
	if( State->Decoder.bOverrideOperand )
	{
		DEBUG_S(" EAX");
		return inD(State, port, &State->AX.D);
	}
	else
	{
		DEBUG_S(" AX");
		return inW(State, port, &State->AX.W);
	}
}

DEF_OPCODE_FCN(OUT, DxA)
{
	DEBUG_S(" AL DX");
	return outB(State, State->DX.W, State->AX.B.L);
}
DEF_OPCODE_FCN(OUT, DxAX)
{
	if( State->Decoder.bOverrideOperand )
	{
		DEBUG_S(" EAX DX");
		return outD(State, State->DX.W, State->AX.D);
	}
	else
	{
		DEBUG_S(" AX DX");
		return outW(State, State->DX.W, State->AX.W);
	}
}

DEF_OPCODE_FCN(OUT, AI)
{
	uint8_t	port;
	READ_INSTR8(port);
	DEBUG_S(" AL 0x%02x", port);
	return outB(State, port, State->AX.B.L);
}

DEF_OPCODE_FCN(OUT, AIX)
{
	uint8_t	port;
	READ_INSTR8(port);
	if( State->Decoder.bOverrideOperand )
	{
		DEBUG_S(" EAX 0x%02x", port);
		return outD(State, port, State->AX.D);
	}
	else
	{
		DEBUG_S(" AX 0x%02x", port);
		return outW(State, port, State->AX.W);
	}
}
