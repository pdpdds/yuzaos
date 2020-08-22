/*
 * Realmode Emulator
 * - Stack (Push/Pop) Operations
 */
#include "rme.h"
#include "rme_internal.h"
#include "opcode_prototypes.h"
// NOTE: Only does 16-bit pushes

// === CODE ===
DEF_OPCODE_FCN(PUSH, Seg)
{
	 int	ret;
	uint16_t	*ptr = Seg(State, Param);
	PUSH(*ptr);
	return 0;
}

DEF_OPCODE_FCN(POP, Seg)
{
	uint16_t	*ptr = Seg(State, Param);
	 int	ret;
	POP(*ptr);
	return 0;
}

DEF_OPCODE_FCN(PUSH, Reg)
{
	 int	ret;
	uint16_t	*ptr = RegW(State, Param);
	if( State->Decoder.bOverrideOperand )
		PUSH(ptr[1]);
	PUSH(*ptr);
	return 0;
}

DEF_OPCODE_FCN(POP, Reg)
{
	 int	ret;
	uint16_t	*ptr = RegW(State, Param);
	POP(*ptr);
	if( State->Decoder.bOverrideOperand )
		POP(ptr[1]);
	return 0;
}

DEF_OPCODE_FCN(PUSH, A)
{
	 int	ret;
	uint16_t	pt2;
	if( State->Decoder.bOverrideOperand )	return RME_ERR_UNDEFOPCODE;
	pt2 = State->SP.W;
	PUSH(State->AX.W);	PUSH(State->CX.W);
	PUSH(State->DX.W);	PUSH(State->BX.W);
	PUSH(pt2);        	PUSH(State->BP.W);
	PUSH(State->SI.W);	PUSH(State->DI.W);
	return 0;
}

DEF_OPCODE_FCN(POP, A)
{
	 int	ret;
	if( State->Decoder.bOverrideOperand )	return RME_ERR_UNDEFOPCODE;
	POP(State->DI.W);	POP(State->SI.W);
	POP(State->BP.W);	State->SP.W += 2;
	POP(State->BX.W);	POP(State->DX.W);
	POP(State->CX.W);	POP(State->AX.W);
	return 0;
}

DEF_OPCODE_FCN(PUSH, F)
{
	 int	ret;
	if(State->Decoder.bOverrideOperand)	return RME_ERR_UNDEFOPCODE;
	PUSH(State->Flags);
	return 0;
}

DEF_OPCODE_FCN(POP, F)
{
	 int	ret;
	uint16_t	tmp;
	const uint16_t	masks[][2] = {
		//        always:  set     clear
		[RME_CPU_8086]  = {0xF002, 0x0028},
		[RME_CPU_80286] = {0x0002, 0xF028},
		[RME_CPU_386]   = {0x0002, 0x0028},
		};
	if(State->Decoder.bOverrideOperand)	return RME_ERR_UNDEFOPCODE;
	POP(tmp);
	tmp |= masks[State->CPUType][0];
	tmp &= ~masks[State->CPUType][1];
	State->Flags = tmp;
	return 0;
}

DEF_OPCODE_FCN(PUSH, MX)
{
	 int	ret;
	void	*destPtr;
	if(State->Decoder.bOverrideOperand)	return RME_ERR_UNDEFOPCODE;
	
	ret = RME_Int_ParseModRMX(State, NULL, (void*)&destPtr, 0);
	if(ret)	return ret;
	
	if( State->Decoder.bOverrideOperand )
	{
		uint32_t	*ptr = destPtr;
		PUSH(*ptr >> 16);
		PUSH(*ptr);
	}
	else
	{
		uint16_t	*ptr = destPtr;
		PUSH(*ptr);
	}
	return 0;
}
DEF_OPCODE_FCN(POP, MX)
{
	 int	ret;
	uint16_t	*ptr;
	if( State->Decoder.bOverrideOperand )	return RME_ERR_UNDEFOPCODE;
	
	ret = RME_Int_ParseModRMX(State, NULL, &ptr, 0);
	if(ret)	return ret;
	POP(*ptr);
	return 0;
}

DEF_OPCODE_FCN(PUSH, I)
{
	 int	ret;
	uint32_t	val;
	READ_INSTR16( val );
	DEBUG_S(" 0x%04x", val);
	if( State->Decoder.bOverrideOperand )
		PUSH(val>>16);
	PUSH(val);
	return 0;
}

DEF_OPCODE_FCN(PUSH, I8)
{
	 int	ret;
	uint32_t	val;
	READ_INSTR8S( val );
	DEBUG_S(" 0x%02x", val);
	if( State->Decoder.bOverrideOperand )
		PUSH(val>>16);
	PUSH(val);
	return 0;
}
