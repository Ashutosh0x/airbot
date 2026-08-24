#ifndef AIRBOT_VM_H
#define AIRBOT_VM_H

#include <stdint.h>

/* Opcodes */
#define OP_NOP   0x00
#define OP_HALT  0x01
#define OP_PUSH  0x02
#define OP_POP   0x03
#define OP_LOAD  0x04
#define OP_STORE 0x05
#define OP_LDI   0x06
#define OP_ADD   0x07
#define OP_SUB   0x08
#define OP_AND   0x09
#define OP_OR    0x0A
#define OP_XOR   0x0B
#define OP_NOT   0x0C
#define OP_SHL   0x0D
#define OP_SHR   0x0E
#define OP_CMP   0x0F
#define OP_JMP   0x10
#define OP_JZ    0x11
#define OP_JNZ   0x12
#define OP_JGT   0x13
#define OP_CALL  0x14
#define OP_RET   0x15
#define OP_EMIT  0x16
#define OP_READ  0x17
#define OP_BFLIP 0x18
#define OP_BREAD 0x19
#define OP_SELF  0x1A
#define OP_SPAWN 0x1B
#define OP_CCHK  0x1C
#define OP_FUEL  0x1D
#define OP_MOV   0x1E
#define OP_ADDI  0x1F

/* Error Codes */
#define VM_ERR_NONE             0
#define VM_ERR_OUT_OF_FUEL    -10
#define VM_ERR_STACK_OVERFLOW -11
#define VM_ERR_STACK_UNDERFLOW -12
#define VM_ERR_BOUNDS         -13
#define VM_ERR_INVALID_OPCODE -14
#define VM_ERR_NO_CAPABILITY  -15
#define VM_ERR_HALTED         -16

typedef struct {
    uint16_t regs[8];           /* R0-R7 */
    uint16_t data_stack[64];    /* Data stack */
    uint8_t  dsp;               /* Data stack pointer */
    uint16_t ret_stack[32];     /* Return stack */
    uint8_t  rsp;               /* Return stack pointer */
    uint8_t  memory[4096];      /* Main memory */
    uint16_t pc;                /* Program counter (in instruction units) */
    uint16_t fuel;              /* Remaining fuel */
    uint8_t  zero_flag;         /* Zero flag */
    uint8_t  carry_flag;        /* Carry/borrow flag */
    uint8_t  greater_flag;      /* Greater-than flag */
    uint16_t output[16];        /* Output buffer */
    uint8_t  output_count;      /* Number of outputs */
    uint8_t  halted;            /* 1 if HALT executed */
    int      error;             /* Error code (0 = none) */
    
    const uint8_t *bytecode;    /* Pointer to bytecode (not owned) */
    uint16_t bytecode_len;      /* Bytecode length in bytes */
    
    uint64_t granted_caps;      /* Capabilities granted to this VM */
    
    uint8_t  spawn_buf[512];    /* Buffer for spawned EIU data */
    uint16_t spawn_len;         /* Length of spawned data */
    uint8_t  spawn_pending;     /* 1 if a spawn was requested */
} VMState;

/* Function Declarations */
void vm_init(VMState *vm);
void vm_load_program(VMState *vm, const uint8_t *bytecode, uint16_t len, uint16_t fuel);
void vm_set_capabilities(VMState *vm, uint64_t caps);
int vm_step(VMState *vm);
int vm_run(VMState *vm);
int vm_is_halted(const VMState *vm);
uint16_t vm_get_register(const VMState *vm, uint8_t reg);
void vm_set_register(VMState *vm, uint8_t reg, uint16_t value);
uint16_t vm_encode_instruction(uint8_t opcode, uint8_t a, uint8_t b, uint8_t c);
uint16_t vm_encode_immediate(uint8_t opcode, uint8_t reg, uint8_t imm);
uint16_t vm_encode_jump(uint8_t opcode, uint16_t offset);
const char* vm_opcode_name(uint8_t opcode);

#endif /* AIRBOT_VM_H */
