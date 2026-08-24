#include "vm.h"
#include <string.h>

void vm_init(VMState *vm) {
    if (!vm) return;
    memset(vm, 0, sizeof(VMState));
}

void vm_load_program(VMState *vm, const uint8_t *bytecode, uint16_t len, uint16_t fuel) {
    if (!vm) return;
    vm->bytecode = bytecode;
    vm->bytecode_len = len;
    vm->fuel = fuel;
    /* Initialize memory with bytecode if needed, but we execute directly from bytecode pointer.
       Let's copy up to 4096 bytes into main memory for read/write access if that's intended,
       though execution fetches from bytecode pointer. */
    uint16_t copy_len = len > 4096 ? 4096 : len;
    if (bytecode && copy_len > 0) {
        memcpy(vm->memory, bytecode, copy_len);
    }
}

void vm_set_capabilities(VMState *vm, uint64_t caps) {
    if (!vm) return;
    vm->granted_caps = caps;
}

uint16_t vm_get_register(const VMState *vm, uint8_t reg) {
    if (!vm || reg > 7) return 0;
    return vm->regs[reg];
}

void vm_set_register(VMState *vm, uint8_t reg, uint16_t value) {
    if (!vm || reg > 7) return;
    vm->regs[reg] = value;
}

uint16_t vm_encode_instruction(uint8_t opcode, uint8_t a, uint8_t b, uint8_t c) {
    return ((opcode & 0x1F) << 11) | ((a & 0x7) << 8) | ((b & 0x7) << 5) | ((c & 0x7) << 2);
}

uint16_t vm_encode_immediate(uint8_t opcode, uint8_t reg, uint8_t imm) {
    return ((opcode & 0x1F) << 11) | ((reg & 0x7) << 8) | (imm & 0xFF);
}

uint16_t vm_encode_jump(uint8_t opcode, uint16_t offset) {
    return ((opcode & 0x1F) << 11) | (offset & 0x7FF);
}

const char* vm_opcode_name(uint8_t opcode) {
    switch (opcode) {
        case OP_NOP: return "NOP";
        case OP_HALT: return "HALT";
        case OP_PUSH: return "PUSH";
        case OP_POP: return "POP";
        case OP_LOAD: return "LOAD";
        case OP_STORE: return "STORE";
        case OP_LDI: return "LDI";
        case OP_ADD: return "ADD";
        case OP_SUB: return "SUB";
        case OP_AND: return "AND";
        case OP_OR: return "OR";
        case OP_XOR: return "XOR";
        case OP_NOT: return "NOT";
        case OP_SHL: return "SHL";
        case OP_SHR: return "SHR";
        case OP_CMP: return "CMP";
        case OP_JMP: return "JMP";
        case OP_JZ: return "JZ";
        case OP_JNZ: return "JNZ";
        case OP_JGT: return "JGT";
        case OP_CALL: return "CALL";
        case OP_RET: return "RET";
        case OP_EMIT: return "EMIT";
        case OP_READ: return "READ";
        case OP_BFLIP: return "BFLIP";
        case OP_BREAD: return "BREAD";
        case OP_SELF: return "SELF";
        case OP_SPAWN: return "SPAWN";
        case OP_CCHK: return "CCHK";
        case OP_FUEL: return "FUEL";
        case OP_MOV: return "MOV";
        case OP_ADDI: return "ADDI";
        default: return "UNKNOWN";
    }
}

int vm_step(VMState *vm) {
    if (!vm || vm->error != VM_ERR_NONE) return vm ? vm->error : -1;
    if (vm->halted) return VM_ERR_HALTED;
    
    if (vm->fuel == 0) {
        vm->error = VM_ERR_OUT_OF_FUEL;
        return VM_ERR_OUT_OF_FUEL;
    }
    vm->fuel--;

    if (vm->pc * 2 + 1 >= vm->bytecode_len) {
        vm->error = VM_ERR_BOUNDS;
        return VM_ERR_BOUNDS;
    }

    uint16_t instr = (vm->bytecode[vm->pc * 2] << 8) | vm->bytecode[vm->pc * 2 + 1];
    uint8_t opcode = (instr >> 11) & 0x1F;
    uint8_t regA = (instr >> 8) & 0x7;
    uint8_t regB = (instr >> 5) & 0x7;
    uint8_t regC = (instr >> 2) & 0x7;
    uint8_t imm8 = instr & 0xFF;
    uint16_t offset = instr & 0x7FF;

    vm->pc++;

    switch (opcode) {
        case OP_NOP:
            break;
        case OP_HALT:
            vm->halted = 1;
            break;
        case OP_PUSH:
            if (vm->dsp >= 64) { vm->error = VM_ERR_STACK_OVERFLOW; return vm->error; }
            vm->data_stack[vm->dsp++] = vm->regs[regA];
            break;
        case OP_POP:
            if (vm->dsp == 0) { vm->error = VM_ERR_STACK_UNDERFLOW; return vm->error; }
            vm->regs[regA] = vm->data_stack[--vm->dsp];
            break;
        case OP_LOAD:
            if (vm->regs[regB] >= 4096) { vm->error = VM_ERR_BOUNDS; return vm->error; }
            vm->regs[regA] = vm->memory[vm->regs[regB]];
            break;
        case OP_STORE:
            if (vm->regs[regB] >= 4096) { vm->error = VM_ERR_BOUNDS; return vm->error; }
            vm->memory[vm->regs[regB]] = vm->regs[regA] & 0xFF;
            break;
        case OP_LDI:
            vm->regs[regA] = imm8;
            break;
        case OP_ADD: {
            uint32_t res = (uint32_t)vm->regs[regB] + vm->regs[regC];
            vm->regs[regA] = (uint16_t)res;
            vm->carry_flag = (res > 0xFFFF) ? 1 : 0;
            break;
        }
        case OP_SUB: {
            uint32_t res = (uint32_t)vm->regs[regB] - vm->regs[regC];
            vm->regs[regA] = (uint16_t)res;
            vm->carry_flag = (vm->regs[regB] < vm->regs[regC]) ? 1 : 0;
            break;
        }
        case OP_AND:
            vm->regs[regA] = vm->regs[regB] & vm->regs[regC];
            break;
        case OP_OR:
            vm->regs[regA] = vm->regs[regB] | vm->regs[regC];
            break;
        case OP_XOR:
            vm->regs[regA] = vm->regs[regB] ^ vm->regs[regC];
            break;
        case OP_NOT:
            vm->regs[regA] = ~vm->regs[regB];
            break;
        case OP_SHL:
            vm->regs[regA] = vm->regs[regB] << (vm->regs[regC] & 0xF);
            break;
        case OP_SHR:
            vm->regs[regA] = vm->regs[regB] >> (vm->regs[regC] & 0xF);
            break;
        case OP_CMP:
            vm->zero_flag = (vm->regs[regA] == vm->regs[regB]);
            vm->greater_flag = (vm->regs[regA] > vm->regs[regB]);
            break;
        case OP_JMP:
            vm->pc = offset;
            break;
        case OP_JZ:
            if (vm->zero_flag) vm->pc = offset;
            break;
        case OP_JNZ:
            if (!vm->zero_flag) vm->pc = offset;
            break;
        case OP_JGT:
            if (vm->greater_flag) vm->pc = offset;
            break;
        case OP_CALL:
            if (vm->rsp >= 32) { vm->error = VM_ERR_STACK_OVERFLOW; return vm->error; }
            vm->ret_stack[vm->rsp++] = vm->pc;
            vm->pc = offset;
            break;
        case OP_RET:
            if (vm->rsp == 0) { vm->error = VM_ERR_STACK_UNDERFLOW; return vm->error; }
            vm->pc = vm->ret_stack[--vm->rsp];
            break;
        case OP_EMIT:
            if (vm->output_count < 16) {
                vm->output[vm->output_count++] = vm->regs[regA];
            }
            break;
        case OP_READ:
            vm->regs[regA] = 0; /* No input support in this basic version */
            break;
        case OP_BFLIP: {
            uint16_t addr = vm->regs[regA];
            uint8_t bit = vm->regs[regB] & 7;
            if (addr >= 4096) { vm->error = VM_ERR_BOUNDS; return vm->error; }
            vm->memory[addr] ^= (1 << bit);
            break;
        }
        case OP_BREAD: {
            uint16_t addr = vm->regs[regA];
            uint8_t bit = vm->regs[regB] & 7;
            if (addr >= 4096) { vm->error = VM_ERR_BOUNDS; return vm->error; }
            uint8_t val = (vm->memory[addr] >> bit) & 1;
            if (vm->dsp >= 64) { vm->error = VM_ERR_STACK_OVERFLOW; return vm->error; }
            vm->data_stack[vm->dsp++] = val;
            break;
        }
        case OP_SELF:
            if (!(vm->granted_caps & (1ULL << 1))) { /* CAP_SELF_READ assumed bit 1 */
                vm->error = VM_ERR_NO_CAPABILITY;
                return vm->error;
            }
            if (imm8 >= vm->bytecode_len) { vm->error = VM_ERR_BOUNDS; return vm->error; }
            vm->regs[regA] = vm->bytecode[imm8];
            break;
        case OP_SPAWN: {
            if (!(vm->granted_caps & (1ULL << 0))) { /* CAP_SPAWN assumed bit 0 */
                vm->error = VM_ERR_NO_CAPABILITY;
                return vm->error;
            }
            uint16_t start = vm->regs[regA];
            uint16_t sz = vm->regs[regB];
            if ((uint32_t)start + sz > 4096 || sz > 512) {
                vm->error = VM_ERR_BOUNDS;
                return vm->error;
            }
            memcpy(vm->spawn_buf, &vm->memory[start], sz);
            vm->spawn_len = sz;
            vm->spawn_pending = 1;
            break;
        }
        case OP_CCHK:
            vm->zero_flag = ((vm->granted_caps & (1ULL << imm8)) != 0) ? 1 : 0;
            break;
        case OP_FUEL:
            vm->regs[regA] = vm->fuel;
            break;
        case OP_MOV:
            vm->regs[regA] = vm->regs[regB];
            break;
        case OP_ADDI:
            vm->regs[regA] += imm8;
            break;
        default:
            vm->error = VM_ERR_INVALID_OPCODE;
            return VM_ERR_INVALID_OPCODE;
    }
    
    return VM_ERR_NONE;
}

int vm_run(VMState *vm) {
    if (!vm) return -1;
    while (!vm->halted && vm->error == VM_ERR_NONE) {
        int res = vm_step(vm);
        if (res != VM_ERR_NONE) return res;
    }
    return vm->error;
}

int vm_is_halted(const VMState *vm) {
    return vm ? vm->halted : 1;
}
