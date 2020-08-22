/*
 * Realmode Emulator
 * - ALU Operations Header
 * 
 */
#ifndef _RME_OPS_ALU_H_
#define _RME_OPS_ALU_H_

#define _ALU_SMASK	(1ULL << (width-1))
#define _ALU_NSMASK	((1ULL << (width-1))-1)

#define _ALU_ADD_SETFLAGS	\
	State->Flags |= (*dest&_ALU_SMASK) == (*src&_ALU_SMASK) && (v&_ALU_SMASK) != (*dest&_ALU_SMASK) ? FLAG_OF : 0; \
	if( *dest ) \
		State->Flags |= (v <= *src) ? FLAG_CF : 0; \
	else \
		State->Flags |= (v < *src) ? FLAG_CF : 0; \
	if( *dest&15 ) \
		State->Flags |= (v&15) <= (*src&15) ? FLAG_AF : 0;\
	else \
		State->Flags |= (v&15) < (*src&15) ? FLAG_AF : 0;

#define __SUB_FLAGS	\
	State->Flags |= (*dest < r) ? FLAG_CF : 0; \
	State->Flags |= ((*dest&15) < (r&15)-c) ? FLAG_AF : 0; \
	uint32_t _sub_tmp = ( ((*dest ^ r) & (*dest ^ v)) & (1ULL<<(width-1)) ); \
	if( _sub_tmp )	State->Flags |= FLAG_OF;
// 0: Add
#define ALU_OPCODE_ADD_CODE	\
	v = *dest + *src; \
	State->Flags &= ~(FLAG_PF|FLAG_ZF|FLAG_SF|FLAG_OF|FLAG_CF|FLAG_AF); \
	_ALU_ADD_SETFLAGS
// 1: Bitwise OR
#define ALU_OPCODE_OR_CODE	\
	v = *dest | *src; \
	State->Flags &= ~(FLAG_PF|FLAG_ZF|FLAG_SF|FLAG_OF|FLAG_CF);
// 2: Add with carry
#define ALU_OPCODE_ADC_CODE	\
	v = *dest + *src + ((State->Flags & FLAG_CF) ? 1 : 0); \
	State->Flags &= ~(FLAG_PF|FLAG_ZF|FLAG_SF|FLAG_OF|FLAG_CF|FLAG_AF); \
	_ALU_ADD_SETFLAGS \
	if(*dest && v == *src ) State->Flags |= FLAG_CF;
// 3: Subtract with Borrow
#define ALU_OPCODE_SBB_CODE	\
	int c = (State->Flags & FLAG_CF) ? 1 : 0; \
	uint64_t r = (*src + c); \
	v = *dest - r; \
	State->Flags &= ~(FLAG_OF|FLAG_CF|FLAG_AF); \
	__SUB_FLAGS
// 4: Bitwise AND
#define ALU_OPCODE_AND_CODE	\
	v = *dest & *src; \
	State->Flags &= ~(FLAG_PF|FLAG_ZF|FLAG_SF|FLAG_OF|FLAG_CF);
// 5: Subtract
#define ALU_OPCODE_SUB_CODE	\
	const int c = 0; \
	uint32_t r = *src; \
	v = *dest - r; \
	State->Flags &= ~(FLAG_PF|FLAG_ZF|FLAG_SF|FLAG_OF|FLAG_CF|FLAG_AF); \
	__SUB_FLAGS
// 6: Bitwise XOR
#define ALU_OPCODE_XOR_CODE	\
	v = *dest ^ *src; \
	State->Flags &= ~(FLAG_PF|FLAG_ZF|FLAG_SF|FLAG_OF|FLAG_CF);
// 7: Compare
// NOTE: The variable `hack` is just used as dummy space (and, yes it goes
//       out of scope, but nothing else should come back in, so it doesn't
//       matter)
#define ALU_OPCODE_CMP_CODE	\
	const int c = 0; \
	uint32_t r = *src; \
	v = *dest - r; \
	State->Flags &= ~(FLAG_PF|FLAG_ZF|FLAG_SF|FLAG_OF|FLAG_CF|FLAG_AF); \
	__SUB_FLAGS \
	dest = NULL;

// x: Test
#define ALU_OPCODE_TEST_CODE	\
	v = (*dest) & (*src); \
	State->Flags &= ~(FLAG_OF|FLAG_CF); \
	dest = NULL;
// x: NOT
#define ALU_OPCODE_NOT_CODE	\
	*dest = ~*src;
// x: NEG
// - TODO: OF/AF?
#define ALU_OPCODE_NEG_CODE	\
	State->Flags &= ~(FLAG_CF|FLAG_AF|FLAG_OF); \
	if( *src == 0 ) { \
		*dest = 0;\
	} \
	else { \
		State->Flags |= FLAG_CF; \
		State->Flags |= (*src == 1 << (width-1)) ? FLAG_OF : 0;\
		State->Flags |= ((*src&7) != 0) ? FLAG_AF : 0; \
		*dest = ~*src + 1; \
	} \
	SET_COMM_FLAGS(State, *dest, width);

// x: Increment
#define ALU_OPCODE_INC_CODE	\
	(*dest) ++; \
	State->Flags &= ~(FLAG_OF|FLAG_ZF|FLAG_SF|FLAG_PF|FLAG_AF); \
	if(*dest == (1UL << (width-1)))	State->Flags |= FLAG_OF; \
	if((*dest&15) == 0)	State->Flags |= FLAG_AF; \
	SET_COMM_FLAGS(State, *dest, width);
// x: Decrement
#define ALU_OPCODE_DEC_CODE	\
	(*dest) --; \
	State->Flags &= ~(FLAG_OF|FLAG_AF); \
	if(*dest + 1 == (1UL << (width-1)))	State->Flags |= FLAG_OF; \
	if((*dest & 15) + 1 == 16)	State->Flags |= FLAG_AF; \
	SET_COMM_FLAGS(State, *dest, width);

// 0: Rotate Left
#define ALU_OPCODE_ROL_CODE	\
	 int	amt = (*src & 31) % width; \
	if(amt > 0) { \
		*dest = (*dest << amt) | (*dest >> (width-amt)); \
		State->Flags &= ~(FLAG_OF|FLAG_CF); \
		State->Flags |= (*dest & 1) ? FLAG_CF : 0; \
		State->Flags |= (*dest >> (width-1)) ^ (*dest & 1) ? FLAG_OF : 0; \
	}
// 1: Rotate Right
#define ALU_OPCODE_ROR_CODE	\
	 int	amt = (*src & 31) % width; \
	if(amt > 0) { \
		*dest = (*dest >> amt) | (*dest << (width-amt)); \
		State->Flags &= ~(FLAG_OF|FLAG_CF); \
		State->Flags |= (*dest >> (width-1)) ? FLAG_CF : 0; \
		State->Flags |= ((*dest >> (width-1)) ^ (*dest >> (width-2))) & 1 ? FLAG_OF : 0; \
	}
	
// 2: Rotate Carry Left
#define ALU_OPCODE_RCL_CODE	\
	 int	amt = (*src & 31) % (width+1); \
	if( amt > 0 ) { \
		 int	cf_new, cf = (State->Flags & FLAG_CF) ? 1 : 0; \
		uint32_t val = *dest; \
		while(amt--) { \
			cf_new = val >> (width-1); \
			val = (val << 1) | cf; \
			cf = cf_new; \
		} \
		State->Flags &= ~(FLAG_OF|FLAG_CF); \
		State->Flags |= cf ? FLAG_CF : 0; \
		State->Flags |= (cf ^ (val >> (width-1))) ? FLAG_OF : 0; \
		*dest = val; \
	}
// 3: Rotate Carry Right
#define ALU_OPCODE_RCR_CODE	\
	 int	amt = (*src & 31) % (width+1); \
	if( amt > 0 ) { \
		 int	cf_new, cf = (State->Flags & FLAG_CF) ? 1 : 0; \
		uint32_t val = *dest; \
		while(amt--) { \
			cf_new = val & 1; \
			val = (val >> 1) | (cf << (width-1)); \
			cf = cf_new; \
		} \
		State->Flags &= ~(FLAG_OF|FLAG_CF); \
		State->Flags |= cf ? FLAG_CF : 0; \
		int high = val >> (width-2); \
		State->Flags |= ((high & 1) ^ (high >> 1)) ? FLAG_OF : 0; \
		*dest = val; \
	}
// 4: Shift Logical Left
#define ALU_OPCODE_SHL_CODE	\
	 int	amt = *src & 31; \
	if( amt > 0 ) { \
		State->Flags &= ~(FLAG_OF|FLAG_CF); \
		if(amt > width) \
			*dest = 0; \
		else { \
			State->Flags |= (*dest >> (width-amt)) & 1 ? FLAG_CF : 0; \
			State->Flags |= ((*dest >> (width-amt)) ^ (*dest >> (width-amt-1))) & 1 ? FLAG_OF : 0; \
			*dest <<= amt; \
		}\
		SET_COMM_FLAGS(State, *dest, width); \
	}
// 5: Shift Logical Right
#define ALU_OPCODE_SHR_CODE	\
	 int	amt = *src & 31; \
	if(amt > 0) { \
		State->Flags &= ~(FLAG_OF|FLAG_CF);\
		if( amt > width ) { \
			*dest = 0; \
		} else { \
			State->Flags |= (*dest >> (amt-1)) & 1 ? FLAG_CF : 0; \
			*dest >>= amt; \
		} \
		int high = *dest >> (width-2); \
		State->Flags |= ((high & 1) ^ (high >> 1)) ? FLAG_OF : 0; \
		SET_COMM_FLAGS(State, *dest, width); \
	}
// 6: Shift Arithmetic Left
#define ALU_OPCODE_SAL_CODE	ALU_OPCODE_SHL_CODE
// 7: Shift Arithmetic Right (applies sign extension)
#define ALU_OPCODE_SAR_CODE	\
	 int	amt = *src & 31; \
	if( amt > 0 ) { \
		int sgn = *dest >> (width-1); \
		State->Flags &= ~(FLAG_OF|FLAG_CF); \
		if(amt >= width) { \
			*dest = sgn ? -1 : 0; \
			State->Flags |= sgn ? FLAG_CF : 0; \
		} else { \
			State->Flags |= ((*dest >> (amt-1)) & 1) ? FLAG_CF : 0; \
			*dest = *dest >> amt; \
			*dest |= sgn ? 0xFFFFFFFF << (width - amt) : 0; \
		} \
		int high = *dest >> (width-2); \
		State->Flags |= ((high & 1) ^ (high >> 1)) ? FLAG_OF : 0; \
		SET_COMM_FLAGS(State, *dest, width); \
	}

// Misc 4: MUL
// CF,OF set if upper bits set; SF, ZF, AF and PF are undefined
#define ALU_OPCODE_MUL_CODE \
	uint64_t	result;\
	switch(width) { \
	case 8: \
		result = (uint16_t)State->AX.B.L * *src; \
		State->AX.W = result; \
		SET_COMM_FLAGS(State, State->AX.B.L, width);\
		break; \
	case 16: \
		result = (uint32_t)State->AX.W * (*src); \
		State->DX.W = result >> 16; \
		State->AX.W = result & 0xFFFF; \
		SET_COMM_FLAGS(State, State->AX.W, width);\
		break; \
	case 32: \
		result = (uint64_t)State->AX.D * (*src); \
		State->DX.D = result >> 32; \
		State->AX.D = result & 0xFFFFFFFF; \
		SET_COMM_FLAGS(State, State->AX.D, width);\
		break; \
	} \
	if(result >> width) \
		State->Flags |= FLAG_CF|FLAG_OF; \
	else \
		State->Flags &= ~(FLAG_CF|FLAG_OF);

#define _IMUL_FLAGS	\
	if(result < -(1 << (width-1)) || result > ((1 << (width-1))-1)) \
		State->Flags |= FLAG_CF|FLAG_OF; \
	else \
		State->Flags &= ~(FLAG_CF|FLAG_OF);

// Misc 5: IMUL
// CF,OF set if upper bits set; SF, ZF, AF and PF are undefined
#define ALU_OPCODE_IMUL_CODE \
	int64_t	result;\
	switch(width) { \
	case 8: \
		result = (int16_t)(int8_t)State->AX.B.L * (*(int8_t*)src); \
		State->AX.W = result; \
		SET_COMM_FLAGS(State, State->AX.B.L, width);\
		break; \
	case 16: \
		result = (int32_t)(int16_t)State->AX.W * (*(int16_t*)src); \
		State->DX.W = result >> 16; \
		State->AX.W = result & 0xFFFF; \
		SET_COMM_FLAGS(State, State->AX.W, width);\
		break; \
	case 32: \
		result = (int64_t)(int32_t)State->AX.D * (*(int32_t*)src); \
		State->DX.D = result >> 32; \
		State->AX.D = result & 0xFFFFFFFF; \
		SET_COMM_FLAGS(State, State->AX.D, width);\
		break; \
	} \
	_IMUL_FLAGS

// Misc 6: DIV
// NOTE: DIV is a real special case, as it has substantially different
//       behavior between different sizes (due to DX:AX)
#define ALU_OPCODE_DIV_CODE if( *src == 0 )	return RME_ERR_DIVERR; \
	switch(width) { \
	case 8: { \
		uint16_t	result, rem; \
		result = State->AX.W / *src; \
		rem = State->AX.W % *src; \
		if(result > 0xFF)	return RME_ERR_DIVERR; \
		State->AX.B.H = rem; \
		State->AX.B.L = result; \
		} break; \
	case 16: { \
		uint32_t	numerator, result, rem; \
		numerator = ((uint32_t)State->DX.W << 16) | State->AX.W; \
		result = numerator / *src; \
		rem = numerator % *src; \
		if(result > 0xFFFF)	return RME_ERR_DIVERR; \
		State->DX.W = rem; \
		State->AX.W = result; \
		} break; \
	case 32: { \
		uint64_t	numerator, result, rem; \
		numerator = ((uint64_t)State->DX.D << 32) | State->AX.D; \
		result = numerator / *src; \
		rem = numerator % *src; \
		if(result > 0xFFFFFFFF)	return RME_ERR_DIVERR; \
		State->DX.D = rem; \
		State->AX.D = result; \
		} break; \
	}
	
// Misc 7: IDIV
// NOTE: DIV is a real special case, as it has substantially different
//       behavior between different sizes (due to DX:AX)
// TODO: Test
#define ALU_OPCODE_IDIV_CODE if( *src == 0 )	return RME_ERR_DIVERR; \
	switch(width) { \
	case 8: { \
		int16_t	quot, rem; \
		int16_t num = (int16_t)State->AX.W, den = *(int8_t*)src; \
		quot = num / den; \
		rem = num - quot*den; \
		if(quot < -0x80 || quot > 0x7F)	return RME_ERR_DIVERR; \
		State->AX.B.H = rem; \
		State->AX.B.L = quot; \
		} break; \
	case 16: { \
		int32_t	numerator, result; \
		numerator = (int32_t)( ((uint32_t)State->DX.W << 16) | State->AX.W ); \
		result = numerator / *(int16_t*)src; \
		if(result > 0x7FFF || result < -0x8000)	return RME_ERR_DIVERR; \
		State->DX.W = numerator % (*(int16_t*)src); \
		State->AX.W = result; \
		} break; \
	case 32: { \
		int64_t	numerator, result; \
		numerator = (int64_t)( ((uint64_t)State->DX.D << 32) | State->AX.D ); \
		result = numerator / *(int32_t*)src; \
		if(result > 0x7FFFFFFF || result < -0x80000000)	return RME_ERR_DIVERR; \
		State->DX.D = numerator % (*(int32_t*)src); \
		State->AX.D = result; \
		} break; \
	}

// Double Precision Shift Right
#define ALU_OPCODE_SHRD_CODE	\
	if( amt > 0 ) { \
		State->Flags &= ~(FLAG_CF|FLAG_OF|FLAG_AF); \
		State->Flags |= (*dest >> (amt-1)) & 1 ? FLAG_CF : 0; \
		*dest >>= amt; \
		*dest |= *src << (width - amt); \
		if( amt == 1 ) \
			State->Flags |= ((*dest >> (width-1)) ^ (*dest >> (width-2))) & 1 ? FLAG_OF : 0; \
		SET_COMM_FLAGS(State, *dest, width); \
	}
// Double Precision Shift Left
#define ALU_OPCODE_SHLD_CODE	\
	if( amt > 0 ) { \
		State->Flags &= ~(FLAG_CF|FLAG_OF|FLAG_AF); \
		State->Flags |= (*dest >> (width-amt)) & 1 ? FLAG_CF : 0; \
		*dest <<= amt; \
		*dest |= *src >> (width - amt); \
		if( amt == 1 ) \
			State->Flags |= ((*dest >> (width-1)) ^ !!(State->Flags & FLAG_CF)) & 1 ? FLAG_OF : 0; \
		SET_COMM_FLAGS(State, *dest, width); \
	}


#endif
