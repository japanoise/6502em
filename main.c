#include <stdio.h>
#include <stdint.h>
#include <strings.h>
#include <stdlib.h>

static char *argv0;
#include "arg.h"
#include "6502.h"

void usage() {
	fprintf(stderr, "usage: %s [args]\n", argv0);
	fprintf(stderr, "\t-o offset - offset for next include argument\n");
	fprintf(stderr, "\t-b byte   - include an 8-bit byte at offset\n");
	fprintf(stderr, "\t-w word   - include a 16-bit word at offset\n");
	fprintf(stderr, "\t-f file   - include a file at offset\n");
}

int main(int argc, char *argv[]) {
	unsigned long offset = 0;
	state_6502 *cpu = create_6502();

	ARGBEGIN {
		case 'f':;
			char *farg = ARGF();
			if (farg) {
				FILE *incfile = fopen(farg, "rb");
				int c = fgetc(incfile);
				int i = 0;
				while (c != EOF)
					cpu->memory[offset+i++] = c&0xFF;
				fclose(incfile);
			} else {
				goto badargs;
			}
			break;
		case 'b':;
			char *barg = ARGF();
			if (barg) {
				unsigned long byte = strtoul(barg, NULL, 0);
				cpu->memory[offset] = byte&0xFF;
			} else {
				goto badargs;
			}
			break;
		case 'w':;
			char *warg = ARGF();
			if (warg) {
				unsigned long word = strtoul(warg, NULL, 0);
				cpu->memory[offset] = word&0xFF;
				cpu->memory[offset+1] = (word>>8)&0xFF;
			} else {
				goto badargs;
			}
			break;
		case 'o':;
			char *ofarg = ARGF();
			if (ofarg) {
				offset = strtoul(ofarg, NULL, 0);
			} else {
				goto badargs;
			}
			break;
		case 'h':
		default:
	badargs:
			destroy_6502(cpu);
			usage();
			return 1;
	} ARGEND;

	reset_6502(cpu);
	while(step_6502(cpu)>=0);

	destroy_6502(cpu);
	return 0;
}
