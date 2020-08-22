/*
 * Realmode Emulator
 * - Opcode Prefixes
 */
#include "rme.h"
#include "rme_internal.h"
#include "opcode_prototypes.h"

// === CODE ===
DEF_OPCODE_FCN(Ovr, Seg)
{
	if( State->Decoder.OverrideSegment != -1 )
		return RME_ERR_UNDEFOPCODE;
	Seg(State, Param);	// Printing
	State->Decoder.OverrideSegment = Param;
	return RME_ERR_CONTINUE;	// Not an error
}

DEF_OPCODE_FCN(Ovr, OpSize)
{
	if( State->Decoder.bOverrideOperand )
		return RME_ERR_UNDEFOPCODE;
	DEBUG_S(" <OPER>");
	State->Decoder.bOverrideOperand = 1;
	return RME_ERR_CONTINUE;	// Not an error
}
DEF_OPCODE_FCN(Ovr, AddrSize)
{
	if( State->Decoder.bOverrideAddress )
		return RME_ERR_UNDEFOPCODE;
	DEBUG_S(" <ADDR>");
	State->Decoder.bOverrideAddress = 1;
	return RME_ERR_CONTINUE;	// Not an error
}

DEF_OPCODE_FCN(Prefix, REP)
{
	if( State->Decoder.RepeatType != 0 )
		return RME_ERR_UNDEFOPCODE;
	State->Decoder.RepeatType = REP;
	return RME_ERR_CONTINUE;
}
DEF_OPCODE_FCN(Prefix, REPNZ)
{
	if( State->Decoder.RepeatType != 0 )
		return RME_ERR_UNDEFOPCODE;
	State->Decoder.RepeatType = REPNZ;
	return RME_ERR_CONTINUE;
}
