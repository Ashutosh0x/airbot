/*
 * Airbot — Executable Information System
 * experiments.c — Hypothesis Testing Framework Implementation
 *
 * Pure C99. Tests H1 (Encoded Behavior), H2 (State Evolution),
 * H3 (Constrained Replication) using the Airbot VM and EIU system.
 */

#include "experiments.h"
#include "assembler.h"
#include "blake3.h"
#include "bitstream.h"
#include <stdio.h>
#include <string.h>

/* ═══════════════════════════════════════════════════════════════
 * Helper: Create a behavioral fingerprint from execution result
 * ═══════════════════════════════════════════════════════════════ */

typedef struct {
    uint16_t outputs[16];
    uint8_t  output_count;
    uint16_t fuel_consumed;
    uint8_t  halted;
    uint8_t  error;
} BehaviorFingerprint;

static int fingerprints_equal(const BehaviorFingerprint *a,
                               const BehaviorFingerprint *b) {
    if (a->output_count != b->output_count) return 0;
    if (a->halted != b->halted) return 0;
    if (a->error != b->error) return 0;
    for (int i = 0; i < a->output_count; i++) {
        if (a->outputs[i] != b->outputs[i]) return 0;
    }
    return 1;
}

/* ═══════════════════════════════════════════════════════════════
 * H1: Encoded Behavior
 *
 * Generate distinct EIUs with varying bytecode patterns.
 * Execute each and collect behavioral fingerprints.
 * Count unique behaviors.
 * ═══════════════════════════════════════════════════════════════ */

int experiment_h1(uint16_t num_units, uint16_t min_unique,
                  ExperimentResult *result) {
    memset(result, 0, sizeof(*result));
    result->hypothesis[0] = 'H';
    result->hypothesis[1] = '1';
    result->hypothesis[2] = '\0';

    if (num_units > 256) num_units = 256;

    BehaviorFingerprint fingerprints[256];
    uint16_t unique_count = 0;
    uint16_t executed = 0;

    Environment env;
    env_init(&env);

    for (uint16_t i = 0; i < num_units; i++) {
        /* Generate a simple EIU with varying immediate values */
        EIU eiu;
        eiu_init(&eiu);
        eiu_set_fuel(&eiu, 200);

        /* Create bytecode: LDI R0, i; LDI R1, (i*3+7)%256; ADD R2,R0,R1; EMIT R2; HALT */
        uint8_t code[10];
        uint16_t instr;
        uint8_t val1 = (uint8_t)(i & 0xFF);
        uint8_t val2 = (uint8_t)((i * 3 + 7) & 0xFF);

        /* LDI R0, val1 */
        instr = vm_encode_immediate(OP_LDI, 0, val1);
        code[0] = (uint8_t)(instr >> 8);
        code[1] = (uint8_t)(instr & 0xFF);

        /* LDI R1, val2 */
        instr = vm_encode_immediate(OP_LDI, 1, val2);
        code[2] = (uint8_t)(instr >> 8);
        code[3] = (uint8_t)(instr & 0xFF);

        /* ADD R2, R0, R1 */
        instr = vm_encode_instruction(OP_ADD, 2, 0, 1);
        code[4] = (uint8_t)(instr >> 8);
        code[5] = (uint8_t)(instr & 0xFF);

        /* EMIT R2 */
        instr = vm_encode_instruction(OP_EMIT, 2, 0, 0);
        code[6] = (uint8_t)(instr >> 8);
        code[7] = (uint8_t)(instr & 0xFF);

        /* HALT */
        instr = vm_encode_jump(OP_HALT, 0);
        code[8] = (uint8_t)(instr >> 8);
        code[9] = (uint8_t)(instr & 0xFF);

        eiu_set_behavior(&eiu, code, 10);

        /* Execute */
        ExecutionResult exec;
        exec_result_init(&exec);
        int rc = env_execute(&env, &eiu, &exec);

        /* Record fingerprint */
        BehaviorFingerprint *fp = &fingerprints[executed];
        memset(fp, 0, sizeof(*fp));
        fp->output_count = exec.output_count;
        for (int j = 0; j < exec.output_count; j++) {
            fp->outputs[j] = exec.output[j];
        }
        fp->fuel_consumed = exec.fuel_consumed;
        fp->halted = (uint8_t)exec.success;
        fp->error = (uint8_t)(rc != 0 ? 1 : 0);

        /* Check if this fingerprint is unique */
        int is_unique = 1;
        for (uint16_t k = 0; k < executed; k++) {
            if (fingerprints_equal(fp, &fingerprints[k])) {
                is_unique = 0;
                break;
            }
        }
        if (is_unique) unique_count++;

        executed++;
    }

    result->total_tests = executed;
    result->passed_tests = unique_count;
    result->failed_tests = executed - unique_count;
    result->score = (double)unique_count / (double)executed;
    result->passed = (unique_count >= min_unique) ? 1 : 0;

    snprintf(result->summary, sizeof(result->summary),
             "H1 Encoded Behavior: %u EIUs generated, %u unique behaviors "
             "(%.1f%%). Target: %u. %s",
             executed, unique_count, result->score * 100.0,
             min_unique, result->passed ? "PASSED" : "FAILED");

    return AIRBOT_OK;
}

/* ═══════════════════════════════════════════════════════════════
 * H2: State Evolution
 *
 * Create an EIU that modifies its state each execution.
 * Run for N steps, track state hashes, detect cycles.
 * ═══════════════════════════════════════════════════════════════ */

int experiment_h2(uint16_t steps, uint16_t min_distinct,
                  ExperimentResult *result) {
    memset(result, 0, sizeof(*result));
    result->hypothesis[0] = 'H';
    result->hypothesis[1] = '2';
    result->hypothesis[2] = '\0';

    /* Create a counter EIU:
     * LOAD R0, R7      ; load state byte from mem[R7] (R7=0 initially)
     * ADDI R0, 1       ; increment
     * STORE R0, R7     ; store back
     * EMIT R0          ; output current value
     * HALT             */
    EIU eiu;
    eiu_init(&eiu);
    eiu_set_fuel(&eiu, 100);

    uint8_t code[10];
    uint16_t instr;

    /* LOAD R0, R7 (R7 points to state area at addr 0) */
    instr = vm_encode_instruction(OP_LOAD, 0, 7, 0);
    code[0] = (uint8_t)(instr >> 8);
    code[1] = (uint8_t)(instr & 0xFF);

    /* ADDI R0, 1 */
    instr = vm_encode_immediate(OP_ADDI, 0, 1);
    code[2] = (uint8_t)(instr >> 8);
    code[3] = (uint8_t)(instr & 0xFF);

    /* STORE R0, R7 */
    instr = vm_encode_instruction(OP_STORE, 0, 7, 0);
    code[4] = (uint8_t)(instr >> 8);
    code[5] = (uint8_t)(instr & 0xFF);

    /* EMIT R0 */
    instr = vm_encode_instruction(OP_EMIT, 0, 0, 0);
    code[6] = (uint8_t)(instr >> 8);
    code[7] = (uint8_t)(instr & 0xFF);

    /* HALT */
    instr = vm_encode_jump(OP_HALT, 0);
    code[8] = (uint8_t)(instr >> 8);
    code[9] = (uint8_t)(instr & 0xFF);

    eiu_set_behavior(&eiu, code, 10);

    /* Initialize state to 1 byte = 0 */
    uint8_t initial_state[1] = {0};
    eiu_set_state(&eiu, initial_state, 1);

    /* Run evolution */
    Environment env;
    env_init(&env);

    EvolutionTrace trace;
    trace_init(&trace);

    int rc = state_evolve(&eiu, &env, steps, &trace);

    /* Count distinct states */
    uint16_t distinct = 0;
    for (uint16_t i = 0; i < trace.length; i++) {
        int is_unique = 1;
        for (uint16_t j = 0; j < i; j++) {
            if (memcmp(trace.entries[i].state_hash,
                       trace.entries[j].state_hash, 32) == 0) {
                is_unique = 0;
                break;
            }
        }
        if (is_unique) distinct++;
    }

    /* Detect cycles */
    trace_detect_fixed_point(&trace);
    trace_detect_cycle(&trace);

    result->total_tests = trace.length;
    result->passed_tests = distinct;
    result->failed_tests = 0;
    result->score = (trace.length > 0)
                    ? (double)distinct / (double)trace.length
                    : 0.0;
    result->passed = (distinct >= min_distinct) ? 1 : 0;

    snprintf(result->summary, sizeof(result->summary),
             "H2 State Evolution: %u steps, %u distinct states (%.1f%%). "
             "Fixed point: %s (step %d). Cycle: %s (start=%d, len=%d). "
             "Target: %u distinct. %s",
             trace.length, distinct, result->score * 100.0,
             trace.fixed_point >= 0 ? "YES" : "NO", trace.fixed_point,
             trace.cycle_length > 0 ? "YES" : "NO",
             trace.cycle_start, trace.cycle_length,
             min_distinct, result->passed ? "PASSED" : "FAILED");

    (void)rc;
    return AIRBOT_OK;
}

/* ═══════════════════════════════════════════════════════════════
 * H3: Constrained Replication
 *
 * Create a SPAWN-capable EIU. Test:
 *   - Authorized replication succeeds
 *   - Unauthorized replication fails
 *   - All replicas are valid
 * ═══════════════════════════════════════════════════════════════ */

int experiment_h3(uint16_t replica_count, ExperimentResult *result) {
    memset(result, 0, sizeof(*result));
    result->hypothesis[0] = 'H';
    result->hypothesis[1] = '3';
    result->hypothesis[2] = '\0';

    uint32_t tests_passed = 0;
    uint32_t tests_total = 0;

    /* Test 1: Authorized replication */
    {
        tests_total++;
        EIU eiu;
        eiu_init(&eiu);
        eiu_set_fuel(&eiu, 500);

        /* Simple bytecode: HALT */
        uint8_t code[2];
        uint16_t instr = vm_encode_jump(OP_HALT, 0);
        code[0] = (uint8_t)(instr >> 8);
        code[1] = (uint8_t)(instr & 0xFF);
        eiu_set_behavior(&eiu, code, 2);

        /* Set some state */
        uint8_t state[4] = {0x00, 0x01, 0x02, 0x03};
        eiu_set_state(&eiu, state, 4);

        /* Grant replication capability */
        Capability cap;
        cap_init(&cap, CAP_READ | CAP_EXECUTE | CAP_REPLICATE);
        eiu_set_capability(&eiu, &cap);

        /* Set up environment that allows replication */
        Environment env;
        env_init(&env);
        env_set_capabilities(&env, CAP_ALL);

        /* Attempt replication */
        ReplicationResult repl;
        repl_result_init(&repl);
        int rc = repl_execute(&eiu, &env,
                              replica_count > MAX_REPLICAS
                                  ? MAX_REPLICAS
                                  : replica_count,
                              &repl);

        if (rc == AIRBOT_OK && repl.authorized && repl.count > 0
            && repl.all_valid) {
            tests_passed++;
        }
    }

    /* Test 2: Unauthorized replication (no CAP_REPLICATE) */
    {
        tests_total++;
        EIU eiu;
        eiu_init(&eiu);
        eiu_set_fuel(&eiu, 500);

        uint8_t code[2];
        uint16_t instr = vm_encode_jump(OP_HALT, 0);
        code[0] = (uint8_t)(instr >> 8);
        code[1] = (uint8_t)(instr & 0xFF);
        eiu_set_behavior(&eiu, code, 2);

        /* NO replication capability */
        Capability cap;
        cap_init(&cap, CAP_READ | CAP_EXECUTE);
        eiu_set_capability(&eiu, &cap);

        Environment env;
        env_init(&env);

        ReplicationResult repl;
        repl_result_init(&repl);
        int rc = repl_execute(&eiu, &env, 5, &repl);

        /* Should fail — unauthorized */
        if (!repl.authorized && repl.count == 0) {
            tests_passed++;
        }
        (void)rc;
    }

    /* Test 3: Replica validity */
    {
        tests_total++;
        EIU eiu;
        eiu_init(&eiu);
        eiu_set_fuel(&eiu, 500);

        uint8_t code[2];
        uint16_t instr = vm_encode_jump(OP_HALT, 0);
        code[0] = (uint8_t)(instr >> 8);
        code[1] = (uint8_t)(instr & 0xFF);
        eiu_set_behavior(&eiu, code, 2);

        uint8_t state[2] = {0x00, 0x00};
        eiu_set_state(&eiu, state, 2);

        Capability cap;
        cap_init(&cap, CAP_READ | CAP_EXECUTE | CAP_REPLICATE);
        eiu_set_capability(&eiu, &cap);

        Environment env;
        env_init(&env);
        env_set_capabilities(&env, CAP_ALL);

        ReplicationResult repl;
        repl_result_init(&repl);
        repl_execute(&eiu, &env, 5, &repl);

        if (repl_validate_all(&repl)) {
            tests_passed++;
        }
    }

    /* Test 4: Replicas have mutated state (not identical copies) */
    {
        tests_total++;
        EIU eiu;
        eiu_init(&eiu);
        eiu_set_fuel(&eiu, 500);

        uint8_t code[2];
        uint16_t instr = vm_encode_jump(OP_HALT, 0);
        code[0] = (uint8_t)(instr >> 8);
        code[1] = (uint8_t)(instr & 0xFF);
        eiu_set_behavior(&eiu, code, 2);

        uint8_t state[2] = {0x00, 0x00};
        eiu_set_state(&eiu, state, 2);

        Capability cap;
        cap_init(&cap, CAP_READ | CAP_EXECUTE | CAP_REPLICATE);
        eiu_set_capability(&eiu, &cap);

        Environment env;
        env_init(&env);
        env_set_capabilities(&env, CAP_ALL);

        ReplicationResult repl;
        repl_result_init(&repl);
        repl_execute(&eiu, &env, 3, &repl);

        /* Check replicas differ from each other */
        int all_different = 1;
        for (uint16_t i = 0; i < repl.count && i + 1 < repl.count; i++) {
            if (eiu_compare(&repl.replicas[i], &repl.replicas[i + 1]) == 0) {
                all_different = 0;
                break;
            }
        }
        if (all_different && repl.count >= 2) {
            tests_passed++;
        }
    }

    result->total_tests = tests_total;
    result->passed_tests = tests_passed;
    result->failed_tests = tests_total - tests_passed;
    result->score = (tests_total > 0)
                    ? (double)tests_passed / (double)tests_total
                    : 0.0;
    result->passed = (tests_passed == tests_total) ? 1 : 0;

    snprintf(result->summary, sizeof(result->summary),
             "H3 Constrained Replication: %u/%u tests passed (%.0f%%). %s",
             tests_passed, tests_total, result->score * 100.0,
             result->passed ? "PASSED" : "FAILED");

    return AIRBOT_OK;
}

/* ═══════════════════════════════════════════════════════════════
 * Run All Experiments
 * ═══════════════════════════════════════════════════════════════ */

int experiments_run_all(void) {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║         AIRBOT HYPOTHESIS TESTING FRAMEWORK             ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n\n");

    ExperimentResult r;
    int all_passed = 1;

    /* H1: Encoded Behavior */
    printf("─── H1: Encoded Behavior ────────────────────────────────\n");
    experiment_h1(256, 32, &r);
    experiment_print_result(&r);
    if (!r.passed) all_passed = 0;

    /* H2: State Evolution */
    printf("─── H2: State Evolution ─────────────────────────────────\n");
    experiment_h2(100, 10, &r);
    experiment_print_result(&r);
    if (!r.passed) all_passed = 0;

    /* H3: Constrained Replication */
    printf("─── H3: Constrained Replication ─────────────────────────\n");
    experiment_h3(5, &r);
    experiment_print_result(&r);
    if (!r.passed) all_passed = 0;

    printf("\n════════════════════════════════════════════════════════\n");
    printf("  OVERALL: %s\n", all_passed
        ? "ALL IMPLEMENTED HYPOTHESES PASSED THE PROTOTYPE TEST SUITE"
        : "SOME HYPOTHESES DID NOT PASS THE PROTOTYPE TEST SUITE");
    printf("════════════════════════════════════════════════════════\n\n");

    return all_passed ? AIRBOT_OK : -1;
}

void experiment_print_result(const ExperimentResult *result) {
    printf("  Result: %s\n", result->passed ? "[PASS]" : "[FAIL]");
    printf("  %s\n", result->summary);
    printf("  Score:  %.1f%%\n\n", result->score * 100.0);
}
