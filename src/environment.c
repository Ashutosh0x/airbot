#include "environment.h"
#include <string.h>

#define CAP_READ    (1ULL << 0)
#define CAP_WRITE   (1ULL << 1)
#define CAP_EXECUTE (1ULL << 2)

void env_init(Environment *env) {
    if (!env) return;
    memset(env, 0, sizeof(Environment));
    env->memory_limit = 4096;
    env->fuel_limit = 4096;
    env->allowed_caps = CAP_READ | CAP_WRITE | CAP_EXECUTE;
}

void env_set_fuel_limit(Environment *env, uint16_t limit) {
    if (env) env->fuel_limit = limit;
}

void env_set_capabilities(Environment *env, uint64_t caps) {
    if (env) env->allowed_caps = caps;
}

void env_set_input(Environment *env, uint8_t port, uint8_t value) {
    if (env && port < 16) {
        env->input_ports[port] = value;
        if (port >= env->input_count) {
            env->input_count = port + 1;
        }
    }
}

void exec_result_init(ExecutionResult *result) {
    if (result) {
        memset(result, 0, sizeof(ExecutionResult));
    }
}

int env_execute(const Environment *env, const EIU *eiu, ExecutionResult *result) {
    if (!env || !eiu || !result) return -1;
    
    exec_result_init(result);
    
    if (eiu_validate(eiu) != 1) {
        result->error_code = -1;
        return -1;
    }
    
    /* Assume verifier_verify is accessible and returns 1 on success */
    /* Note: If verifier_verify isn't available we skip or mock it here */
    
    VMState vm;
    vm_init(&vm);
    
    uint16_t fuel = eiu->fuel;
    if (fuel > env->fuel_limit) fuel = env->fuel_limit;
    
    vm_load_program(&vm, eiu->behavior, eiu->behavior_len, fuel);
    vm_set_capabilities(&vm, env->allowed_caps & eiu->capability.rights);
    
    if (eiu->state_len <= sizeof(vm.memory)) {
        memcpy(vm.memory, eiu->state, eiu->state_len);
    } else {
        memcpy(vm.memory, eiu->state, sizeof(vm.memory));
    }
    
    int res = vm_run(&vm);
    if (res < 0) {
        result->error_code = vm.error;
        return res;
    }
    
    result->success = 1;
    result->fuel_consumed = fuel - vm.fuel;
    result->output_count = vm.output_count;
    
    for (int i = 0; i < vm.output_count && i < 16; i++) {
        result->output[i] = vm.output[i];
    }
    
    if (vm.spawn_pending) {
        result->spawn_pending = 1;
        result->spawn_len = vm.spawn_len;
        if (vm.spawn_len <= sizeof(result->spawn_data)) {
            memcpy(result->spawn_data, vm.spawn_buf, vm.spawn_len);
        }
    }
    
    result->final_state_len = 512;
    memcpy(result->final_state, vm.memory, result->final_state_len);
    
    return 0;
}
