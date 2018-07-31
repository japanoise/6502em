#include <stdint.h>

typedef struct {
	uint8_t A; // Accumulator
	uint8_t X; // X index
	uint8_t Y; // Y index
	uint8_t SP; // Stack pointer
	uint16_t PC; // Program counter
	uint8_t flags; // NV-BDIZC
	uint8_t *memory;
} state_6502;

state_6502 *create_6502();

void destroy_6502(state_6502*);

void reset_6502(state_6502 *);

int step_6502(state_6502*);
