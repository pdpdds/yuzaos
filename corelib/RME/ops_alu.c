/*
 * Realmode Emulator
 * - ALU Operations
 * 
 */
#include "rme.h"
#include "rme_internal.h"
#include "opcode_prototypes.h"
#include "ops_alu.h"

#define _EXT_OP_SD(srcPtr, destPtr, __code)	if(State->Decoder.bOverrideOperand) { \
		const int	width = 32; \
		uint32_t	*src = srcPtr, *dest = destPtr; \
		{__code} \
	} else { \
		const int	width = 16; \
		uint16_t	*src = srcPtr, *dest = destPtr; \
		{__code} \
	}
 
#define CREATE_ALU_OPCODE_FCN_RM(__name, __code) DEF_OPCODE_FCN(__name,RM) {\
	 int	ret;\
	const int	width=8; \
	uint8_t	v, *dest=0, *src=0; \
	ret = RME_Int_ParseModRM(State, &dest, &src, 0); \
	if(ret)	return ret; \
	__code \
	SET_COMM_FLAGS(State,v,width); \
	if(dest) *dest = v; \
	return 0; \
}
#define CREATE_ALU_OPCODE_FCN_RMX(__name, __code) DEF_OPCODE_FCN(__name,RMX) {\
	 int	ret; \
	void	*destPtr=0, *srcPtr=0; \
	ret = RME_Int_ParseModRMX(State, (void*)&destPtr, (void*)&srcPtr, 0); \
	if(ret)	return ret; \
	if(State->Decoder.bOverrideOperand) { \
		uint32_t v,*dest=destPtr, *src=srcPtr; \
		 int	width=32;\
		__code \
		SET_COMM_FLAGS(State,v,width); \
		if(dest) *dest = v; \
	} else { \
		uint16_t v,*dest=destPtr, *src=srcPtr; \
		 int	width=16;\
		__code \
		SET_COMM_FLAGS(State,v,width); \
		if(dest) *dest = v; \
	} \
	return 0; \
}
#define CREATE_ALU_OPCODE_FCN_MR(__name, __code) DEF_OPCODE_FCN(__name,MR) {\
	 int	ret;\
	const int	width=8; \
	uint8_t	v, *dest=0, *src=0; \
	ret = RME_Int_ParseModRM(State, &src, &dest, 1); \
	if(ret)	return ret; \
	__code \
	SET_COMM_FLAGS(State,v,width); \
	if(dest) *dest = v;\
	return 0; \
}
#define CREATE_ALU_OPCODE_FCN_MRX(__name, __code) DEF_OPCODE_FCN(__name,MRX) {\
	 int	ret; \
	void	*destPtr=0, *srcPtr=0; \
	ret = RME_Int_ParseModRMX(State, (void*)&srcPtr, (void*)&destPtr, 1); \
	if(ret)	return ret; \
	if(State->Decoder.bOverrideOperand) { \
		uint32_t v, *dest=destPtr, *src=srcPtr; \
		 int	width=32;\
		__code \
		SET_COMM_FLAGS(State,v,width); \
		if(dest) *dest = v; \
	} else { \
		uint16_t v, *dest=destPtr, *src=srcPtr; \
		 int	width=16;\
		__code \
		SET_COMM_FLAGS(State,v,width); \
		if(dest) *dest = v;\
	} \
	return 0; \
}
#define CREATE_ALU_OPCODE_FCN_AI(__name, __code) DEF_OPCODE_FCN(__name,AI) {\
	const int	width=8; \
	uint8_t srcData, v; \
	uint8_t	*dest=&State->AX.B.L, *src=&srcData; \
	READ_INSTR8( srcData ); \
	DEBUG_S(" AL 0x%02x", *src); \
	__code \
	SET_COMM_FLAGS(State,v,width); \
	if(dest) *dest = v;\
	return 0; \
}
#define CREATE_ALU_OPCODE_FCN_AIX(__name, __code) DEF_OPCODE_FCN(__name,AIX) {\
	if(State->Decoder.bOverrideOperand) { \
		uint32_t srcData, v; \
		uint32_t *dest=&State->AX.D, *src=&srcData; \
		 int	width=32;\
		READ_INSTR32( *src ); \
		DEBUG_S(" EAX 0x%08x", *src); \
		__code \
		SET_COMM_FLAGS(State,v,width); \
		if(dest) *dest=v;\
	} else { \
		uint16_t srcData, v; \
		uint16_t *dest=&State->AX.W, *src=&srcData; \
		 int	width=16;\
		READ_INSTR16( *src ); \
		DEBUG_S(" AX 0x%04x", *src); \
		__code \
		SET_COMM_FLAGS(State,v,width); \
		if(dest) *dest=v;\
	} \
	return 0; \
}

#define CREATE_ALU_OPCODE_FCN(__name, __code) \
	CREATE_ALU_OPCODE_FCN_RM(__name, __code) \
	CREATE_ALU_OPCODE_FCN_RMX(__name, __code) \
	CREATE_ALU_OPCODE_FCN_MR(__name, __code) \
	CREATE_ALU_OPCODE_FCN_MRX(__name, __code) \
	CREATE_ALU_OPCODE_FCN_AI(__name, __code) \
	CREATE_ALU_OPCODE_FCN_AIX(__name, __code)

// === CODE ===
CREATE_ALU_OPCODE_FCN(ADD, ALU_OPCODE_ADD_CODE)
CREATE_ALU_OPCODE_FCN(OR , ALU_OPCODE_OR_CODE)
CREATE_ALU_OPCODE_FCN(ADC, ALU_OPCODE_ADC_CODE)
CREATE_ALU_OPCODE_FCN(SBB, ALU_OPCODE_SBB_CODE)
CREATE_ALU_OPCODE_FCN(AND, ALU_OPCODE_AND_CODE)
CREATE_ALU_OPCODE_FCN(SUB, ALU_OPCODE_SUB_CODE)
CREATE_ALU_OPCODE_FCN(XOR, ALU_OPCODE_XOR_CODE)
CREATE_ALU_OPCODE_FCN(CMP, ALU_OPCODE_CMP_CODE)

CREATE_ALU_OPCODE_FCN_MR(TEST, ALU_OPCODE_TEST_CODE)
CREATE_ALU_OPCODE_FCN_MRX(TEST, ALU_OPCODE_TEST_CODE)
CREATE_ALU_OPCODE_FCN_AI(TEST, ALU_OPCODE_TEST_CODE)
CREATE_ALU_OPCODE_FCN_AIX(TEST, ALU_OPCODE_TEST_CODE)

#define ALU_SELECT_OPERATION() do { switch( op_num ) { \
	case 0: do { ALU_OPCODE_ADD_CODE } while(0);	break; \
	case 1: do { ALU_OPCODE_OR_CODE  } while(0);	break; \
	case 2: do { ALU_OPCODE_ADC_CODE } while(0);	break; \
	case 3: do { ALU_OPCODE_SBB_CODE } while(0);	break; \
	case 4: do { ALU_OPCODE_AND_CODE } while(0);	break; \
	case 5: do { ALU_OPCODE_SUB_CODE } while(0);	break; \
	case 6: do { ALU_OPCODE_XOR_CODE } while(0);	break; \
	case 7: do { ALU_OPCODE_CMP_CODE } while(0);	break; \
	} } while(0)
#define SHIFT_SELECT_OPERATION()	do{ switch( op_num ) {\
	case 0: { ALU_OPCODE_ROL_CODE }	break; \
	case 1: { ALU_OPCODE_ROR_CODE }	break; \
	case 2:	{ ALU_OPCODE_RCL_CODE }	break; \
	case 3:	{ ALU_OPCODE_RCR_CODE }	break; \
	case 4:	{ ALU_OPCODE_SHL_CODE }	break; \
	case 5:	{ ALU_OPCODE_SHR_CODE }	break; \
	case 6:	{ ALU_OPCODE_SAL_CODE }	break; \
	case 7:	{ ALU_OPCODE_SAR_CODE }	break; \
	default: ERROR_S(" - Shift Undef %i\n", op_num); return RME_ERR_UNDEFOPCODE;\
	} }while(0)
#define _READIMM() do { switch(width) {\
	case 8: 	READ_INSTR8(v); DEBUG_S(" 0x%02x", v); break;\
	case 16:	READ_INSTR16(v);DEBUG_S(" 0x%04x", v); break;\
	case 32:	READ_INSTR32(v);DEBUG_S(" 0x%08x", v); break;\
	default: ERROR_S(" - _READIMM size %i unk", width); return RME_ERR_UNDEFOPCODE;\
} } while(0)
#define MISC_SELECT_OPERATION() do { switch( op_num ) { \
	case 0:	{dest=&v; _READIMM(); ALU_OPCODE_TEST_CODE SET_COMM_FLAGS(State, v, width);} break;\
	case 1:	DEBUG_S(" Misc /1 UNDEF");	return RME_ERR_UNDEFOPCODE; \
	case 2: {ALU_OPCODE_NOT_CODE} break; \
	case 3: {ALU_OPCODE_NEG_CODE} break; \
	case 4:	{ALU_OPCODE_MUL_CODE} break; \
	case 5:	{ALU_OPCODE_IMUL_CODE} break; \
	case 6:	{ALU_OPCODE_DIV_CODE} break; \
	case 7:	{ALU_OPCODE_IDIV_CODE} break; \
	default: ERROR_S(" - Misc %i error\n", op_num); return RME_ERR_UNDEFOPCODE; \
} } while(0)

DEF_OPCODE_FCN(Arith, MI)
{
	 int	ret;
	const int width = 8;
	uint8_t	srcData, v;
	uint8_t *dest, *src = &srcData;
	 int	op_num;
	
	// Get rrr term from ModRM byte
	ret = RME_Int_GetModRM(State, NULL, &op_num, NULL);
	if(ret)	return ret;
	State->Decoder.IPOffset --;
	
//	DEBUG_S(" %s", casArithOps[op_num]);
	
	ret = RME_Int_ParseModRM(State, NULL, &dest, 0);
	if(ret)	return ret;
	
	// Read data, perform operation, set common flags
	READ_INSTR8(srcData);
	DEBUG_S(" 0x%02x", srcData);
	ALU_SELECT_OPERATION();
	SET_COMM_FLAGS(State, v, width);
	if(dest)	*dest = v;
	
	return 0;
}

DEF_OPCODE_FCN(Arith, MIX)
{
	 int	ret, op_num;
	void	*destPtr;
	
	// Read rrr from ModRM
	ret = RME_Int_GetModRM(State, NULL, &op_num, NULL);
	if(ret)	return ret;
	State->Decoder.IPOffset --;
	
//	DEBUG_S(" %s", casArithOps[op_num]);
	
	ret = RME_Int_ParseModRMX(State, NULL, (void*)&destPtr, 0);
	if(ret)	return ret;
	
	State->Flags &= ~(FLAG_OF|FLAG_ZF|FLAG_SF|FLAG_PF);
	if( State->Decoder.bOverrideOperand )
	{
		uint32_t	v, srcData, *src = &srcData, *dest = destPtr;
		const int	width = 32;
		READ_INSTR32(srcData);
		DEBUG_S(" 0x%08x", srcData);
		
		ALU_SELECT_OPERATION();
		SET_COMM_FLAGS(State, v, width);
		if(dest)	*dest = v;
	}
	else
	{
		uint16_t	v, srcData, *src = &srcData, *dest = destPtr;
		const int	width = 16;
		READ_INSTR16(srcData);
		DEBUG_S(" 0x%04x", srcData);
		
		ALU_SELECT_OPERATION();
		SET_COMM_FLAGS(State, v, width);
		if(dest)	*dest = v;
	}
	
	return 0;
}

DEF_OPCODE_FCN(Arith, MI8X)
{
	 int	ret, op_num;
	void	*destPtr;
	
	// Read rrr from ModRM
	ret = RME_Int_GetModRM(State, NULL, &op_num, NULL);
	if(ret)	return ret;
	State->Decoder.IPOffset --;
	
//	DEBUG_S(" %s", casArithOps[op_num]);
	
	ret = RME_Int_ParseModRMX(State, NULL, (void*)&destPtr, 0);
	if(ret)	return ret;
	
	State->Flags &= ~(FLAG_OF|FLAG_ZF|FLAG_SF|FLAG_PF);
	if( State->Decoder.bOverrideOperand )
	{
		uint32_t	v, srcData, *src = &srcData, *dest = destPtr;
		const int	width = 32;
		READ_INSTR8S( srcData );
		DEBUG_S(" 0x%08x", srcData);
		
		ALU_SELECT_OPERATION();
		SET_COMM_FLAGS(State, v, width);
		if(dest)	*dest = v;
	}
	else
	{
		uint16_t	v, srcData, *src = &srcData, *dest = destPtr;
		const int	width = 16;
		READ_INSTR8S( srcData );
		DEBUG_S(" 0x%04x", srcData);
		
		ALU_SELECT_OPERATION();
		SET_COMM_FLAGS(State, v, width);
		if(dest)	*dest = v;
	}
	
	return 0;
}

DEF_OPCODE_FCN(INC, Reg)
{
	if( State->Decoder.bOverrideOperand )
	{
		uint32_t	*dest = (void*)RegW(State, Param);
		const int	width = 32;
		{ALU_OPCODE_INC_CODE}
	}
	else
	{
		uint16_t	*dest = RegW(State, Param);
		const int	width = 16;
		{ALU_OPCODE_INC_CODE}
	}
	
	return 0;
}

DEF_OPCODE_FCN(DEC, Reg)
{
	if( State->Decoder.bOverrideOperand )
	{
		uint32_t	*dest = (void*)RegW(State, Param);
		const int	width = 32;
		{ALU_OPCODE_DEC_CODE}
	}
	else
	{
		uint16_t	*dest = RegW(State, Param);
		const int	width = 16;
		{ALU_OPCODE_DEC_CODE}
	}
	return 0;
}

DEF_OPCODE_FCN(ArithMisc, MI)	// 0xF6
{
	 int	ret, op_num;
	const int	width = 8;
	uint8_t	v=0, *arg;
	uint8_t	*src, *dest;

	ret = RME_Int_GetModRM(State, NULL, &op_num, NULL);	State->Decoder.IPOffset --;
	if(ret)	return ret;
	
	ret = RME_Int_ParseModRM(State, NULL, &arg, 0);
	if(ret)	return ret;
	
	// Set up defaults
	src = arg;
	dest = arg;
	
	MISC_SELECT_OPERATION();
	
	return 0;
}

DEF_OPCODE_FCN(ArithMisc, MIX)	// 0xF7
{
	 int	ret, op_num;
	void	*arg;

	// Get operation number
	ret = RME_Int_GetModRM(State, NULL, &op_num, NULL);	State->Decoder.IPOffset --;
	if(ret)	return ret;
	
//	DEBUG_S(" %s", casMiscOps[op_num]);
	
	// Get argument (defaults to source)
	ret = RME_Int_ParseModRMX(State, NULL, (void*)&arg, 0);
	if(ret)	return ret;
	
	if( State->Decoder.bOverrideOperand )
	{
		const int	width=32;
		uint32_t	v=0, *dest=arg, *src = arg;
		MISC_SELECT_OPERATION();
	}
	else
	{
		const int	width=16;
		uint16_t	v=0, *dest=arg, *src = arg;
		MISC_SELECT_OPERATION();
	}
	
	return 0;
}

DEF_OPCODE_FCN(IMUL,MI8X)	// 0x6B
{
	 int	ret;

	const int	width = 16;
	uint16_t	result;
	uint16_t	*src, *dest;
	uint16_t	imm16;

	if( State->Decoder.bOverrideOperand )
	{
		ERROR_S("IMUL (MIX) OvrSize Unimpl");
		return RME_ERR_BUG;
	}
	ret = RME_Int_ParseModRMX(State, &dest, &src, 0);
	if(ret)	return ret;

	READ_INSTR8S(imm16);
	DEBUG_S(" 0x%04x", imm16);

	result = *src * imm16;
	SET_COMM_FLAGS(State, result, width);
	_IMUL_FLAGS
	*dest = result;
	
	return 0;
}

DEF_OPCODE_FCN(IMUL,MIX)	// 0x69
{
	 int	ret;

	const int	width = 16;
	uint16_t	result;
	uint16_t	*src, *dest;
	uint16_t	imm16;

	if( State->Decoder.bOverrideOperand )
	{
		ERROR_S("IMUL (MIX) OvrSize Unimpl");
		return RME_ERR_BUG;
	}
	ret = RME_Int_ParseModRMX(State, &dest, &src, 0);
	if(ret)	return ret;

	READ_INSTR16(imm16);
	DEBUG_S(" 0x%04x", imm16);

	result = *src * imm16;
	SET_COMM_FLAGS(State, result, width);
	_IMUL_FLAGS
	*dest = result;
	
	return 0;
}

DEF_OPCODE_FCN(IMUL,RMX)	// 0x0F 0xAF
{
	 int	ret;

	const int	width = 16;
	uint16_t	result;
	uint16_t	*src, *dest;

	if( State->Decoder.bOverrideOperand )
	{
		ERROR_S("IMUL (MIX) OvrSize Unimpl");
		return RME_ERR_BUG;
	}
	ret = RME_Int_ParseModRMX(State, &dest, &src, 0);
	if(ret)	return ret;
	
	result = *src * *dest;
	
	SET_COMM_FLAGS(State, result, width);
	_IMUL_FLAGS
	*dest = result;
	
	return 0;
}

// 0xC0 - Shift by Imm8
DEF_OPCODE_FCN(Shift, MI)
{
	const int	width = 8;
	 int	ret, op_num;
	uint8_t	*dest;
	uint8_t	srcData, *src = &srcData;

	ret = RME_Int_GetModRM(State, NULL, &op_num, NULL);	State->Decoder.IPOffset --;
	if(ret)	return ret;
	
	ret = RME_Int_ParseModRM(State, NULL, &dest, 0);
	if(ret)	return ret;
	
	READ_INSTR8(srcData);
	DEBUG_S(" 0x%02x", srcData);
	
	SHIFT_SELECT_OPERATION();
	
	return 0;
}

// 0xC1 - Shift Extended by Imm8
DEF_OPCODE_FCN(Shift, MI8X)
{
	 int	ret, op_num;
	uint16_t	*destPtr;
	uint8_t	srcData, *src = &srcData;

	ret = RME_Int_GetModRM(State, NULL, &op_num, NULL);	State->Decoder.IPOffset --;
	if(ret)	return ret;
	
//	DEBUG_S(" %s", casLogicOps[op_num]);
	
	ret = RME_Int_ParseModRMX(State, NULL, &destPtr, 0);
	if(ret)	return ret;
	
	READ_INSTR8(srcData);
	DEBUG_S(" 0x%02x", srcData);
	
	if( State->Decoder.bOverrideOperand )
	{
		const int	width = 32;
		uint32_t	*dest = (void*)destPtr;
		
		SHIFT_SELECT_OPERATION();
		
//		SET_COMM_FLAGS(State, *dest, width);
	}
	else
	{
		const int	width = 16;
		uint16_t	*dest = destPtr;
		
		SHIFT_SELECT_OPERATION();
		
//		SET_COMM_FLAGS(State, *dest, width);
	}
	
	return 0;
}

// 0xD0 - Shift with 1
DEF_OPCODE_FCN(Shift, M1)
{
	const int	width = 8;
	 int	ret, op_num;
	uint8_t	*dest;
	uint8_t	srcData = 1, *src = &srcData;

	ret = RME_Int_GetModRM(State, NULL, &op_num, NULL);	State->Decoder.IPOffset --;
	if(ret)	return ret;
	
	ret = RME_Int_ParseModRM(State, NULL, &dest, 0);
	if(ret)	return ret;
	
	DEBUG_S(" 1");
	
	SHIFT_SELECT_OPERATION();
	
	return 0;
}

// 0xD1 - Shift Extended with 1
DEF_OPCODE_FCN(Shift, M1X)
{
	 int	ret, op_num;
	uint16_t	*destPtr;
	uint8_t	srcData = 1, *src = &srcData;

	ret = RME_Int_GetModRM(State, NULL, &op_num, NULL);	State->Decoder.IPOffset --;
	if(ret)	return ret;
	
	ret = RME_Int_ParseModRMX(State, NULL, &destPtr, 0);
	if(ret)	return ret;
	
	DEBUG_S(" 1");
	
	if( State->Decoder.bOverrideOperand )
	{
		const int	width = 32;
		uint32_t	*dest = (void*)destPtr;
		
		SHIFT_SELECT_OPERATION();
	}
	else
	{
		const int	width = 16;
		uint16_t	*dest = destPtr;
		
		SHIFT_SELECT_OPERATION();
	}
	
	return 0;
}

// 0xD2 - Shift with CL
DEF_OPCODE_FCN(Shift, MCl)
{
	const int	width = 8;
	 int	ret, op_num;
	uint8_t	*dest;
	uint8_t	srcData = State->CX.B.L, *src = &srcData;

	ret = RME_Int_GetModRM(State, NULL, &op_num, NULL);	State->Decoder.IPOffset --;
	if(ret)	return ret;
	
	ret = RME_Int_ParseModRM(State, NULL, &dest, 0);
	if(ret)	return ret;
	
	DEBUG_S(" CL");
	
	SHIFT_SELECT_OPERATION();
	
	return 0;
}

// 0xD3 - Shift Extended with CL
DEF_OPCODE_FCN(Shift, MClX)
{
	 int	ret, op_num;
	uint16_t	*destPtr;
	uint8_t	srcData = State->CX.B.L, *src = &srcData;

	ret = RME_Int_GetModRM(State, NULL, &op_num, NULL);	State->Decoder.IPOffset --;
	if(ret)	return ret;
	
	ret = RME_Int_ParseModRMX(State, NULL, &destPtr, 0);
	if(ret)	return ret;
	
	DEBUG_S(" CL");
	
	if( State->Decoder.bOverrideOperand )
	{
		const int	width = 32;
		uint32_t	*dest = (void*)destPtr;
		
		SHIFT_SELECT_OPERATION();
	}
	else
	{
		const int	width = 16;
		uint16_t	*dest = destPtr;
		
		SHIFT_SELECT_OPERATION();
	}
	
	return 0;
}

// 0x0F 0xAC - Double Precision Shift Right by imm8
DEF_OPCODE_FCN(SHRD, I8)
{
	 int	ret;
	uint8_t	amt;
	void	*srcPtr, *destPtr;

	ret = RME_Int_ParseModRM(State, (void*)&srcPtr, (void*)&destPtr, 0);
	if(ret)	return ret;	
	
	READ_INSTR8(amt);
	DEBUG_S(" %i", amt);

	_EXT_OP_SD(srcPtr, destPtr, ALU_OPCODE_SHRD_CODE);
	return 0;
}

// 0x0F 0xAD - Double Precision Shift Right by CL
DEF_OPCODE_FCN(SHRD, Cl)
{
	 int	ret;
	uint8_t	amt;
	void	*srcPtr, *destPtr;

	ret = RME_Int_ParseModRM(State, (void*)&srcPtr, (void*)&destPtr, 0);
	if(ret)	return ret;	
	
	DEBUG_S(" CL");
	amt = State->CX.B.L;

	_EXT_OP_SD(srcPtr, destPtr, ALU_OPCODE_SHRD_CODE);
	return 0;
}

// 0x0F 0xA4 - Double Precision Shift Left by imm8
DEF_OPCODE_FCN(SHLD, I8)
{
	 int	ret;
	uint8_t	amt;
	void	*srcPtr, *destPtr;

	ret = RME_Int_ParseModRM(State, (void*)&srcPtr, (void*)&destPtr, 0);
	if(ret)	return ret;	
	
	READ_INSTR8(amt);
	DEBUG_S(" %i", amt);

	_EXT_OP_SD(srcPtr, destPtr, ALU_OPCODE_SHLD_CODE);
	return 0;
}

// 0x0F 0xA5 - Double Precision Shift Left by CL
DEF_OPCODE_FCN(SHLD, Cl)
{
	 int	ret;
	uint8_t	amt;
	void	*srcPtr, *destPtr;

	ret = RME_Int_ParseModRM(State, (void*)&srcPtr, (void*)&destPtr, 0);
	if(ret)	return ret;	
	
	DEBUG_S(" CL");
	amt = State->CX.B.L;

	_EXT_OP_SD(srcPtr, destPtr, ALU_OPCODE_SHLD_CODE);
	return 0;
}

