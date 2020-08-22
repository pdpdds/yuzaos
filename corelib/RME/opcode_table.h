/*
 */
#ifndef _RME_OPCODE_TABLE_
#define _RME_OPCODE_TABLE_

#include "opcode_prototypes.h"

#define REP_2(val)	val,val
#define REP_4(val)	val,val,val,val
#define REP_8(val)	val,val,val,val,val,val,val,val

#define UNDEF_OP	{"Undef","",NULL,0,NULL}
#define DEF_OP_X(name,type,arg,subnames)	{#name,#type,RME_Op_##name##_##type,arg,subnames}
#define DEF_OP_A(name,type,arg)	DEF_OP_X(name, type, arg, NULL)
#define DEF_OP_N(name,type,names)	DEF_OP_X(name, type, 0, names)
#define DEF_OP(name,type)	DEF_OP_X(name, type, 0, NULL)

#define DEF_ALU_OP(name)	DEF_OP(name,MR), DEF_OP(name,MRX),\
	DEF_OP(name,RM), DEF_OP(name,RMX),\
	DEF_OP(name,AI), DEF_OP(name,AIX)

#define DEF_REG_OP(name)	DEF_OP_A(name,Reg,AX), DEF_OP_A(name,Reg,CX),\
	DEF_OP_A(name,Reg,DX), DEF_OP_A(name,Reg,BX),\
	DEF_OP_A(name,Reg,SP), DEF_OP_A(name,Reg,BP),\
	DEF_OP_A(name,Reg,SI), DEF_OP_A(name,Reg,DI)
#define DEF_REGB_OP(name)	DEF_OP_A(name,RegB,AX), DEF_OP_A(name,RegB,CX),\
	DEF_OP_A(name,RegB,DX), DEF_OP_A(name,RegB,BX),\
	DEF_OP_A(name,RegB,SP), DEF_OP_A(name,RegB,BP),\
	DEF_OP_A(name,RegB,SI), DEF_OP_A(name,RegB,DI)


static const char *casArithOps[] = {"ADD", "OR", "ADC", "SBB", "AND", "SUB", "XOR", "CMP"};
static const char *casMiscOps[] = {"TEST", "M1-", "NOT", "NEG", "MUL", "IMUL", "DIV", "IDIV"};
static const char *casShiftOps[] = {"ROL", "ROR", "RCL", "RCR", "SHL", "SHR", "SAL", "SAR"};
//static const char *casUnaryOps[] = {"INC", "DEC", "", "", "", "", "PUSH", ""};

typedef struct sOperation
{
	const char	*Name;
	const char	*Type;
	tOpcodeFcn Function;
	 int	Arg;
	const char	**ModRMNames;
} tOperation;

const tOperation	caOperations[256] = {
	/* ADD RM(X), PUSH ES, POP ES */
	/* 0x00 */	DEF_ALU_OP(ADD), DEF_OP_A(PUSH,Seg,SREG_ES), DEF_OP_A(POP,Seg,SREG_ES),
	/* OR RM(X), PUSH CS, #UD */
	/* 0x08 */	DEF_ALU_OP(OR ), DEF_OP_A(PUSH,Seg,SREG_CS), DEF_OP(Ext,0F),
	/* ADC RM(X), PUSH SS, #UD (POP SS) */
	/* 0x10 */	DEF_ALU_OP(ADC), DEF_OP_A(PUSH,Seg,SREG_SS), DEF_OP_A(POP,Seg,SREG_SS),
	/* SBB RM(X), PUSH DS, POP DS */
	/* 0x18 */	DEF_ALU_OP(SBB), DEF_OP_A(PUSH,Seg,SREG_DS), DEF_OP_A(POP,Seg,SREG_DS),
	/* AND RM(X), ES Override, #UD */
	/* 0x20 */	DEF_ALU_OP(AND), DEF_OP_A(Ovr,Seg,SREG_ES), DEF_OP(DAA,z),
	/* SUB RM(X), CS Override, #UD */
	/* 0x28 */	DEF_ALU_OP(SUB), DEF_OP_A(Ovr,Seg,SREG_CS), DEF_OP(DAS,z),
	/* XOR RM(X), SS Override, #UD */
	/* 0x30 */	DEF_ALU_OP(XOR), DEF_OP_A(Ovr,Seg,SREG_SS), DEF_OP(AAA,z),
	/* CMP RM(X), DS Override, #UD */
	/* 0x38 */	DEF_ALU_OP(CMP), DEF_OP_A(Ovr,Seg,SREG_DS), DEF_OP(AAS,z),
	/* INC R */
	/* 0x40 */	DEF_REG_OP(INC),
	/* DEC R */
	/* 0x48 */	DEF_REG_OP(DEC),
	/* PUSH R */
	/* 0x50 */	DEF_REG_OP(PUSH),
	/* PUSH R */
	/* 0x58 */	DEF_REG_OP(POP),
	/* PUSHA, POPA, 2x #UD */
	/* 0x60 */	DEF_OP(PUSH,A), DEF_OP(POP,A), UNDEF_OP, UNDEF_OP,
	/*  2x #UD, OPSZ, ADDRSZ */
	/*  0x64*/	DEF_OP_A(Ovr,Seg,SREG_FS), DEF_OP_A(Ovr,Seg,SREG_GS), DEF_OP(Ovr, OpSize), DEF_OP(Ovr, AddrSize),
	/* PUSH I, #UD, PUSH I8, #UD */
	/* 0x68 */	DEF_OP(PUSH,I), DEF_OP(IMUL,MIX), DEF_OP(PUSH,I8), DEF_OP(IMUL,MI8X),
	/*  0x6C*/	DEF_OP(IN,SB), DEF_OP(IN,SW), DEF_OP(OUT,SB), DEF_OP(OUT,SW),
	// Short Conditional Jumps
	/* 0x70 */	DEF_OP(JO,S), DEF_OP(JNO,S), DEF_OP(JC ,S), DEF_OP(JNC,S),
	/*  0x74*/	DEF_OP(JZ,S), DEF_OP(JNZ,S), DEF_OP(JBE,S), DEF_OP(JA ,S),
	/* 0x78 */	DEF_OP(JS,S), DEF_OP(JNS,S), DEF_OP(JPE,S), DEF_OP(JPO,S),
	/*  0x7C*/	DEF_OP(JL,S), DEF_OP(JGE,S), DEF_OP(JLE,S), DEF_OP(JG ,S),
	/* 0x80 */	DEF_OP_N(Arith,MI,casArithOps), DEF_OP_N(Arith,MIX,casArithOps),
				UNDEF_OP, DEF_OP_N(Arith, MI8X,casArithOps),
	/*  0x84*/	DEF_OP(TEST,MR), DEF_OP(TEST,MRX), DEF_OP(XCHG,RM), DEF_OP(XCHG,RMX),
	/* 0x88 */	DEF_OP(MOV,MR), DEF_OP(MOV,MRX), DEF_OP(MOV,RM), DEF_OP(MOV,RMX),
	/*  0x8C*/	DEF_OP(MOV,RS), DEF_OP(LEA,z), DEF_OP(MOV,SR), DEF_OP(POP,MX),
	/* 0x90 */	DEF_REG_OP(XCHG),
	/* 0x98 */	DEF_OP(CBW,z), DEF_OP(CWD,z), DEF_OP(CALL,F), UNDEF_OP,
	/*  0x9C*/	DEF_OP(PUSH,F), DEF_OP(POP,F), DEF_OP(Flag,SAHF), DEF_OP(Flag,LAHF),
	/* 0xA0 */	DEF_OP(MOV,AMo), DEF_OP(MOV,AMoX), DEF_OP(MOV,MoA), DEF_OP(MOV,MoAX),
	/*  0xA4*/	DEF_OP(MOV,SB), DEF_OP(MOV,SW), DEF_OP(CMP,SB), DEF_OP(CMP,SW),
	/* 0xA8 */	DEF_OP(TEST,AI), DEF_OP(TEST,AIX), DEF_OP(STO,SB), DEF_OP(STO,SW),
	/*  0xAC*/	DEF_OP(LOD,SB), DEF_OP(LOD,SW), DEF_OP(SCA,SB), DEF_OP(SCA,SW),
	/* 0xB0 */	DEF_REGB_OP(MOV),
	/* 0xB8 */	DEF_REG_OP(MOV),
	/* 0xC0 */	DEF_OP_N(Shift,MI,casShiftOps), DEF_OP_N(Shift, MI8X,casShiftOps),
				DEF_OP(RET,iN), DEF_OP(RET,N),
	/*  0xC4*/	DEF_OP(LES,z), DEF_OP(LDS,z), DEF_OP(MOV,MI), DEF_OP(MOV,MIX),
	/* 0xC8 */	DEF_OP(ENTER,z), DEF_OP(LEAVE,z), DEF_OP(RET,iF), DEF_OP(RET,F),
	/*  0xCC*/	DEF_OP(INT,3), DEF_OP(INT,I), DEF_OP(INTO,z), DEF_OP(IRET,z),
	/* 0xD0 */	DEF_OP_N(Shift,M1,casShiftOps), DEF_OP_N(Shift,M1X,casShiftOps),
				DEF_OP_N(Shift,MCl,casShiftOps), DEF_OP_N(Shift,MClX,casShiftOps),
	/*  0xD4*/	DEF_OP(AAM,z), DEF_OP(AAD,z), UNDEF_OP, DEF_OP(XLAT,z),
	/* 0xD8 */	UNDEF_OP, UNDEF_OP, UNDEF_OP, UNDEF_OP,
	/*  0xDC*/	UNDEF_OP, UNDEF_OP, UNDEF_OP, UNDEF_OP,
	/* 0xE0 */	DEF_OP(LOOPNZ,S), DEF_OP(LOOPZ,S), DEF_OP(LOOP,S), DEF_OP(JCXZ,S),
	/*  0xE4*/	DEF_OP(IN,AI ), DEF_OP(IN,AIX ), DEF_OP(OUT,AI ), DEF_OP(OUT,AIX ),
	/* 0xE8 */	DEF_OP(CALL,N), DEF_OP(JMP,N), DEF_OP(JMP,F), DEF_OP(JMP,S),
	/*  0xEC*/	DEF_OP(IN,ADx), DEF_OP(IN,ADxX), DEF_OP(OUT,DxA), DEF_OP(OUT,DxAX),
	/* 0xF0 */	UNDEF_OP, UNDEF_OP, DEF_OP(Prefix, REPNZ), DEF_OP(Prefix, REP),
	/*  0xF4*/	DEF_OP(HLT,z), DEF_OP(Flag,CMC),
				DEF_OP_N(ArithMisc, MI,casMiscOps), DEF_OP_N(ArithMisc, MIX,casMiscOps),
	/* 0xF8 */	DEF_OP(Flag, CLC), DEF_OP(Flag, STC), DEF_OP(Flag, CLI), DEF_OP(Flag, STI),
	/*  0xFC*/	DEF_OP(Flag, CLD), DEF_OP(Flag, STD), DEF_OP(Unary,M), DEF_OP(Unary,MX)
	/*0x100 */
};

const tOperation	caOperations0F[256] = {
	/* 0x00 */	REP_8(UNDEF_OP),	// 0x01 = LGDT
	/* 0x08 */	REP_8(UNDEF_OP),
	/* 0x10 */	REP_8(UNDEF_OP),
	/* 0x18 */	REP_8(UNDEF_OP),
	/* 0x20 */	REP_8(UNDEF_OP),
	/* 0x28 */	REP_8(UNDEF_OP),
	/* 0x30 */	REP_8(UNDEF_OP),
	/* 0x38 */	REP_8(UNDEF_OP),
	/* 0x40 */	REP_8(UNDEF_OP),
	/* 0x48 */	REP_8(UNDEF_OP),
	/* 0x50 */	REP_8(UNDEF_OP),
	/* 0x58 */	REP_8(UNDEF_OP),
	/* 0x60 */	REP_8(UNDEF_OP),
	/* 0x68 */	REP_8(UNDEF_OP),
	/* 0x70 */	REP_8(UNDEF_OP),
	/* 0x78 */	REP_8(UNDEF_OP),
	/* 0x80 */	DEF_OP(JO,N), DEF_OP(JNO,N), DEF_OP(JC ,N), DEF_OP(JNC,N),
	/*  0x84*/	DEF_OP(JZ,N), DEF_OP(JNZ,N), DEF_OP(JBE,N), DEF_OP(JA ,N),
	/* 0x88 */	DEF_OP(JS,N), DEF_OP(JNS,N), DEF_OP(JPE,N), DEF_OP(JPO,N),
	/*  0x8C*/	DEF_OP(JL,N), DEF_OP(JGE,N), DEF_OP(JLE,N), DEF_OP(JG ,N),
	/* 0x90 */	REP_8(UNDEF_OP),
	/* 0x98 */	REP_8(UNDEF_OP),
	/* 0xA0 */	DEF_OP_A(PUSH,Seg,SREG_FS), DEF_OP_A(POP,Seg,SREG_FS), UNDEF_OP, UNDEF_OP,
	/*  0xA4*/	DEF_OP(SHLD, I8), DEF_OP(SHLD, Cl), UNDEF_OP, UNDEF_OP,
	/* 0xA8 */	DEF_OP_A(PUSH,Seg,SREG_GS), DEF_OP_A(POP,Seg,SREG_GS), UNDEF_OP, UNDEF_OP,
	/*  0xAC*/	DEF_OP(SHRD, I8), DEF_OP(SHRD, Cl), UNDEF_OP, DEF_OP(IMUL,RMX),
	/* 0xB0 */	UNDEF_OP, UNDEF_OP, DEF_OP(LSS,z), UNDEF_OP,
	/*  0xB4*/	DEF_OP(LFS, z), DEF_OP(LGS,z), DEF_OP(MOV,Z), DEF_OP(MOV,ZX),
	/* 0xB8 */	REP_2(UNDEF_OP), DEF_OP(BTx,RI8), UNDEF_OP,
	/*  0xBC*/	DEF_OP(BSF,z), UNDEF_OP, UNDEF_OP, UNDEF_OP,
	/* 0xC0 */	REP_8(UNDEF_OP),
	/* 0xC8 */	REP_8(UNDEF_OP),
	/* 0xD0 */	REP_8(UNDEF_OP),
	/* 0xD8 */	REP_8(UNDEF_OP),
	/* 0xE0 */	REP_8(UNDEF_OP),
	/* 0xE8 */	REP_8(UNDEF_OP),
	/* 0xF0 */	REP_8(UNDEF_OP),
	/* 0xF8 */	REP_8(UNDEF_OP)
};

#endif
