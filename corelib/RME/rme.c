/*
 * Realmode Emulator Plugin
 * - By John Hodge (thePowersGang)
 *
 * This code is published under the FreeBSD licence
 * (See the file COPYING for details)
 *
 * ---
 * Core Emulator
 */
#define _RME_C_
#include <stdint.h>
//#include <stdlib.h>
//#include <stdio.h>

//#include <common.h>

typedef uint16_t	Uint16;
typedef int16_t	Sint16;
typedef int8_t	Sint8;

#include "rme.h"
#include "ops_alu.h"
#include "rme_internal.h"

// Settings
#define RME_DO_NULL_CHECK	1
#define	printf	printf	// Formatted print function

// === CONSTANTS ===
#define FLAG_DEFAULT	0x2

// === MACRO VOODOO ===
#define XCHG(a,b)	do{uint32_t t=(a);(a)=(b);(b)=(t);}while(0)

// === TYPES ===
typedef int (*tOpcodeFcn)(tRME_State *State, int Param);

// === PROTOTYPES ===
tRME_State	*RME_CreateState(void);
void	RME_DumpRegs(tRME_State *State);
 int	RME_CallInt(tRME_State *State, int Num);
 int	RME_Call(tRME_State *State);
static int	RME_Int_DoOpcode(tRME_State *State);

// === GLOBALS ===
#include "opcode_table.h"

// === CODE ===
/**
 * \brief Creates a blank RME State
 */
tRME_State *RME_CreateState(void)
{
	tRME_State	*state = calloc(sizeof(tRME_State), 1);

	if(state == NULL)	return NULL;

	// Initial Stack
	state->Flags = FLAG_DEFAULT;

	// Stub CS/IP
	state->CS = 0xF000;
	state->IP = 0xFFF0;

	return state;
}

/**
 * \brief Dump Realmode Registers
 */
void RME_DumpRegs(tRME_State *State)
{
	printf("\n");
	#if USE_SIZE_OVERRIDES == 1
	printf("EAX %08x  ECX %08x  EDX %08x  EBX %08x\n",
		State->AX.D, State->CX.D, State->DX.D, State->BX.D);
	printf("ESP %08x  EBP %08x  ESI %08x  EDI %08x\n",
		State->SP.D, State->BP.D, State->SI.D, State->DI.D);
	#else
	printf("AX %04x  CX %04x  DX %04x  BX %04x\n",
		State->AX.W, State->CX.W, State->DX.W, State->BX.W);
	printf("SP %04x  BP %04x  SI %04x  DI %04x\n",
		State->SP.W, State->BP.W, State->SI.W, State->DI.W);
	#endif
	printf("SS %04x  DS %04x  ES %04x\n",
		State->SS, State->DS, State->ES);
	printf("CS:IP = 0x%04x:%04x\n", State->CS, State->IP);
	printf("Flags = %04x", State->Flags);
	if(State->Flags & FLAG_OF)	printf(" OF");
	if(State->Flags & FLAG_DF)	printf(" DF");
	if(State->Flags & FLAG_IF)	printf(" IF");
	if(State->Flags & FLAG_TF)	printf(" TF");
	if(State->Flags & FLAG_SF)	printf(" SF");
	if(State->Flags & FLAG_ZF)	printf(" ZF");
	if(State->Flags & FLAG_AF)	printf(" AF");
	if(State->Flags & FLAG_PF)	printf(" PF");
	if(State->Flags & FLAG_CF)	printf(" CF");
	printf("\n");
}

int RME_int_CallInt(tRME_State *State, int Num)
{
	 int	ret;
	DEBUG_S("RM_Int: Calling Int 0x%x\n", Num);

	if(Num < 0 || Num > 0xFF) {
		ERROR_S("WARNING: %i is not a valid interrupt number", Num);
		return RME_ERR_INVAL;
	}

	PUSH(State->Flags);
	PUSH(State->CS);
	PUSH(State->IP);
	State->Flags &= ~(FLAG_IF|FLAG_TF);

	ret = RME_Int_Read16(State, 0, Num*4, &State->IP);
	if(ret)	return ret;
	ret = RME_Int_Read16(State, 0, Num*4+2, &State->CS);
	if(ret)	return ret;
	
	return 0;
}

/**
 * \brief Run Realmode interrupt
 */
int RME_CallInt(tRME_State *State, int Num)
{
	 int	ret;

	State->CS = RME_MAGIC_CS;	
	State->IP = RME_MAGIC_IP;

	ret = RME_int_CallInt(State, Num);
	if(ret)	return ret;

	return RME_Call(State);
}

/**
 * \brief Call a realmode function (a jump to a magic location is used as the return)
 */
int RME_Call(tRME_State *State)
{
	 int	ret;
	for(;;)
	{
		ret = RME_RunOne(State);
		if( ret == RME_ERR_FCNRET )
			return 0;
		if( ret != RME_ERR_OK )
			return ret;
	}
}

/*
 * \brief Run one instruction (or HLE operation)
 */
int RME_RunOne(tRME_State *State)
{
	#if DEBUG >= 2
	RME_DumpRegs(State);
	#endif
	
	if(State->IP == RME_MAGIC_IP && State->CS == RME_MAGIC_CS)
		return RME_ERR_FCNRET;
	
	if(State->CS == RME_HLE_CS && State->IP < 0x100) {
		// HLE Call
		if( State->HLECallbacks[State->IP] )
			State->HLECallbacks[State->IP](State, State->IP);
		// IRET
		caOperations[0xCF].Function(State, 0);
		return 0;
	}
	
	int ret = RME_Int_DoOpcode(State);
	switch(ret)
	{
	case RME_ERR_OK:
		break;
	case RME_ERR_DIVERR:
		ret = RME_int_CallInt(State, 0);
		if(ret)	return ret;
		break;
	// TODO: Handle #UD and others here
	default:
		return ret;
	}
	
	return 0;
}

/**
 * \brief Processes a single instruction
 */
int RME_Int_DoOpcode(tRME_State *State)
{
	 uint8_t	opcode;	// Current opcode and second byte
	 int	ret;	// Return value from functions
	uint16_t	startIP, startCS;	// Initial CPU location

	startIP = State->IP;
	startCS = State->CS;

	State->Decoder.OverrideSegment = -1;
	State->Decoder.RepeatType = 0;
	State->Decoder.bOverrideOperand = 0;
	State->Decoder.bOverrideAddress = 0;
	State->Decoder.bDontChangeIP = 0;
	State->Decoder.IPOffset = 0;
	State->InstrNum ++;

	DEBUG_S("(%8i) [0x%x] %04x:%04x", State->InstrNum, State->CS*16+State->IP, State->CS, State->IP);

	do
	{
		READ_INSTR8( opcode );
		// HACK 0xF1 is blank in the x86 opcode map, so it's used as uninit padding
		if( opcode == 0xF1 ) {
			ERROR_S(" Executing unset memory (opcode 0xF1) %04x:%04x",
				State->CS, State->IP);
			return 7;
		}
		if( caOperations[opcode].Function == NULL )
		{
			ERROR_S(" Unkown Opcode 0x%02x", opcode);
			return RME_ERR_UNDEFOPCODE;
		}
		if(caOperations[opcode].ModRMNames) {
			 int	rrr;
			RME_Int_GetModRM(State, NULL, &rrr, NULL);
			State->Decoder.IPOffset --;
			DEBUG_S(" %s", caOperations[opcode].ModRMNames[rrr]);
		}
		else {
			DEBUG_S(" %s", caOperations[opcode].Name);
		}
		DEBUG_S(" (%s)", caOperations[opcode].Type);
		ret = caOperations[opcode].Function(State, caOperations[opcode].Arg);
	} while( ret == RME_ERR_CONTINUE );	// RME_ERR_CONTINUE is returned by prefixes
	
	if( ret )
		return ret;

	// repType is cleared if it is used, so if it's not used, it's invalid
	if(State->Decoder.RepeatType)
	{
		DEBUG_S(" Prefix 0x%02x used with wrong opcode 0x%02x", State->Decoder.RepeatType, opcode);
		//return RME_ERR_UNDEFOPCODE;
	}

	if( !State->Decoder.bDontChangeIP )
		State->IP += State->Decoder.IPOffset;

	// HACK: Detect more than one instance of 00 00 in a row
	#if 1
	if( State->Decoder.IPOffset == 2 ) {
		uint8_t	byte1=0xFF, byte2=0xFF;
		ret = RME_Int_Read8(State, startCS, startIP+0, &byte1);
		if(ret)	return ret;
		ret = RME_Int_Read8(State, startCS, startIP+1, &byte2);
		if(ret)	return ret;
		
		if( byte1 == 0 && byte2 == 0 )
		{
			if( State->bWasLastOperationNull )
				return RME_ERR_BREAKPOINT;
			State->bWasLastOperationNull = 1;
		}
		else
			State->bWasLastOperationNull = 0;
	}
	else
		State->bWasLastOperationNull = 0;
	#endif

	#if DEBUG
	{
		uint16_t	i = startIP;
		uint8_t	byte;
		 int	j = State->Decoder.IPOffset;

		DEBUG_S("\t;");
		while(i < 0x10000 && j--) {
			ret = RME_Int_Read8(State, startCS, i, &byte);
			if(ret)	return ret;
			DEBUG_S(" %02x", byte);
			i ++;
		}
		if(j > 0)
		{
			while(j--) {
				ret = RME_Int_Read8(State, startCS, i, &byte);
				if(ret)	return ret;
				DEBUG_S(" %02x", byte);
				i ++;
			}
		}
	}
	#endif
	DEBUG_S("\n");
	return 0;
}

DEF_OPCODE_FCN(Ext,0F)
{
	uint8_t	extra;
	READ_INSTR8(extra);
	
	if( caOperations0F[extra].Function == NULL )
	{
		ERROR_S(" Unkown Opcode 0x0F 0x%02x", extra);
		return RME_ERR_UNDEFOPCODE;
	}
	
	DEBUG_S(" %s", caOperations0F[extra].Name);
	return caOperations0F[extra].Function(State, caOperations0F[extra].Arg);
}

DEF_OPCODE_FCN(Unary, M)	// INC/DEC r/m8
{
	const int	width = 8;
	 int	ret, op_num;
	uint8_t	*dest;
	
	ret = RME_Int_GetModRM(State, NULL, &op_num, NULL);
	if(ret)	return ret;
	State->Decoder.IPOffset --;
	
	switch(op_num)
	{
	case 0:	// INC
		DEBUG_S(" INC");
		ret = RME_Int_ParseModRM(State, NULL, &dest, 0);
		if(ret)	return ret;
		{ALU_OPCODE_INC_CODE}
		break;
	case 1:	// DEC
		DEBUG_S(" DEC");
		ret = RME_Int_ParseModRM(State, NULL, &dest, 0);
		if(ret)	return ret;
		{ALU_OPCODE_DEC_CODE}
		break;
	default:
		ERROR_S(" - Unary M /%i unimplemented\n", op_num);
		return RME_ERR_UNDEFOPCODE;
	}
	
	return 0;
}

DEF_OPCODE_FCN(Unary, MX)	// INC/DEC r/m16, CALL/JMP/PUSH r/m16
{
	 int	ret, op_num, mod, mmm;
	
	ret = RME_Int_GetModRM(State, &mod, &op_num, &mmm);
	if(ret)	return ret;
	State->Decoder.IPOffset --;
	
	if( State->Decoder.bOverrideOperand )
	{
		uint32_t	*dest;
		const int	width = 32;	
		switch( op_num )
		{
		case 0:
			DEBUG_S(" INC");
			ret = RME_Int_ParseModRMX(State, NULL, (void*)&dest, 0);
			if(ret)	return ret;
			{ALU_OPCODE_INC_CODE}
			SET_COMM_FLAGS(State, *dest, width);
			break;
		case 1:
			DEBUG_S(" DEC");
			ret = RME_Int_ParseModRMX(State, NULL, (void*)&dest, 0);
			if(ret)	return ret;
			{ALU_OPCODE_DEC_CODE}
			SET_COMM_FLAGS(State, *dest, width);
			break;
		case 6:
			DEBUG_S(" PUSH");
			ret = RME_Int_ParseModRMX(State, NULL, (void*)&dest, 0);	//Get Register Value
			if(ret)	return ret;
			PUSH( *dest );
			break;
		default:
			ERROR_S(" - Unary MX (32) /%i unimplemented\n", op_num);
			return RME_ERR_UNDEFOPCODE;
		}
	}
	else
	{
		uint16_t	*dest;
		const int	width = 16;	
		switch( op_num )
		{
		case 0:
			DEBUG_S(" INC");
			ret = RME_Int_ParseModRMX(State, NULL, &dest, 0);
			if(ret)	return ret;
			{ALU_OPCODE_INC_CODE}
			SET_COMM_FLAGS(State, *dest, width);
			break;
		case 1:
			DEBUG_S(" DEC");
			ret = RME_Int_ParseModRMX(State, NULL, &dest, 0);
			if(ret)	return ret;
			{ALU_OPCODE_DEC_CODE}
			SET_COMM_FLAGS(State, *dest, width);
			break;
		case 2:	// Call Near Indirect
			DEBUG_S(" CALL (NI)");
			ret = RME_Int_ParseModRMX(State, NULL, &dest, 0);
			if(ret)	return ret;
			PUSH(State->IP + State->Decoder.IPOffset);
			State->IP = *dest;
			State->Decoder.bDontChangeIP = 1;
			break;
		case 3:	// Call Far Indirect
			DEBUG_S(" CALL (FI)");
			if( mod == 3 )	return RME_ERR_UNDEFOPCODE;	// TODO: Check this
			ret = RME_Int_ParseModRMX(State, NULL, &dest, 0);
			if(ret)	return ret;
			PUSH(State->CS);
			PUSH(State->IP + State->Decoder.IPOffset);
			State->IP = dest[0];
			State->CS = dest[1];	// NOTE: Possible edge case on segment boundary
			State->Decoder.bDontChangeIP = 1;
			break;
		case 4:	// Jump Near Indirect
			DEBUG_S(" JMP (NI)");
			ret = RME_Int_ParseModRMX(State, NULL, &dest, 0);
			if(ret)	return ret;
			State->IP = *dest;
			State->Decoder.bDontChangeIP = 1;
			break;
		case 5:	// Jump Far Indirect
			DEBUG_S(" JMP (FI)");
			if( mod == 3 )	return RME_ERR_UNDEFOPCODE;	// TODO: Check this
			ret = RME_Int_ParseModRMX(State, NULL, &dest, 0);
			if(ret)	return ret;
			State->IP = dest[0];
			State->CS = dest[1];	// NOTE: Possible edge case on segment boundary
			State->Decoder.bDontChangeIP = 1;
			break;
		case 6:
			DEBUG_S(" PUSH");
			ret = RME_Int_ParseModRMX(State, NULL, &dest, 0);	//Get Register Value
			if(ret)	return ret;
			PUSH( *dest );
			break;
			
		default:
			ERROR_S(" - Unary MX (16) /%i unimplemented\n", op_num);
			return RME_ERR_UNDEFOPCODE;
		}
	}
	
	return 0;
}

// =====================================================================
//                 ModR/M and SIB Addressing Helpers
// =====================================================================
/**
 * \brief Performs a memory addressing function
 * \param State	Emulator State
 * \param mmm	Function ID (mmm field from ModR/M byte)
 * \param disp	Displacement
 * \param ptr	Destination for final pointer
 */
static int DoFunc(tRME_State *State, int mmm, int16_t disp, uint16_t *Segment, uint32_t *Offset)
{
	uint32_t	addr;
	uint16_t	seg;

	switch(mmm){
	case 2:	case 3:	case 6:
		seg = SREG_SS;
		break;
	default:
		seg = SREG_DS;
		break;
	}

	if(State->Decoder.OverrideSegment != -1)
		seg = State->Decoder.OverrideSegment;

	seg = *Seg(State, seg);

	DEBUG_S(":[");
	switch(mmm & 7)
	{
	case 0:
		DEBUG_S("BX+SI");
		addr = State->BX.W + State->SI.W;
		break;
	case 1:
		DEBUG_S("BX+DI");
		addr = State->BX.W + State->DI.W;
		break;
	case 2:
		DEBUG_S("BP+SI");
		addr = State->BP.W + State->SI.W;
		break;
	case 3:
		DEBUG_S("BP+DI");
		addr = State->BP.W + State->DI.W;
		break;
	case 4:
		DEBUG_S("SI");
		addr = State->SI.W;
		break;
	case 5:
		DEBUG_S("DI");
		addr = State->DI.W;
		break;
	case 6:
		if( mmm & 8 ) {
			READ_INSTR16( disp );
			DEBUG_S("0x%04x", disp);
			addr = disp;
		}
		else {
			DEBUG_S("BP");
			addr = State->BP.W;
		}
		break;
	case 7:
		DEBUG_S("BX");
		addr = State->BX.W;
		break;
	default:
		ERROR_S("Unknown mmm value passed to DoFunc (%i)", mmm);
		return RME_ERR_BUG;
	}
	if( !(mmm & 8) ) {
		DEBUG_S("+0x%x", disp);
		addr += disp;
	}
	DEBUG_S("]");
	*Segment = seg;
	*Offset = addr;
	return 0;
}

/**
 * \brief Performs a memory addressing function (32-bit, with SIB if nessesary)
 * \param State	Emulator State
 * \param mmm	Function ID (mmm field from ModR/M byte)
 * \param disp	Displacement
 * \param ptr	Destination for final pointer
 */
static int DoFunc32(tRME_State *State, int mmm, int32_t disp, uint16_t *Segment, uint32_t *Offset)
{
	uint32_t	addr;
	uint16_t	seg;
	uint8_t	sib;

	switch(mmm){
	case 2:	case 3:	case 6:
		seg = SREG_SS;
		break;
	default:
		seg = SREG_DS;
		break;
	}

	if(State->Decoder.OverrideSegment != -1)
		seg = State->Decoder.OverrideSegment;

	seg = *Seg(State, seg);

	DEBUG_S(":[");
	switch(mmm & 7)
	{
	case 0:
		DEBUG_S("EAX");
		addr = State->AX.D;
		break;
	case 1:
		DEBUG_S("ECX");
		addr = State->CX.D;
		break;
	case 2:
		DEBUG_S("EDX");
		addr = State->DX.D;
		break;
	case 3:
		DEBUG_S("EBX");
		addr = State->BX.D;
		break;
	case 4:	// SIB (uses ESP's slot)
		READ_INSTR8(sib);
		addr = 0;
		// Index Reg
		switch( (sib >> 3) & 7 )
		{
		case 0:	DEBUG_S("EAX");	addr = State->AX.D;	break;
		case 1:	DEBUG_S("ECX");	addr = State->CX.D;	break;
		case 2:	DEBUG_S("EDX");	addr = State->DX.D;	break;
		case 3:	DEBUG_S("EBX");	addr = State->BX.D;	break;
		case 4:	DEBUG_S("EIZ");	addr = 0;	break;
		case 5:	DEBUG_S("EBP");	addr = State->BP.D;	break;
		case 6:	DEBUG_S("ESI");	addr = State->SI.D;	break;
		case 7:	DEBUG_S("EDI");	addr = State->DI.D;	break;
		}
		// Scale
		DEBUG_S("*%i", 1 << (sib >> 6));
		addr <<= (sib >> 6);
		// Base
		switch( sib & 7 )
		{
		case 0:	DEBUG_S("+EAX");	addr += State->AX.D;	break;
		case 1:	DEBUG_S("+ECX");	addr += State->CX.D;	break;
		case 2:	DEBUG_S("+EDX");	addr += State->DX.D;	break;
		case 3:	DEBUG_S("+EBX");	addr += State->BX.D;	break;
		case 4:	DEBUG_S("+ESP");	addr += State->SP.D;	break;
		case 5:	// SPECIAL CASE
			if( mmm & 8 ) {
				READ_INSTR32(disp);
			}
			else
			{
				DEBUG_S("+EBP");
				addr += State->BP.D;
			}
			break;
		case 6:	DEBUG_S("+ESI");	addr += State->SI.D;	break;
		case 7:	DEBUG_S("+EDI");	addr += State->DI.D;	break;
		}
		break;
	case 5:
		if( mmm & 8 )
		{
			// R/M == 5 and Mod == 0, disp32
			READ_INSTR32( addr );
			DEBUG_S("0x%x", addr);
		}
		else
		{
			DEBUG_S("EBP");
			addr = State->BP.D;
		}
		break;
	case 6:
		DEBUG_S("ESI");
		addr = State->SI.D;
		break;
	case 7:
		DEBUG_S("EDI");
		addr = State->DI.D;
		break;
	default:
		ERROR_S("Unknown mmm value passed to DoFunc32 (%i)", mmm);
		return RME_ERR_BUG;
	}
	if( !(mmm & 8) ) {
		DEBUG_S("+0x%x", disp);
		addr += disp;
	}
	DEBUG_S("]");
	*Segment = seg;
	*Offset = addr;
	return 0;
}

inline int RME_Int_GetMMM(tRME_State *State, int mod, int mmm, uint16_t *Segment, uint32_t *Offset)
{
	uint16_t	ofs;
	 int	ret;
	
	if( State->Decoder.bOverrideAddress )
	{
		switch(mod)
		{
		case 0:	// No Offset
			ret = DoFunc32( State, mmm | 8, 0, Segment, Offset );
			if(ret)	return ret;
			break;
		case 1:	// disp8
			READ_INSTR8S( ofs );
			ret = DoFunc32( State, mmm, ofs, Segment, Offset);
			if(ret)	return ret;
			break;
		case 2:	// disp32
			READ_INSTR32( ofs );
			ret = DoFunc32( State, mmm, ofs, Segment, Offset );
			if(ret)	return ret;
			break;
		case 3:
			ERROR_S("mod=3 passed to RME_Int_GetMMM");
			return RME_ERR_BUG;
		default:
			ERROR_S("Unknown mod value passed to RME_Int_GetMMM (%i)", mod);
			return RME_ERR_BUG;
		}
	}
	else
	{
		switch(mod)
		{
		case 0:	// No Offset
			ret = DoFunc( State, mmm | 8, 0, Segment, Offset );
			if(ret)	return ret;
			break;
		case 1:	// 8 Bit
			READ_INSTR8S( ofs );
			ret = DoFunc( State, mmm, ofs, Segment, Offset);
			if(ret)	return ret;
			break;
		case 2:	// 16 Bit
			READ_INSTR16( ofs );
			ret = DoFunc( State, mmm, ofs, Segment, Offset );
			if(ret)	return ret;
			break;
		case 3:
			ERROR_S("mod=3 passed to RME_Int_GetMMM");
			return RME_ERR_BUG;
		default:
			ERROR_S("Unknown mod value passed to RME_Int_GetMMM (%i)", mod);
			return RME_ERR_BUG;
		}
	}
	return 0;
}

/**
 * \brief Parses the ModR/M byte as a 8-bit value
 * \param State	Emulator State
 * \param to	R field destination (ignored if NULL)
 * \param from	M field destination (ignored if NULL)
 */
int RME_Int_ParseModRM(tRME_State *State, uint8_t **to, uint8_t **from, int bReverse)
{
	 int	ret;
	 int	mod, rrr, mmm;

	RME_Int_GetModRM(State, &mod, &rrr, &mmm);
	
	#if DEBUG
	if(!bReverse) {
	#endif
		if(to) *to = RegB( State, rrr );
	#if DEBUG
	}
	#endif
	if(from)
	{
		if( mod == 3 )
			*from = RegB( State, mmm );
		else
		{
			uint16_t	segment;
			uint32_t	offset;
			ret = RME_Int_GetMMM( State, mod, mmm, &segment, &offset );
			if(ret)	return ret;
			ret = RME_Int_GetPtr(State, segment, offset, (void**)from);
			if(ret)	return ret;
		}
	}
	#if DEBUG
	if(bReverse) {
		if(to) *to = RegB( State, rrr );
	}
	#endif
	return 0;
}

/**
 * \brief Parses the ModR/M byte as a 16-bit value
 * \param State	Emulator State
 * \param to	R field destination (ignored if NULL)
 * \param from	M field destination (ignored if NULL)
 */
int RME_Int_ParseModRMX(tRME_State *State, uint16_t **reg, uint16_t **mem, int bReverse)
{
	 int	ret;
	 int	mod, rrr, mmm;

	RME_Int_GetModRM(State, &mod, &rrr, &mmm);
	
	#if DEBUG
	if(!bReverse) {
	#endif
		if(reg) *reg = RegW( State, rrr );
	#if DEBUG
	}
	#endif
	if(mem)
	{
		if( mod == 3 )
			*mem = RegW( State, mmm );
		else
		{
			uint16_t	segment;
			uint32_t	offset;
			ret = RME_Int_GetMMM( State, mod, mmm, &segment, &offset );
			if(ret)	return ret;
			if( (segment * 0x10 + offset) % RME_BLOCK_SIZE == RME_BLOCK_SIZE-1 ) {
				ERROR_S("%x:%x Word read across boundary (0x%x)",
					State->CS, State->IP, segment * 0x10 + offset);
				return RME_ERR_BADMEM;
			}
			ret = RME_Int_GetPtr(State, segment, offset, (void**)mem);
			if(ret)	return ret;
		}
	}
	#if DEBUG
	if(bReverse) {
		if(reg) *reg = RegW( State, rrr );
	}
	#endif
	return 0;
}
