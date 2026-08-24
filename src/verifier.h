#ifndef AIRBOT_VERIFIER_H
#define AIRBOT_VERIFIER_H

#include <stdint.h>

typedef struct {
    int  passed;            /* 1 if all checks passed */
    int  total_checks;      /* Number of checks performed */
    int  failed_checks;     /* Number of failed checks */
    char messages[8][128];  /* Diagnostic messages (up to 8) */
    int  message_count;
    
    /* Analysis results */
    uint16_t instruction_count;  /* Total instructions */
    uint16_t max_stack_depth;    /* Maximum data stack depth reached */
    uint16_t max_ret_depth;      /* Maximum return stack depth */
    int      has_halt;           /* 1 if at least one HALT instruction exists */
    int      uses_spawn;         /* 1 if SPAWN opcode is used */
    int      uses_self;          /* 1 if SELF opcode is used */
    uint64_t required_caps;      /* Capabilities required by the bytecode */
} VerificationResult;

/* Function Declarations */
int verifier_analyze(const uint8_t *bytecode, uint16_t len, uint64_t granted_caps, VerificationResult *result);

#endif /* AIRBOT_VERIFIER_H */
