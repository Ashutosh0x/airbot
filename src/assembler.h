#ifndef AIRBOT_ASSEMBLER_H
#define AIRBOT_ASSEMBLER_H

#include <stdint.h>
#include <stddef.h>

#define ASM_MAX_LABELS    64
#define ASM_MAX_OUTPUT    2048  /* Max output bytes */
#define ASM_MAX_LINE      256   /* Max line length */

typedef struct {
    char     name[32];
    uint16_t address;  /* Instruction index */
} AsmLabel;

typedef struct {
    uint8_t  output[ASM_MAX_OUTPUT];  /* Output bytecode */
    uint16_t output_len;               /* Output length in bytes */
    uint16_t instruction_count;        /* Number of instructions */
    uint16_t fuel;                     /* Fuel directive value */
    uint64_t capabilities;             /* Capability flags */
    AsmLabel labels[ASM_MAX_LABELS];   /* Label table */
    uint16_t label_count;
    int      error;                    /* Error code */
    char     error_msg[128];           /* Error message */
    int      error_line;               /* Line number of error */
} AssemblerState;

/* Initialize assembler state */
void asm_init(AssemblerState *as);

/* Assemble source text */
int asm_assemble(AssemblerState *as, const char *source, size_t source_len);

/* Assemble from file */
int asm_assemble_file(AssemblerState *as, const char *filename);

/* Parse register (R0-R7) */
int asm_parse_register(const char *s);

/* Parse immediate value */
int asm_parse_immediate(const char *s, uint16_t *out);

/* Lookup label */
int asm_lookup_label(const AssemblerState *as, const char *name, uint16_t *addr);

#endif /* AIRBOT_ASSEMBLER_H */
