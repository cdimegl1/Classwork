#include "stall-sim.h"

#include "y86-util.h"

#include "errors.h"
#include "memalloc.h"

#include <assert.h>

enum {
  MAX_DATA_BUBBLES = 3,  /** max # of bubbles due to data hazards */
  JUMP_BUBBLES = 2,      /** # of bubbles for cond jump op */
  RET_BUBBLES = 3,       /** # of bubbles for return op */
  MAX_REG_WRITE = 2      /** max # of registers written per clock cycle */
};

struct StallSimStruct {
  Y86* y86;
  Register* hazards;
  int bubbles;
  int stalled;
};

void add_hazard(StallSim* stallSim, Register hazard) {
	Register* hazards = stallSim->hazards;
	hazards[5] = hazards[4];
	hazards[4] = hazards[3];
	hazards[3] = hazards[2];
	hazards[2] = hazards[1];
	hazards[1] = hazards[0];
	hazards[0] = hazard;
}

void print_hazards(StallSim* stallSim) {
	for(int i = 0; i < 6; i++) {
		printf("%d ", stallSim->hazards[i]);
	}
	printf("\t");
}

int bubbles_for_hazard(StallSim* stallSim, Register hazard) {
	for(int i = 0; i < 6; i++) {
		if(stallSim->hazards[i] == hazard)
			return (i % 2 == 0) ? (6 - i) / 2 : ((6 - i) / 2) + 1;
	}
	return 0;
}

/********************** Allocation / Deallocation **********************/

/** Create a new pipeline stall simulator for y86. */
StallSim *
new_stall_sim(Y86 *y86)
{
	StallSim* stall_sim = callocChk(1, sizeof(struct StallSimStruct));
	stall_sim->y86 = y86;
	stall_sim->hazards = mallocChk(6 * sizeof(Register));
	for(int i = 0; i < 6; i ++) {
		stall_sim->hazards[i] = -1;
	}
	stall_sim->bubbles = 4;
	return stall_sim;
}

/** Free all resources allocated by new_pipe_sim() in stallSim. */
void
free_stall_sim(StallSim *stallSim)
{
  free(stallSim->hazards);
  free(stallSim);
}


/** Apply next pipeline clock to stallSim.  Return true if
 *  processor can proceed, false if pipeline is stalled.
 *
 * The pipeline will stall under the following circumstances:
 *
 * Exactly 4 clock cycles on startup to allow the pipeline to fill up.
 *
 * Exactly 2 clock cyclies after execution of a conditional jump.
 *
 * Exactly 3 clock cycles after execution of a return.
 *
 * Upto 3 clock cycles when attempting to read a register which was
 * written by any of upto 3 preceeding instructions.
 */
bool
clock_stall_sim(StallSim *stallSim)
{
  if(stallSim->bubbles > 0) {
	  stallSim->bubbles--;
	  add_hazard(stallSim, -1);
	  add_hazard(stallSim, -1);
	  return false;
  }
  Y86* y86 = stallSim->y86;
  Address pc = read_pc_y86(y86);
  Byte fullopCode = read_memory_byte_y86(y86, pc);
  Byte opCode = get_nybble(fullopCode, 1);
  switch(opCode) {
	  case Jxx_CODE:
		if(stallSim->stalled) {
			stallSim->stalled = 0;
		} else {
			if(get_nybble(fullopCode, 0) != 0) {
				stallSim->bubbles = 2;
				stallSim->stalled = 1;
			}
		}
		break;
	  case RET_CODE:
		if(stallSim->stalled) {
			stallSim->stalled = 0;
		} else {
			stallSim->bubbles = 3;
			stallSim->stalled = 1;
		}
		break;
	  case CALL_CODE:
		if(stallSim->stalled) {
			Register writeReg = REG_RSP;
			add_hazard(stallSim, writeReg);
			stallSim->stalled = 0;
		} else {
			Register readReg = REG_RSP;
			stallSim->bubbles = bubbles_for_hazard(stallSim, readReg);
			if(stallSim->bubbles != 0)
				stallSim->stalled = 1;
		}
		break;
	  case PUSHQ_CODE:
		if(stallSim->stalled) {
			Register writeReg = REG_RSP;
			add_hazard(stallSim, writeReg);
			stallSim->stalled = 0;
		} else {
			Register readReg = read_memory_byte_y86(y86, pc + sizeof(Byte));
			readReg = get_nybble(readReg, 1);
			stallSim->bubbles = bubbles_for_hazard(stallSim, readReg);
			stallSim->stalled = 1;
		}
		break;
	  case POPQ_CODE :
		if(stallSim->stalled) {
			Register writeReg = read_memory_byte_y86(y86, pc + sizeof(Byte));
			writeReg = get_nybble(writeReg, 1);
			add_hazard(stallSim, writeReg);
			add_hazard(stallSim, REG_RSP);
			stallSim->stalled = 0;
		} else {
			stallSim->bubbles = bubbles_for_hazard(stallSim, REG_RSP);
			stallSim->stalled = 1;
		}
		break;
	  case CMOVxx_CODE:
	  {
		Register registers = read_memory_byte_y86(y86, pc + sizeof(Byte));
		if(stallSim->stalled) {
			Register writeReg = get_nybble(registers, 0);
			add_hazard(stallSim, writeReg);
			stallSim->stalled = 0;
		} else {
			Register readReg = get_nybble(registers, 1);
			stallSim->bubbles = bubbles_for_hazard(stallSim, readReg);
		}
		break;
	  }
	  case IRMOVQ_CODE:
	  {
		Register registers = read_memory_byte_y86(y86, pc + sizeof(Byte));
		Register writeReg = get_nybble(registers, 0);
		add_hazard(stallSim, -1);
		add_hazard(stallSim, writeReg);
		break;
	  }
	  case MRMOVQ_CODE:
	  {
		Register registers = read_memory_byte_y86(y86, pc + sizeof(Byte));
		if(stallSim->stalled) {
			Register writeReg = get_nybble(registers, 1);
			add_hazard(stallSim, writeReg);
			add_hazard(stallSim, -1);
			stallSim->stalled = 0;
		} else {
			Register readReg = get_nybble(registers, 0);
			stallSim->bubbles = bubbles_for_hazard(stallSim, readReg);
			if(stallSim->bubbles != 0)
				stallSim->stalled = 1;
			else {
				Register writeReg = get_nybble(registers, 1);
				add_hazard(stallSim, writeReg);
				add_hazard(stallSim, -1);
			}
		}
		break;
	  }
	  case OP1_CODE:
	  {
		Register registers = read_memory_byte_y86(y86, pc + sizeof(Byte));
		Register writeReg = get_nybble(registers, 0);
		Register readReg = get_nybble(registers, 1);
		if(stallSim->stalled) {
			add_hazard(stallSim, writeReg);
			add_hazard(stallSim, -1);
			stallSim->stalled = 0;
		} else {
			int bubble1, bubble2;
			bubble1 = bubbles_for_hazard(stallSim, readReg);
			bubble2 = bubbles_for_hazard(stallSim, writeReg);
			stallSim->bubbles = bubble1 > bubble2 ? bubble1 : bubble2;
			if(stallSim->bubbles != 0)
				stallSim->stalled = 1;
			else {
				add_hazard(stallSim, writeReg);
				add_hazard(stallSim, -1);
			}
		}
		break;
	  }
	  case NOP_CODE:
		stallSim->bubbles--;
		add_hazard(stallSim, -1);
		add_hazard(stallSim, -1);
		return true;
  }
  if(stallSim->bubbles > 0) {
	  stallSim->bubbles--;
	  add_hazard(stallSim, -1);
	  add_hazard(stallSim, -1);
	  return false;
  }
  return true;
}
