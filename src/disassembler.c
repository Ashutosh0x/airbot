#include "disassembler.h"
#include "vm.h"
#include <stdio.h>
#include <string.h>

void disasm_init(DisassemblerState *ds) {
    if (ds) {
        memset(ds, 0, sizeof(DisassemblerState));
    }
}

int disasm_instruction(char *buf, size_t buf_cap, uint16_t instruction, uint16_t addr) {
    if (!buf || buf_cap == 0) return -1;
    uint8_t opcode = (instruction >> 11) & 0x1F;
    
    /* Dummy switch case for basic decoding */
    switch (opcode) {
        case OP_NOP:
            snprintf(buf, buf_cap, "0x%04X: NOP", addr);
            break;
        case OP_HALT:
            snprintf(buf, buf_cap, "0x%04X: HALT", addr);
            break;
        default:
            snprintf(buf, buf_cap, "0x%04X: UNKNOWN %04X", addr, instruction);
            break;
    }
    return 0;
}

int disasm_disassemble(DisassemblerState *ds, const uint8_t *bytecode, uint16_t len) {
    if (!ds || !bytecode) return -1;
    disasm_init(ds);
    
    for (uint16_t i = 0; i < len; i += 2) {
        if (i + 1 >= len) break; /* Incomplete instruction */
        uint16_t inst = ((uint16_t)bytecode[i] << 8) | bytecode[i+1];
        
        char inst_buf[64];
        if (disasm_instruction(inst_buf, sizeof(inst_buf), inst, i / 2) == 0) {
            size_t curr_len = strlen(ds->output);
            size_t new_len = curr_len + strlen(inst_buf) + 2;
            if (new_len < sizeof(ds->output)) {
                snprintf(ds->output + curr_len, sizeof(ds->output) - curr_len, "%s\n", inst_buf);
                ds->output_len = (uint16_t)new_len;
                ds->instruction_count++;
            } else {
                return -1; /* Out of space */
            }
        }
    }
    return 0;
}
