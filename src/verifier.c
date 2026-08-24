#include "verifier.h"
#include "vm.h"
#include <string.h>

static void add_message(VerificationResult *result, const char *msg) {
    if (result->message_count < 8) {
        strncpy(result->messages[result->message_count], msg, 127);
        result->messages[result->message_count][127] = '\0';
        result->message_count++;
    }
}

int verifier_analyze(const uint8_t *bytecode, uint16_t len, uint64_t granted_caps, VerificationResult *result) {
    if (!result) return -1;
    
    memset(result, 0, sizeof(VerificationResult));
    result->passed = 1;
    
    result->total_checks++;
    if (len % 2 != 0 || len > 4096) {
        result->passed = 0;
        result->failed_checks++;
        add_message(result, "Invalid bytecode length");
        return -1;
    }
    
    result->instruction_count = len / 2;
    
    /* Simple check loop */
    for (uint16_t i = 0; i < result->instruction_count; i++) {
        uint16_t instr = (bytecode[i * 2] << 8) | bytecode[i * 2 + 1];
        uint8_t opcode = (instr >> 11) & 0x1F;
        uint16_t offset = instr & 0x7FF;
        
        result->total_checks++;
        if (opcode > OP_ADDI) {
            result->passed = 0;
            result->failed_checks++;
            add_message(result, "Invalid opcode");
        }
        
        if (opcode == OP_HALT) {
            result->has_halt = 1;
        } else if (opcode == OP_SPAWN) {
            result->uses_spawn = 1;
            result->required_caps |= (1ULL << 0);
        } else if (opcode == OP_SELF) {
            result->uses_self = 1;
            result->required_caps |= (1ULL << 1);
        }
        
        if (opcode == OP_JMP || opcode == OP_JZ || opcode == OP_JNZ || opcode == OP_JGT || opcode == OP_CALL) {
            result->total_checks++;
            if (offset >= result->instruction_count) {
                result->passed = 0;
                result->failed_checks++;
                add_message(result, "Jump target out of bounds");
            }
        }
    }
    
    result->total_checks++;
    if (!result->has_halt) {
        result->passed = 0;
        result->failed_checks++;
        add_message(result, "No HALT instruction found");
    }
    
    result->total_checks++;
    if ((result->required_caps & granted_caps) != result->required_caps) {
        result->passed = 0;
        result->failed_checks++;
        add_message(result, "Missing required capabilities");
    }
    
    return result->passed ? 0 : -1;
}
