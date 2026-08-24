#ifndef AIRBOT_ENVIRONMENT_H
#define AIRBOT_ENVIRONMENT_H

#include <stdint.h>
#include "eiu.h"
#include "vm.h"
#include "verifier.h"
#include "capability.h"

/**
 * Execution Environment: Models E = (M, I, R, C)
 */
typedef struct {
    uint32_t memory_limit;    /* Max memory bytes available */
    uint16_t fuel_limit;      /* Max fuel allowed */
    uint64_t allowed_caps;    /* Capabilities the environment permits */
    uint8_t  input_ports[16]; /* Input port values */
    uint8_t  input_count;     /* Number of available inputs */
    uint64_t timestamp;       /* Current timestamp */
    uint32_t node_id;         /* Identity of this execution node */
} Environment;

/**
 * Result of executing an EIU within an Environment
 */
typedef struct {
    int      success;         /* 1 if execution completed successfully */
    int      error_code;      /* Error code from VM or verifier */
    uint16_t fuel_consumed;   /* Fuel units used */
    uint16_t instructions_executed; /* Total instructions run */
    uint16_t output[16];      /* Output values */
    uint8_t  output_count;    /* Number of outputs */
    uint8_t  spawn_data[512]; /* Spawned successor data */
    uint16_t spawn_len;       /* Length of spawn data */
    int      spawn_pending;   /* 1 if spawn was requested */
    uint8_t  final_state[512];/* Final state of the VM memory (first 512 bytes) */
    uint16_t final_state_len;
} ExecutionResult;

/** Initialize environment with defaults */
void env_init(Environment *env);

/** Set the fuel limit for the environment */
void env_set_fuel_limit(Environment *env, uint16_t limit);

/** Set capabilities the environment permits */
void env_set_capabilities(Environment *env, uint64_t caps);

/** Set input value for a port */
void env_set_input(Environment *env, uint8_t port, uint8_t value);

/** Execute the EIU within the environment */
int env_execute(const Environment *env, const EIU *eiu, ExecutionResult *result);

/** Initialize the execution result to zero */
void exec_result_init(ExecutionResult *result);

#endif /* AIRBOT_ENVIRONMENT_H */
