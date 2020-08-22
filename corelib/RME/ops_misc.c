/*
 * Realmode Emulator
 * - Misc operations
 */
#include "rme.h"
#include "rme_internal.h"
#include "opcode_prototypes.h"

// === CODE ===
DEF_OPCODE_FCN(CBW, z)	// Convert signed Byte to Word
{
	if( State->Decoder.bOverrideOperand )
		State->AX.D = (uint32_t)( (int8_t)State->AX.B.L );
	else
		State->AX.W = (uint16_t)( (int8_t)State->AX.B.L );
	return 0;
}

DEF_OPCODE_FCN(BSF,z)
{
	int ret;
	uint16_t	*dest, *src;
	
	ret = RME_Int_ParseModRMX(State, &dest, &src, 0);
	if(ret)	return ret;
	
	if( *src == 0 )
	{
		State->Flags |= FLAG_ZF;
	}
	else
	{
		const int width = 16;
		 int	val = 0;
		while(val < width && 0 == ((*src) & (1 << val)))
			val ++;
		*dest = val;
		State->Flags &= ~FLAG_ZF;
	}
	return 0;
}

// 0xf4 - Halt execution
DEF_OPCODE_FCN(HLT, z)
{
	return RME_ERR_HALT;
}

// 0x37 - ASCII adjust AL after Addition
DEF_OPCODE_FCN(AAA, z)
{
	if( (State->AX.B.L & 0xF) > 9 || (State->Flags & FLAG_AF) )
	{
		State->AX.B.L += 6;
		State->AX.B.H += 1;
		State->Flags |= FLAG_AF|FLAG_CF;
		State->AX.B.L &= 0xF;
	}
	else
	{
		State->Flags &= ~(FLAG_AF|FLAG_CF);
		State->AX.B.L &= 0xF;
	}
	SET_COMM_FLAGS(State, State->AX.B.L, 8);
	return 0;
}

// 0x3F - ASCII adjust AL after Subtraction
DEF_OPCODE_FCN(AAS, z)
{
	if( (State->AX.B.L & 0xF) > 9 || (State->Flags & FLAG_AF) )
	{
		State->AX.B.L -= 6;
		State->AX.B.H -= 1;
		State->Flags |= FLAG_AF|FLAG_CF;
		State->AX.B.L &= 0xF;
	}
	else
	{
		State->Flags &= ~(FLAG_AF|FLAG_CF);
		State->AX.B.L &= 0xF;
	}
	SET_COMM_FLAGS(State, State->AX.B.L, 8);
	return 0;
}

// 0xD4 - ASCII adjust AL after Multiply
DEF_OPCODE_FCN(AAM, z)
{
	uint8_t	imm8;
	READ_INSTR8(imm8);
	if(imm8 == 0)	return RME_ERR_DIVERR;
	State->AX.B.H = State->AX.B.L / imm8;
	State->AX.B.L = State->AX.B.L % imm8;
	SET_COMM_FLAGS(State, State->AX.B.L, 8);
	return 0;
}
// 0xD5 - ASCII adjust AL before Division
DEF_OPCODE_FCN(AAD, z)
{
	uint8_t	imm8;
	READ_INSTR8(imm8);
	State->AX.B.L += State->AX.B.H * imm8;
	State->AX.B.H = 0;
	SET_COMM_FLAGS(State, State->AX.B.L, 8);
	State->Flags &= ~(FLAG_OF|FLAG_CF);	// Effect undefined, cleared for Artlav tests
	return 0;
}

// 0x27 - Decimal adjust AL after Addition
DEF_OPCODE_FCN(DAA, z)
{
	 int	old_CF = !!(State->Flags & FLAG_CF);
	uint8_t	old_AL = State->AX.B.L;
	State->Flags &= ~FLAG_CF;
	if( (State->AX.B.L & 0xF) > 9 || (State->Flags & FLAG_AF) )
	{
		if( State->AX.B.L >= 0x100-6 )
			State->Flags |= FLAG_CF;
		State->AX.B.L += 6;
		State->Flags |= FLAG_AF;
	}
	else
	{
		State->Flags &= ~FLAG_AF;
	}
	
	if( old_AL > 0x99 || old_CF )
	{
		State->AX.B.L += 0x60;
		State->Flags |= FLAG_CF;
	}
	else
	{
		State->Flags &= ~FLAG_CF;
	}
	SET_COMM_FLAGS(State, State->AX.B.L, 8);
	return 0;
}

// 0x2F - Decimal adjust AL after Subtraction
DEF_OPCODE_FCN(DAS, z)
{
	 int	old_CF = !!(State->Flags & FLAG_CF);
	uint8_t	old_AL = State->AX.B.L;
	State->Flags &= ~FLAG_CF;
	if( (State->AX.B.L & 0xF) > 9 || (State->Flags & FLAG_AF) )
	{
		if( State->AX.B.L < 6 )
			State->Flags |= FLAG_CF;
		State->AX.B.L -= 6;
		State->Flags |= FLAG_AF;
	}
	else
	{
		State->Flags &= ~FLAG_AF;
	}
	
	if( old_AL > 0x99 || old_CF )
	{
		State->AX.B.L -= 0x60;
		State->Flags |= FLAG_CF;
	}
	SET_COMM_FLAGS(State, State->AX.B.L, 8);
	return 0;
}

// 0x99 - Convert Word to Doubleword
DEF_OPCODE_FCN(CWD, z)
{
	if( State->Decoder.bOverrideOperand )
	{
		if( State->AX.D & 0x80000000 )
			State->DX.D = 0xFFFFFFFF;
		else
			State->DX.D = 0;
	}
	else
	{
		if( State->AX.W & 0x8000 )
			State->DX.W = 0xFFFF;
		else
			State->DX.W = 0;
	}
	return 0;
}

/**
 * \brief Internals of LDS and LES 
 * \note Does ModR/M parsing for it self to avoid issues with alignment
 */
static inline int _LDS_LES_internal(tRME_State *State, uint16_t *SegRegPtr)
{
	 int	ret;
	uint16_t	seg, *dest;
	uint32_t	addr;
	 int	mod, rrr, mmm;
	
	RME_Int_GetModRM(State, &mod, &rrr, &mmm);
	
	if( mod == 3 )
		return RME_ERR_UNDEFOPCODE;	// Source cannot be a register
	
	ret = RME_Int_GetMMM(State, mod, mmm, &seg, &addr);
	if(ret)	return ret;
	dest = RegW(State, rrr);
	
	// Get address of the destination
	
	if( State->Decoder.bOverrideOperand )
	{
		ret = RME_Int_Read32(State, seg, addr, (uint32_t*)dest);
		if(ret)	return ret;
		addr += 4;
	}
	else
	{
		ret = RME_Int_Read16(State, seg, addr, dest);
		if(ret)	return ret;
		addr += 2;
	}

	// Get segment
	ret = RME_Int_Read16(State, seg, addr, SegRegPtr);
	if(ret)	return ret;
	
	return 0;
}

DEF_OPCODE_FCN(LES, z)	// Load ES:r16/32 with m16:m16/32
{
	return _LDS_LES_internal(State, &State->ES);
}

DEF_OPCODE_FCN(LDS, z)	// Load DS:r16/32 with m16:m16/32
{
	return _LDS_LES_internal(State, &State->DS);
}
DEF_OPCODE_FCN(LFS, z)
{
	return _LDS_LES_internal(State, &State->FS);
}
DEF_OPCODE_FCN(LGS, z)
{
	return _LDS_LES_internal(State, &State->GS);
}
DEF_OPCODE_FCN(LSS, z)
{
	return _LDS_LES_internal(State, &State->SS);
}

DEF_OPCODE_FCN(LEA, z)
{
	 int	ret;
	uint16_t	seg, *dest;
	uint32_t	addr;
	 int	mod, rrr, mmm;
	
	RME_Int_GetModRM(State, &mod, &rrr, &mmm);
	
	if( mod == 3 )
		return RME_ERR_UNDEFOPCODE;	// Source cannot be a register
	
	dest = RegW(State, rrr);
	
	ret = RME_Int_GetMMM(State, mod, mmm, &seg, &addr);
	if(ret)	return ret;
	
	if( State->Decoder.bOverrideOperand )
		*(uint32_t*)dest = addr;
	else
		*dest = addr;
	return 0;
}

DEF_OPCODE_FCN(XLAT, z)
{
	void	*ptr;
	uint32_t	address;
	uint16_t	seg;
	 int	rv;

	seg = *Seg(State, State->Decoder.OverrideSegment == -1 ? SREG_DS : State->Decoder.OverrideSegment);

	if( State->Decoder.bOverrideAddress ) {
		DEBUG_S("[EBX+AL]");
		address = State->BX.D;
	}
	else {
		DEBUG_S("[BX+AL]");
		address = State->BX.W;
	}
	address += State->AX.B.L;

	if( (rv = RME_Int_GetPtr(State, seg, address, &ptr)) )
		return rv;

	State->AX.B.L = *(uint8_t*)ptr;
	return 0;
}

DEF_OPCODE_FCN(BTx,RI8)
{
	 int	ret;
	 int	op_num;
	uint16_t	*src;
	uint32_t	*src32;
	uint8_t	ofs;
	
	ret = RME_Int_GetModRM(State, NULL, &op_num, NULL);
	if(ret)	return ret;
	State->Decoder.IPOffset --;
	
	if( op_num < 4 ) {
		return RME_ERR_UNDEFOPCODE;
	}
	
	ret = RME_Int_ParseModRMX(State, NULL, &src, 0);
	if(ret)	return ret;
	src32 = (void*)src;
	
	READ_INSTR8(ofs);

	 uint32_t	val = (State->Decoder.bOverrideOperand ? *src32 : *src);
	 int	width = (State->Decoder.bOverrideOperand ? 32 : 16);

	ofs %= width;
	if( val & (1 << ofs) )
		State->Flags &= ~FLAG_CF;
	else
		State->Flags |= FLAG_CF;

	switch(op_num)
	{
	case 4:	// BT
		return 0;
	case 5:	// BTS
		val |= (1 << ofs);
		break;
	case 6:	// BTR
		val &= ~(1 << ofs);
		break;
	case 7:	// BTC
		val ^= (1 << ofs);
		break;
	}
	
	if( State->Decoder.bOverrideOperand )
		*src32 = val;
	else
		*src = val;
	return 0;
}

