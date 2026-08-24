#include "assembler.h"
#include "vm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void asm_init(AssemblerState *as) {
    if (as) {
        memset(as, 0, sizeof(AssemblerState));
    }
}

int asm_parse_register(const char *s) {
    if (!s) return -1;
    if (s[0] == 'R' || s[0] == 'r') {
        if (isdigit((unsigned char)s[1])) {
            int reg = s[1] - '0';
            if (reg >= 0 && reg <= 7) return reg;
        }
    }
    return -1;
}

int asm_parse_immediate(const char *s, uint16_t *out) {
    if (!s || !out) return -1;
    char *endptr;
    long val = strtol(s, &endptr, 0);
    if (*endptr != '\0' && !isspace((unsigned char)*endptr)) return -1;
    *out = (uint16_t)val;
    return 0;
}

int asm_lookup_label(const AssemblerState *as, const char *name, uint16_t *addr) {
    if (!as || !name || !addr) return -1;
    for (uint16_t i = 0; i < as->label_count; i++) {
        if (strcmp(as->labels[i].name, name) == 0) {
            *addr = as->labels[i].address;
            return 0;
        }
    }
    return -1;
}

int asm_assemble(AssemblerState *as, const char *source, size_t source_len) {
    if (!as || !source) return -1;
    asm_init(as);
    /* Simple stub implementation for assembling */
    /* Real logic would involve two passes, parsing ops and labels */
    (void)source_len;
    return 0;
}

int asm_assemble_file(AssemblerState *as, const char *filename) {
    if (!as || !filename) return -1;
    FILE *f = fopen(filename, "rb");
    if (!f) {
        snprintf(as->error_msg, sizeof(as->error_msg), "Could not open file");
        as->error = -1;
        return -1;
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    char *buf = malloc(size + 1);
    if (!buf) {
        fclose(f);
        return -1;
    }
    
    size_t read_len = fread(buf, 1, size, f);
    buf[read_len] = '\0';
    fclose(f);
    
    int result = asm_assemble(as, buf, read_len);
    free(buf);
    
    return result;
}
