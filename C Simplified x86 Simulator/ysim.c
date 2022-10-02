#include "ysim.h"

#include "errors.h"

/************************** Utility Routines ****************************/

/** Return nybble from op (pos 0: least-significant; pos 1:
 *  most-significant)
 */
static Byte
get_nybble(Byte op, int pos) {
  return (op >> (pos * 4)) & 0xF;
}

/************************** Condition Codes ****************************/

/** Conditions used in instructions */
typedef enum {
  ALWAYS_COND, LE_COND, LT_COND, EQ_COND, NE_COND, GE_COND, GT_COND
} Condition;

/** accessing condition code flags */
static inline bool get_cc_flag(Byte cc, unsigned flagBitIndex) {
  return !!(cc & (1 << flagBitIndex));
}
static inline bool get_zf(Byte cc) { return get_cc_flag(cc, ZF_CC); }
static inline bool get_sf(Byte cc) { return get_cc_flag(cc, SF_CC); }
static inline bool get_of(Byte cc) { return get_cc_flag(cc, OF_CC); }

/** Return true iff the condition specified in the least-significant
 *  nybble of op holds in y86.  Encoding of Figure 3.15 of Bryant's
 *  CompSys3e.
 */
bool
check_cc(const Y86 *y86, Byte op)
{
  bool ret = false;
  Condition condition = get_nybble(op, 0);
  Byte cc = read_cc_y86(y86);
  switch (condition) {
  case ALWAYS_COND:
    ret = true;
    break;
  case LE_COND:
    ret = (get_sf(cc) ^ get_of(cc)) | get_zf(cc);
    break;
  case LT_COND:
	ret = get_sf(cc) ^ get_of(cc);
	break;
  case EQ_COND:
	ret = get_zf(cc);
	break;
  case NE_COND:
	ret = !get_zf(cc);
	break;
  case GE_COND:
	ret = !(get_sf(cc) ^ get_of(cc));
	break;
  case GT_COND:
	ret = !((get_sf(cc) ^ get_of(cc)) | get_zf(cc));
	break;
  default: {
    Address pc = read_pc_y86(y86);
    fatal("%08lx: bad condition code %d\n", pc, condition);
    break;
    }
  }
  return ret;
}

/** return true iff word has its sign bit set */
static inline bool
isLt0(Word word) {
  return (word & (1UL << (sizeof(Word)*CHAR_BIT - 1))) != 0;
}

/** Set condition codes for addition operation with operands opA, opB
 *  and result with result == opA + opB.
 */
static void
set_add_arith_cc(Y86 *y86, Word opA, Word opB, Word result)
{
  Byte flags = 0;
  if(result == 0) flags |= 4;
  if(isLt0(result)) flags |= 2;
  if(opA > 0 && opB > 0 && result < 0) flags |= 1;
  if(opA < 0 && opB < 0 && result > 0) flags |= 1;
  write_cc_y86(y86, flags);
}

/** Set condition codes for subtraction operation with operands opA, opB
 *  and result with result == opA - opB.
 */
static void
set_sub_arith_cc(Y86 *y86, Word opA, Word opB, Word result)
{
	Byte flags = 0;
	if(result == 0) flags |= 4;
	if(isLt0(result)) flags |= 2;
	if((opA ^ opB) < 0 && (opA ^ result) < 0) flags |= 1;
	write_cc_y86(y86, flags);
}

static void
set_logic_op_cc(Y86 *y86, Word result)
{
	Byte flags = 0;
	if(result == 0) flags |= 4;
	if(isLt0(result)) flags |= 2;
	write_cc_y86(y86, flags);
}

/**************************** Operations *******************************/

static void
op1(Y86 *y86, Byte op, Register regA, Register regB)
{
  enum {ADDL_FN, SUBL_FN, ANDL_FN, XORL_FN };
  Byte function = get_nybble(op, 0);
  Word opA = read_register_y86(y86, regA);
  Word opB = read_register_y86(y86, regB);
  Word result;
  switch(function) {
	  case ADDL_FN:
		result = opA + opB;
		set_add_arith_cc(y86, opA, opB, result);
		write_register_y86(y86, regB, result);
		break;
	  case SUBL_FN:
		result = opB - opA;
		set_sub_arith_cc(y86, opA, opB, result);
		write_register_y86(y86, regB, result);
		break;
	  case ANDL_FN:
		result = opA & opB;
		set_logic_op_cc(y86, result);
		write_register_y86(y86, regB, result);
		break;
	  case XORL_FN:
		result = opA ^ opB;
		set_logic_op_cc(y86, result);
		write_register_y86(y86, regB, result);
		break;
	  default:
		write_status_y86(y86, STATUS_INS);
  }
}

/*********************** Single Instruction Step ***********************/

typedef enum {
  HALT_CODE, NOP_CODE, CMOVxx_CODE, IRMOVQ_CODE, RMMOVQ_CODE, MRMOVQ_CODE,
  OP1_CODE, Jxx_CODE, CALL_CODE, RET_CODE,
  PUSHQ_CODE, POPQ_CODE } BaseOpCode;

/** Execute the next instruction of y86. Must change status of
 *  y86 to STATUS_HLT on halt, STATUS_ADR or STATUS_INS on
 *  bad address or instruction.
 */
void
step_ysim(Y86 *y86)
{
  Address pc = read_pc_y86(y86);
  Address rsp;
  Byte opCode = read_memory_byte_y86(y86, pc);
  if(read_status_y86(y86) != STATUS_AOK) return;
  Byte opCodeHigh = get_nybble(opCode, 1);
  Byte reg;
  Byte regHigh;
  Byte regLow;
  Address location;
  Word value;
  switch (opCodeHigh) {
	  case HALT_CODE:
		write_status_y86(y86, STATUS_HLT);
		break;
	  case NOP_CODE:
		write_pc_y86(y86, ++pc);
		break;
	  case CMOVxx_CODE:
		if(check_cc(y86, opCode)) {
			reg = read_memory_byte_y86(y86, pc + sizeof(Byte));
			if(read_status_y86(y86) != STATUS_AOK) return;
			regHigh = get_nybble(reg, 1);
			regLow = get_nybble(reg, 0);
			write_register_y86(y86, regLow, read_register_y86(y86, regHigh));
		}
		write_pc_y86(y86, pc + 1 + sizeof(Byte));
		break;
	  case IRMOVQ_CODE:
	    ;
		reg = read_memory_byte_y86(y86, pc + sizeof(Byte));
		if(read_status_y86(y86) != STATUS_AOK) return;
		reg = get_nybble(reg, 0);
		Word imm = read_memory_word_y86(y86, pc + 2 * sizeof(Byte));
		if(read_status_y86(y86) != STATUS_AOK) return;
		write_register_y86(y86, reg, imm);
		write_pc_y86(y86, pc + 1 + sizeof(Byte) + sizeof(Word));
		break;
	  case RMMOVQ_CODE:
		reg = read_memory_byte_y86(y86, pc + sizeof(Byte));
		if(read_status_y86(y86) != STATUS_AOK) return;
		regLow = get_nybble(reg, 0);
		location = read_register_y86(y86, regLow);
		if(read_status_y86(y86) != STATUS_AOK) return;
		regHigh = get_nybble(reg, 1);
		write_memory_word_y86(y86, location, read_register_y86(y86, regHigh));
		if(read_status_y86(y86) != STATUS_AOK) return;
		write_pc_y86(y86, pc + 1 + sizeof(Byte) + sizeof(Word));
		break;
	  case MRMOVQ_CODE:
		reg = read_memory_byte_y86(y86, pc + sizeof(Byte));
		if(read_status_y86(y86) != STATUS_AOK) return;
		regLow = get_nybble(reg, 0);
		location = read_register_y86(y86, regLow);
		value = read_memory_word_y86(y86, location);
		if(read_status_y86(y86) != STATUS_AOK) return;
		regHigh = get_nybble(reg, 1);
		write_register_y86(y86, regHigh, value);
		write_pc_y86(y86, pc + 1 + sizeof(Byte) + sizeof(Word));
		break;
	  case OP1_CODE:
		reg = read_memory_byte_y86(y86, pc + sizeof(Byte));
		if(read_status_y86(y86) != STATUS_AOK) return;
		regHigh = get_nybble(reg, 1);
		regLow = get_nybble(reg, 0);
		op1(y86, opCode, regHigh, regLow);
		write_pc_y86(y86, pc + 1 + sizeof(Byte));
		break;
	  case Jxx_CODE:
		if(check_cc(y86, opCode)) {
			reg = read_memory_byte_y86(y86, pc + sizeof(Byte));
			if(read_status_y86(y86) != STATUS_AOK) return;
			write_pc_y86(y86, reg);
		} else {
			write_pc_y86(y86, pc + 1 + sizeof(Word));
		}
		break;
	  case CALL_CODE:
		;
		Address functionAddress = read_memory_word_y86(y86, pc + sizeof(Byte));
		if(read_status_y86(y86) != STATUS_AOK) return;
		rsp = read_register_y86(y86, REG_RSP) - sizeof(Word);
		write_register_y86(y86, REG_RSP, rsp);
		write_memory_word_y86(y86, rsp, pc + sizeof(Byte) + sizeof(Word));
		if(read_status_y86(y86) != STATUS_AOK) return;
		write_pc_y86(y86, functionAddress);
		break;
	  case RET_CODE:
		rsp = read_register_y86(y86, REG_RSP);
		Address returnAddress = read_memory_word_y86(y86, rsp);
		if(read_status_y86(y86) != STATUS_AOK) return;
		rsp += sizeof(Word);
		write_register_y86(y86, REG_RSP, rsp);
		write_pc_y86(y86, returnAddress);
		break;
	  case PUSHQ_CODE:
		regHigh = get_nybble(read_memory_byte_y86(y86, pc + sizeof(Byte)), 1);
		if(read_status_y86(y86) != STATUS_AOK) return;
		rsp = read_register_y86(y86, REG_RSP) - sizeof(Word);
		write_memory_word_y86(y86, rsp, read_register_y86(y86, regHigh));
		write_register_y86(y86, REG_RSP, rsp);
		if(read_status_y86(y86) != STATUS_AOK) return;
		write_pc_y86(y86, pc + 1 + sizeof(Byte));
		break;
	  case POPQ_CODE:
		regHigh = get_nybble(read_memory_byte_y86(y86, pc + sizeof(Byte)), 1);
		if(read_status_y86(y86) != STATUS_AOK) return;
		rsp = read_register_y86(y86, REG_RSP);
		value = read_memory_word_y86(y86, rsp);
		if(read_status_y86(y86) != STATUS_AOK) return;
		write_register_y86(y86, REG_RSP, rsp + sizeof(Word));
		write_register_y86(y86, regHigh, value);
		write_pc_y86(y86, pc + 1 + sizeof(Byte));
		break;
	  default:
		write_status_y86(y86, STATUS_INS);
  }
}
