/*
 * Realmode Emulator
 * - Call / Return (Including interrupts)
 */
#include "rme.h"
#include "rme_internal.h"
#include "opcode_prototypes.h"

// === CODE ===
DEF_OPCODE_FCN(CALL, N)
{
	 int	ret;
	uint16_t	dist;
	READ_INSTR16( dist );
	DEBUG_S(" .+0x%04x", dist);
	PUSH(State->IP + State->Decoder.IPOffset);
	State->IP += dist;
	return 0;
}

DEF_OPCODE_FCN(CALL, F)
{
	 int	ret;
	uint16_t	ofs, seg;
	READ_INSTR16( ofs );
	READ_INSTR16( seg );
	DEBUG_S(" 0x%04x:%04x", seg, ofs);
	PUSH(State->CS);
	PUSH(State->IP + State->Decoder.IPOffset);
	State->CS = seg;
	State->IP = ofs;
	State->Decoder.bDontChangeIP = 1;
	return 0;
}

DEF_OPCODE_FCN(RET, N)
{
	 int	ret;
	uint16_t	ofs;
	POP(ofs);
	State->IP = ofs;
	State->Decoder.bDontChangeIP = 1;
	return 0;
}
DEF_OPCODE_FCN(RET, F)
{
	 int	ret;
	uint16_t	ofs, seg;
	POP(ofs);
	POP(seg);
	State->CS = seg;
	State->IP = ofs;
	State->Decoder.bDontChangeIP = 1;
	return 0;
}
DEF_OPCODE_FCN(RET, iN)	// Return, and pop imm16 bytes from stack
{
	 int	ret;
	uint16_t	ofs;
	uint16_t	popSize;
	READ_INSTR16(popSize);
	POP(ofs);
	State->IP = ofs;
	State->SP.W += popSize;
	State->Decoder.bDontChangeIP = 1;
	return 0;
}
DEF_OPCODE_FCN(RET, iF)	// Return, and pop imm16 bytes from stack
{
	 int	ret;
	uint16_t	ofs, seg;
	uint16_t	popSize;
	READ_INSTR16(popSize);
	POP(ofs);
	POP(seg);
	State->CS = seg;
	State->IP = ofs;
	State->SP.W += popSize;
	State->Decoder.bDontChangeIP = 1;
	return 0;
}

DEF_OPCODE_FCN(ENTER, z)
{
	 int	ret;
	uint16_t	size;
	uint16_t	frameTemp;
	uint8_t 	level;
	READ_INSTR16(size);
	DEBUG_S(" 0x%04x", size);
	READ_INSTR8(level);
	level &= 32-1;
	DEBUG_S(" %i", level);

	// No operand override: selected by stack size	
	PUSH(State->BP.W);
	frameTemp = State->SP.W;
	
	if( level > 1 )
	{
		 int	i;
		for( i = 1; i < level; i ++ )
		{
			if( State->Decoder.bOverrideOperand )
			{
				ERROR_S("- ENTER Operand Override unimplimented");
				return RME_ERR_UNDEFOPCODE;
				//uint32_t val;
				//State->BP.W -= 4;
				//RME_Int_Read32(State, State->SS, State->BP.W, &val);
				//PUSH32(val);
			}
			else
			{
				uint16_t	val;
				State->BP.W -= 2;
				ret = RME_Int_Read16(State, State->SS, State->BP.W, &val);
				if(ret)	return ret;
				PUSH(val);
			}
		}
	
		if( State->Decoder.bOverrideOperand )
		{
			//PUSH32(frameTemp);
		}
		else
		{
			PUSH(frameTemp);
		}
	}
	
	// StackSize==16
	State->BP.W = frameTemp;
	if( State->SP.W < size ) {
		// Exception #SS
	}
	State->SP.W -= size;
	
	return 0;
}

DEF_OPCODE_FCN(LEAVE, z)
{
	 int	ret;
	State->SP.W = State->BP.W;
	if( State->Decoder.bOverrideOperand ) {
		//POP32(State->BP.D);
	}
	else
		POP(State->BP.W);
	return 0;
}

static inline int _CallInterrupt(tRME_State *State, int Num)
{
	 int	ret;
	uint16_t	seg, ofs;
	
	// Full emulation then
	ret = RME_Int_Read16(State, 0, Num*4+0, &ofs);	// Offset
	if(ret)	return ret;
	ret = RME_Int_Read16(State, 0, Num*4+2, &seg);	// Segment
	if(ret)	return ret;
	
	if(ofs == 0 && seg == 0) {
		ERROR_S(" Caught attempt to execute IVT pointing to 0000:0000");
		return RME_ERR_BADMEM;
	}
	
	// High-Level Emulation Call (only if not overridden)
	if( seg == RME_HLE_CS && ofs == Num )
	{
		if( State->HLECallbacks[Num] )
			State->HLECallbacks[Num](State, Num);
		return 0;
	}
	
	PUSH( State->Flags );
	PUSH( State->CS );
	PUSH( State->IP + State->Decoder.IPOffset );
	State->IP = ofs;
	State->CS = seg;
	State->Flags &= ~(FLAG_IF|FLAG_TF);	

	State->Decoder.bDontChangeIP = 1;
	
	return 0;
}

// Interrupts
DEF_OPCODE_FCN(INT, 3)	// INT 0x3 - Debug
{
	return _CallInterrupt(State, 3);
}
DEF_OPCODE_FCN(INT, I)	// INT imm8
{
	uint8_t	num;
	READ_INSTR8(num);
	DEBUG_S(" 0x%x", num);
	return _CallInterrupt(State, num);
}
DEF_OPCODE_FCN(INTO, z)	// INTO - INT 4 if OF
{
	if( State->Flags & FLAG_OF )
		return _CallInterrupt(State, 4);
	else
		return 0;
}
DEF_OPCODE_FCN(IRET, z)	// Interrupt Return
{
	 int	ret;
	POP( State->IP );
	POP( State->CS );
	POP( State->Flags );
	State->Decoder.bDontChangeIP = 1;
	return 0;
}
