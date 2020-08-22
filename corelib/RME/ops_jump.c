/*
 * Realmode Emulator
 * - Jumps (Conditional and Unconditional)
 */
#include "rme.h"
#include "rme_internal.h"
#include "opcode_prototypes.h"

#define MAKE_COND_JUMP_S(__condition) do {\
	uint16_t	dist; \
	READ_INSTR8S( dist ); \
	DEBUG_S(" .+0x%04x", dist);\
	if( (__condition)) { \
		State->IP += dist; \
	} \
	return 0; \
} while(0)

#define MAKE_COND_JUMP_N(__condition) do {\
	uint16_t	dist; \
	READ_INSTR16( dist ); \
	DEBUG_S(" .+0x%04x", dist);\
	if( (__condition)) { \
		State->IP += dist; \
	} \
	return 0; \
} while(0)

#define CONDITION_O 	(State->Flags & FLAG_OF)
#define CONDITION_NO	(!CONDITION_O)
#define CONDITION_C 	(State->Flags & FLAG_CF)
#define CONDITION_NC	(!CONDITION_C)
#define CONDITION_Z 	(State->Flags & FLAG_ZF)
#define CONDITION_NZ	(!CONDITION_Z)
#define CONDITION_BE	(State->Flags & FLAG_CF || State->Flags & FLAG_ZF)
#define CONDITION_A 	(!CONDITION_BE)
#define CONDITION_S 	(State->Flags & FLAG_SF)
#define CONDITION_NS 	(!CONDITION_S)
#define CONDITION_PE	(State->Flags & FLAG_PF)
#define CONDITION_PO	(!CONDITION_PE)
#define CONDITION_L 	(!!(State->Flags & FLAG_SF) != !!(State->Flags & FLAG_OF))
#define CONDITION_GE	(!CONDITION_L)
#define CONDITION_LE	((State->Flags & FLAG_ZF) || !!(State->Flags & FLAG_SF) != !!(State->Flags & FLAG_OF))
#define CONDITION_G 	(!CONDITION_LE)

// === CODE ===
DEF_OPCODE_FCN(JO, S)  { MAKE_COND_JUMP_S(CONDITION_O);  }
DEF_OPCODE_FCN(JNO, S) { MAKE_COND_JUMP_S(CONDITION_NO); }
DEF_OPCODE_FCN(JC, S)  { MAKE_COND_JUMP_S(CONDITION_C);  }
DEF_OPCODE_FCN(JNC, S) { MAKE_COND_JUMP_S(CONDITION_NC); }
DEF_OPCODE_FCN(JZ, S)  { MAKE_COND_JUMP_S(CONDITION_Z);  }
DEF_OPCODE_FCN(JNZ, S) { MAKE_COND_JUMP_S(CONDITION_NZ); }
DEF_OPCODE_FCN(JBE,S)  { MAKE_COND_JUMP_S(CONDITION_BE); }
DEF_OPCODE_FCN(JA, S)  { MAKE_COND_JUMP_S(CONDITION_A);  }
DEF_OPCODE_FCN(JS, S)  { MAKE_COND_JUMP_S(CONDITION_S);  }
DEF_OPCODE_FCN(JNS, S) { MAKE_COND_JUMP_S(CONDITION_NS); }
DEF_OPCODE_FCN(JPE,S)  { MAKE_COND_JUMP_S(CONDITION_PE); }
DEF_OPCODE_FCN(JPO, S) { MAKE_COND_JUMP_S(CONDITION_PO); }
DEF_OPCODE_FCN(JL, S)  { MAKE_COND_JUMP_S(CONDITION_L);  }
DEF_OPCODE_FCN(JGE, S) { MAKE_COND_JUMP_S(CONDITION_GE); }
DEF_OPCODE_FCN(JLE,S)  { MAKE_COND_JUMP_S(CONDITION_LE); }
DEF_OPCODE_FCN(JG, S)  { MAKE_COND_JUMP_S(CONDITION_G);  }

DEF_OPCODE_FCN(JO, N) { MAKE_COND_JUMP_N(CONDITION_O ); }
DEF_OPCODE_FCN(JNO,N) { MAKE_COND_JUMP_N(CONDITION_NO); }
DEF_OPCODE_FCN(JC, N) { MAKE_COND_JUMP_N(CONDITION_C ); }
DEF_OPCODE_FCN(JNC,N) { MAKE_COND_JUMP_N(CONDITION_NC); }
DEF_OPCODE_FCN(JZ, N) { MAKE_COND_JUMP_N(CONDITION_Z ); }
DEF_OPCODE_FCN(JNZ,N) { MAKE_COND_JUMP_N(CONDITION_NZ); }
DEF_OPCODE_FCN(JBE,N) { MAKE_COND_JUMP_N(CONDITION_BE); }
DEF_OPCODE_FCN(JA, N) { MAKE_COND_JUMP_N(CONDITION_A ); }
DEF_OPCODE_FCN(JS, N) { MAKE_COND_JUMP_N(CONDITION_S ); }
DEF_OPCODE_FCN(JNS,N) { MAKE_COND_JUMP_N(CONDITION_NS); }
DEF_OPCODE_FCN(JPE,N) { MAKE_COND_JUMP_N(CONDITION_PE); }
DEF_OPCODE_FCN(JPO,N) { MAKE_COND_JUMP_N(CONDITION_PO); }
DEF_OPCODE_FCN(JL, N) { MAKE_COND_JUMP_N(CONDITION_L ); }
DEF_OPCODE_FCN(JGE,N) { MAKE_COND_JUMP_N(CONDITION_GE); }
DEF_OPCODE_FCN(JLE,N) { MAKE_COND_JUMP_N(CONDITION_LE); }
DEF_OPCODE_FCN(JG, N) { MAKE_COND_JUMP_N(CONDITION_G ); }

DEF_OPCODE_FCN(JCXZ, S)
{
	uint16_t	dist;
	READ_INSTR8S( dist );
	DEBUG_S(" .+0x%02x", dist);
	
	if( State->Decoder.bOverrideOperand && State->CX.D != 0 )
		return 0;
	if( !State->Decoder.bOverrideOperand && State->CX.W != 0 )
		return 0;
	
	State->IP += dist;
	return 0;
}

// Looping
DEF_OPCODE_FCN(LOOPNZ, S)
{
	uint16_t	dist;
	READ_INSTR8S( dist );
	DEBUG_S(" .+0x%02x", dist);
	
	State->CX.W --;
	if(State->CX.W != 0 && !(State->Flags & FLAG_ZF)) {
		State->IP += dist;
	}
	
	return 0;
}

DEF_OPCODE_FCN(LOOPZ, S)
{
	uint16_t	dist;
	READ_INSTR8S( dist );
	DEBUG_S(" .+0x%02x", dist);
	
	State->CX.W --;
	if(State->CX.W != 0 && State->Flags & FLAG_ZF) {
		State->IP += dist;
	}
	
	return 0;
}

DEF_OPCODE_FCN(LOOP, S)
{
	uint16_t	dist;
	READ_INSTR8S( dist );
	DEBUG_S(" .+0x%02x", dist);
	
	State->CX.W --;
	if(State->CX.W != 0) {
		State->IP += dist;
	}
	
	return 0;
}

// Unconditional Jumps
DEF_OPCODE_FCN(JMP, N)
{
	uint16_t	ofs;
	READ_INSTR16( ofs );
	DEBUG_S(" .+0x%04x", ofs);
	State->IP += ofs;
	return 0;
}

DEF_OPCODE_FCN(JMP, F)
{
	uint16_t	seg, ofs;
	READ_INSTR16( ofs );
	READ_INSTR16( seg );
	DEBUG_S(" 0x%04x:%04x", seg, ofs);
	State->CS = seg;
	State->IP = ofs;
	State->Decoder.bDontChangeIP = 1;
	return 0;
}

DEF_OPCODE_FCN(JMP, S)
{
	uint16_t	ofs;
	READ_INSTR8S( ofs );
	DEBUG_S(" .+0x%02x", ofs);
	State->IP += ofs;
	return 0;
}
