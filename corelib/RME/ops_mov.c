/*
 * Realmode Emulator
 * - MOV Family of operations (includes XCHG)
 * 
 */
#include "rme.h"
#include "rme_internal.h"
#include "opcode_prototypes.h"

#define XCHG(a,b)	do{uint32_t t=(a);(a)=(b);(b)=(t);}while(0)

// === CODE ===
// r/m = r
DEF_OPCODE_FCN(MOV, MR)
{
	 int	ret;
	uint8_t	*dest, *src;
	ret = RME_Int_ParseModRM(State, &src, &dest, 1);
	if(ret)	return ret;
	
	*dest = *src;
	
	return 0;
}
DEF_OPCODE_FCN(MOV, MRX)
{
	 int	ret;
	void	*destPtr, *srcPtr;
	ret = RME_Int_ParseModRMX(State, (uint16_t**)&srcPtr, (uint16_t**)&destPtr, 1);
	if(ret)	return ret;
	
	if( State->Decoder.bOverrideOperand )
	{
		uint32_t	*dest=destPtr, *src=srcPtr;
		*dest = *src;
	}
	else
	{
		uint16_t	*dest=destPtr, *src=srcPtr;
		*dest = *src;
	}
	
	return 0;
}

// r = r/m
DEF_OPCODE_FCN(MOV, RM)
{
	 int	ret;
	uint8_t	*dest, *src;
	ret = RME_Int_ParseModRM(State, &dest, &src, 0);
	if(ret)	return ret;
	
	*dest = *src;
	
	return 0;
}
DEF_OPCODE_FCN(MOV, RMX)
{
	 int	ret;
	void	*destPtr, *srcPtr;
	ret = RME_Int_ParseModRMX(State, (uint16_t**)&destPtr, (uint16_t**)&srcPtr, 0);
	if(ret)	return ret;
	
	if( State->Decoder.bOverrideOperand )
	{
		uint32_t	*dest=destPtr, *src=srcPtr;
		*dest = *src;
	}
	else
	{
		uint16_t	*dest=destPtr, *src=srcPtr;
		*dest = *src;
	}
	
	return 0;
}
// r/m = I
DEF_OPCODE_FCN(MOV, MI )
{
	 int	ret;
	uint8_t	val, *dest;
	ret = RME_Int_ParseModRM(State, NULL, &dest, 0);
	if(ret)	return ret;
	READ_INSTR8( val );
	DEBUG_S(" 0x%02x", val);
	*dest = val;
	return 0;
}
DEF_OPCODE_FCN(MOV, MIX)
{
	 int	ret;
	union {
		uint32_t	*D;
		uint16_t	*W;
	}	dest;
	ret = RME_Int_ParseModRMX(State, NULL, &dest.W, 0);
	if(ret)	return ret;
	
	if(State->Decoder.bOverrideOperand)
	{
		uint32_t	val;
		READ_INSTR32( val );
		DEBUG_S(" 0x%08x", val);
		*dest.D = val;
	}
	else
	{
		uint16_t	val;
		READ_INSTR16( val );
		DEBUG_S(" 0x%04x", val);
		*dest.W = val;
	}
	return 0;
}

// A := [imm16/32]
DEF_OPCODE_FCN(MOV, AMo)
{
	 int	ret;
	uint16_t	seg;
	uint32_t	ofs;
	
	DEBUG_S(" AL");
	
	seg = *GET_SEGMENT(State, SREG_DS);
	if( State->Decoder.bOverrideAddress ) {
		READ_INSTR32( ofs );
		DEBUG_S(":[0x%x]", ofs);
	}
	else {
		READ_INSTR16( ofs );
		DEBUG_S(":[0x%x]", ofs);
	}
	
	ret = RME_Int_Read8(State, seg, ofs, &State->AX.B.L);
	if(ret)	return ret;
	
	return 0;
}
DEF_OPCODE_FCN(MOV, AMoX)
{
	 int	ret;
	uint32_t	ofs;
	
	DEBUG_S(" %s", (State->Decoder.bOverrideOperand?"EAX":"AX"));
	
	uint16_t seg = *GET_SEGMENT(State, SREG_DS);
	if( State->Decoder.bOverrideAddress ) {
		READ_INSTR32( ofs );
		DEBUG_S(":[0x%x]", ofs);
	}
	else {
		READ_INSTR16( ofs );
		DEBUG_S(":[0x%x]", ofs);
	}
	
	if( State->Decoder.bOverrideOperand )
		ret = RME_Int_Read32(State, seg, ofs, &State->AX.D);
	else
		ret = RME_Int_Read16(State, seg, ofs, &State->AX.W);
		
	if(ret)	return ret;
	
	return 0;
}

// [imm16/32] := A
DEF_OPCODE_FCN(MOV, MoA)
{
	 int	ret;
	uint16_t	seg;
	uint32_t	ofs;
	
	seg = *GET_SEGMENT(State, SREG_DS);
	if( State->Decoder.bOverrideAddress ) {
		READ_INSTR32( ofs );
		DEBUG_S(":[0x%x]", ofs);
	}
	else {
		READ_INSTR16( ofs );
		DEBUG_S(":[0x%x]", ofs);
	}
	DEBUG_S(" AL");
	ret = RME_Int_Write8(State, seg, ofs, State->AX.B.L);
	if(ret)	return ret;
	
	return 0;
}
DEF_OPCODE_FCN(MOV, MoAX)
{
	 int	ret;
	uint16_t	seg;
	uint32_t	ofs;
	
	seg = *GET_SEGMENT(State, SREG_DS);
	if( State->Decoder.bOverrideAddress ) {
		READ_INSTR32( ofs );
		DEBUG_S(":[0x%x]", ofs);
	}
	else {
		READ_INSTR16( ofs );
		DEBUG_S(":[0x%x]", ofs);
	}
	
	DEBUG_S(" %s", (State->Decoder.bOverrideOperand?"EAX":"AX"));
	
	if( State->Decoder.bOverrideOperand )
		ret = RME_Int_Write32(State, seg, ofs, State->AX.D);
	else
		ret = RME_Int_Write16(State, seg, ofs, State->AX.W);
		
	if(ret)	return ret;
	
	return 0;
}

// r = sr
DEF_OPCODE_FCN(MOV, RS)
{
	 int	ret;
	union {
		uint32_t	*D;
		uint16_t	*W;
	}	dest, src;
	uint8_t	byte2;
	
	if( State->Decoder.bOverrideOperand )
		return RME_ERR_UNDEFOPCODE;
	
	READ_INSTR8( byte2 );	State->Decoder.IPOffset --;
	
	src.W = Seg(State, (byte2>>3)&7);
	
	ret = RME_Int_ParseModRMX(State, NULL, &dest.W, 0);
	if(ret)	return ret;
	
	*dest.W = *src.W;
	
	return 0;
}
// sr = r
DEF_OPCODE_FCN(MOV, SR )
{
	 int	ret;
	union {
		uint32_t	*D;
		uint16_t	*W;
	}	dest, src;
	uint8_t	byte2;
	
	if( State->Decoder.bOverrideOperand )
		return RME_ERR_UNDEFOPCODE;
	
	READ_INSTR8( byte2 );	State->Decoder.IPOffset --;
	
	dest.W = Seg(State, (byte2>>3)&7);
	
	ret = RME_Int_ParseModRMX(State, NULL, &src.W, 0);
	if(ret)	return ret;
	
	*dest.W = *src.W;
	
	return 0;
}
// r = I
DEF_OPCODE_FCN(MOV, RegB)
{
	uint8_t	val, *dest;
	READ_INSTR8(val);
	dest = RegB(State, Param);
	DEBUG_S(" 0x%02x", val);
	*dest = val;
	return 0;
}
DEF_OPCODE_FCN(MOV, Reg)
{
	if( State->Decoder.bOverrideOperand )
	{
		uint32_t	val, *dest;
		READ_INSTR32(val);
		dest = (void*)RegW(State, Param);
		DEBUG_S(" 0x%08x", val);
		*dest = val;
	}
	else
	{
		uint16_t	val, *dest;
		READ_INSTR16(val);
		dest = RegW(State, Param);
		DEBUG_S(" 0x%04x", val);
		*dest = val;
	}
	return 0;
}

// Move and Zero Extend
DEF_OPCODE_FCN(MOV,Z)
{
	uint8_t	*src;
	void	*dest;
	 int	ret, rrr;
	
	ret = RME_Int_GetModRM(State, NULL, &rrr, NULL);	State->Decoder.IPOffset --;
	if( ret )	return ret;
	
	dest = RegW(State, rrr);
	ret = RME_Int_ParseModRM(State, NULL, &src, 0);
	if( ret )	return ret;
	
	if( State->Decoder.bOverrideOperand )
		*(uint32_t*)dest = *src;
	else
		*(uint16_t*)dest = *src;
	
	return 0;
}
DEF_OPCODE_FCN(MOV,ZX)
{
	uint16_t	*src;
	void	*dest;
	 int	ret, rrr;
	
	ret = RME_Int_GetModRM(State, NULL, &rrr, NULL);	State->Decoder.IPOffset --;
	if( ret )	return ret;
	
	dest = RegW(State, rrr);
	ret = RME_Int_ParseModRMX(State, NULL, &src, 0);
	if( ret )	return ret;
	
	if( State->Decoder.bOverrideOperand )
		*(uint32_t*)dest = *src;
	else
		*(uint16_t*)dest = *src;
	
	return 0;
}

// Exchange Family
DEF_OPCODE_FCN(XCHG, RM)
{
	 int	ret;
	uint8_t	*dest, *src;
	ret = RME_Int_ParseModRM(State, &dest, &src, 0);
	if(ret)	return ret;
	XCHG(*dest, *src);
	return 0;
}
DEF_OPCODE_FCN(XCHG, RMX)
{
	 int	ret;
	if( State->Decoder.bOverrideOperand )
	{
		uint32_t	*dest, *src;
		ret = RME_Int_ParseModRMX(State, (void*)&dest, (void*)&src, 0);
		if(ret)	return ret;
		XCHG(*dest, *src);
	}
	else
	{
		uint16_t	*dest, *src;
		ret = RME_Int_ParseModRMX(State, &dest, &src, 0);
		if(ret)	return ret;
		XCHG(*dest, *src);
	}
	return 0;
}
DEF_OPCODE_FCN(XCHG, Reg)	// A with Reg
{
	union {
		uint16_t	*W;
		uint32_t	*D;
	}	src;
	if( Param == 0 ) {
		DEBUG_S(" - NOP");
		return 0;
	}
	
	DEBUG_S(" %s", (State->Decoder.bOverrideOperand?"EAX":"AX"));
	
	src.W = RegW(State, Param);
	if(State->Decoder.bOverrideOperand)
		XCHG(State->AX.D, *src.D);
	else
		XCHG(State->AX.W, *src.W);
	return 0;
}
