#ifndef AIRBOT_DISASSEMBLER_H
#define AIRBOT_DISASSEMBLER_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    char     output[4096];  /* Output text buffer */
    uint16_t output_len;    /* Length of text */
    uint16_t instruction_count;
} DisassemblerState;

/* Initialize disassembler state */
void disasm_init(DisassemblerState *ds);

/* Disassemble bytecode array */
int disasm_disassemble(DisassemblerState *ds, const uint8_t *bytecode, uint16_t len);

/* Disassemble a single instruction */
int disasm_instruction(char *buf, size_t buf_cap, uint16_t instruction, uint16_t addr);

#endif /* AIRBOT_DISASSEMBLER_H */
