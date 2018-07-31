#include <stdlib.h>
#include <stdio.h>
#include "6502.h"

#define BYTES2WORD(highByte, lowByte) (highByte << 8) | lowByte

#define UNSETCARRY(cpu) cpu->flags &= ~1
#define SETCARRY(cpu) cpu->flags |= 1
#define CARRYSET(cpu) cpu->flags & 1

#define UNSETDECIMAL(cpu) cpu->flags &= ~8
#define SETDECIMAL(cpu) cpu->flags |= 8
#define DECIMALSET(cpu) cpu->flags & 8

/* Stack behaviour gleaned from the emulator here: https://skilldrick.github.io/easy6502/ */
void stack_push(state_6502 *cpu, uint8_t data) {
	cpu->memory[BYTES2WORD(0x01, cpu->SP)] = data;
	cpu->SP--;
}

uint8_t stack_pop(state_6502 *cpu) {
	cpu->SP++;
	return cpu->memory[BYTES2WORD(0x01, cpu->SP)];
}

void adc(state_6502 *cpu, uint8_t operand) {
	uint16_t tmp = cpu->A + operand;
	UNSETCARRY(cpu);
	/* is bit 8 set? !! = pretty please give me a bool */
	cpu->flags |= !!(tmp & (1 << 8));
	cpu->A = (tmp&0xFF);
}

state_6502 *create_6502() {
	state_6502 *ret = malloc(sizeof(state_6502));
	ret->memory = malloc(0x10000);
	ret->SP = 0xFF;
	return ret;
}

void reset_6502(state_6502 *cpu) {
	/* http://forum.6502.org/viewtopic.php?p=12769&sid=f92e72d8621a30415528132c008f1127#p12769 */
	cpu->PC = BYTES2WORD(cpu->memory[0xFFFd], cpu->memory[0xFFFc]);
}

void destroy_6502(state_6502 *cpu) {
	free(cpu->memory);
	free(cpu);
}

#define RJUMP(boolexpr)						\
	cpu->PC+=2;						\
	if (boolexpr){						\
		int8_t offset = cpu->PC-1;			\
		uint16_t npc = cpu->PC + offset;		\
		if((npc&0xFF00)==(cpu->PC&0xFF00)) {		\
			cpu->PC = npc;				\
			return 3;				\
		} else {					\
			cpu->PC = npc;				\
			return 4;				\
		}						\
	}

int step_6502(state_6502 *cpu) {
	switch(cpu->memory[cpu->PC]) {
	case 0x08: // PHP
		stack_push(cpu, cpu->flags);
		cpu->PC++;
		return 2;
	case 0x18: // CLC
		UNSETCARRY(cpu);
		cpu->PC++;
		return 2;
	case 0x28: // PLP
		cpu->flags = stack_pop(cpu);
		cpu->PC++;
		return 4;
	case 0x38: // SEC
		SETCARRY(cpu);
		cpu->PC++;
		return 2;
	case 0x48: // PHA
		stack_push(cpu, cpu->A);
		cpu->PC++;
		return 3;
	case 0x68: // PLA
		cpu->A = stack_pop(cpu);
		cpu->PC++;
		return 4;
	case 0x69: // nice<M-DEL> ADC #$xx
		if (DECIMALSET(cpu)) goto unimp;
		else adc(cpu, cpu->memory[cpu->PC+1]);
		cpu->PC +=2;
		return 2;
	case 0x9A: // TXS
		cpu->SP = cpu->X;
		cpu->PC++;
		return 2;
	case 0xA9: // LDA #$xx
		cpu->A = cpu->memory[cpu->PC+1];
		cpu->PC += 2;
		return 2;
	case 0xB0: // BCS
		RJUMP(CARRYSET(cpu))
		return 2;
	case 0xBA: // TSX
		cpu->X = cpu->SP;
		cpu->PC++;
		return 2;
	case 0xD8: // CLD
		UNSETDECIMAL(cpu);
		cpu->PC++;
		return 2;
	case 0xEA: // NOP
		cpu->PC++;
		return 2;
	case 0xF8: // SED
		SETDECIMAL(cpu);
		cpu->PC++;
		return 2;
	default:
	unimp:
		fprintf(stderr, "PC: %x - Unimplemented opcode or bad state - %hhx\n", cpu->PC, cpu->memory[cpu->PC]);
		return -1;
	}
}
