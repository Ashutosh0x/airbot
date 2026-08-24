/*
 * Airbot — Executable Information System
 * experiments.h — Hypothesis Testing Framework
 *
 * Validates the three core hypotheses:
 *   H1: Encoded Behavior (different bits → different behaviors)
 *   H2: State Evolution (valid state transitions U_t → U_{t+1})
 *   H3: Constrained Replication (authorized successor generation)
 */

#ifndef AIRBOT_EXPERIMENTS_H
#define AIRBOT_EXPERIMENTS_H

#include <stdint.h>
#include "eiu.h"
#include "vm.h"
#include "environment.h"
#include "state.h"
#include "replicator.h"
#include "metrics.h"

/* ─── Experiment Result ─────────────────────────────────────── */

typedef struct {
    int      passed;                /* 1 if hypothesis validated */
    char     hypothesis[4];         /* "H1", "H2", or "H3" */
    char     summary[512];          /* Human-readable summary */
    uint32_t total_tests;           /* Number of test cases run */
    uint32_t passed_tests;          /* Number that passed */
    uint32_t failed_tests;          /* Number that failed */
    double   score;                 /* 0.0 to 1.0 success ratio */
} ExperimentResult;

/* ─── H1: Encoded Behavior ──────────────────────────────────── */

/*
 * Generate N distinct EIUs with different bit patterns.
 * Execute each under a fixed environment.
 * Measure the number of distinguishable behavioral fingerprints.
 *
 * A "behavioral fingerprint" is the tuple:
 *   (output_values, final_register_state, fuel_consumed, halted_normally)
 *
 * Success criterion: at least `min_unique` distinct behaviors from N units.
 */
int experiment_h1(uint16_t num_units, uint16_t min_unique,
                  ExperimentResult *result);

/* ─── H2: State Evolution ───────────────────────────────────── */

/*
 * Create a state-evolving EIU and execute it for `steps` iterations.
 * After each execution, the VM's final state becomes the EIU's new state.
 *
 * Validates:
 *   - U_{t+1} = F(U_t, E_t) is deterministic
 *   - At least `min_distinct` distinct states are visited
 *   - Fixed point or cycle detection works correctly
 *
 * Success criterion: valid transitions for all steps with sufficient diversity.
 */
int experiment_h2(uint16_t steps, uint16_t min_distinct,
                  ExperimentResult *result);

/* ─── H3: Constrained Replication ───────────────────────────── */

/*
 * Create a SPAWN-capable EIU and attempt replication.
 *
 * Validates:
 *   - Replication only occurs when A(U,E) = 1
 *   - All successors U_i are structurally valid
 *   - Unauthorized replication is correctly blocked
 *   - Replication count is within budget
 *
 * Success criterion: all authorized replicas valid, unauthorized attempts blocked.
 */
int experiment_h3(uint16_t replica_count, ExperimentResult *result);

/* ─── Run All Experiments ───────────────────────────────────── */

/*
 * Run all three experiments and print results.
 */
int experiments_run_all(void);

/* ─── Utility ───────────────────────────────────────────────── */

void experiment_print_result(const ExperimentResult *result);

#endif /* AIRBOT_EXPERIMENTS_H */
