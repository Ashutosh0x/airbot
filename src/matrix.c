/*
 * Airbot — Executable Information System
 * matrix.c — Population Matrix System Implementation
 *
 * Simulates populations of EIUs evolving in parallel.
 * Tracks diversity, convergence, and emergent patterns.
 */

#include "matrix.h"
#include "bitstream.h"
#include "blake3.h"
#include "state.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

/* ═══════════════════════════════════════════════════════════════ */

void matrix_init(PopulationMatrix *mat, uint16_t population_size,
                 const EIU *template_eiu, const Environment *env) {
    memset(mat, 0, sizeof(*mat));

    if (population_size > MATRIX_MAX_POPULATION) {
        population_size = MATRIX_MAX_POPULATION;
    }

    mat->count = population_size;
    mat->step = 0;
    mat->max_steps = MATRIX_MAX_STEPS;
    memcpy(&mat->env, env, sizeof(Environment));

    /* Initialize each unit as a copy of template with varying state */
    for (uint16_t i = 0; i < population_size; i++) {
        memcpy(&mat->units[i], template_eiu, sizeof(EIU));

        /* Give each unit a unique initial state mutation */
        if (mat->units[i].state_len > 0) {
            mat->units[i].state[0] = (uint8_t)(i & 0xFF);
        }

        mat->entries[i].alive = 1;
        mat->entries[i].generation = 0;
        mat->entries[i].fuel_consumed = 0;

        /* Compute initial hash */
        eiu_compute_hash(&mat->units[i]);
        memcpy(mat->entries[i].hash, mat->units[i].content_hash, 32);
    }

    mat->alive_count = population_size;
}

int matrix_step(PopulationMatrix *mat) {
    for (uint16_t i = 0; i < mat->count; i++) {
        if (!mat->entries[i].alive) continue;

        /* Execute the EIU */
        ExecutionResult exec;
        exec_result_init(&exec);
        int rc = env_execute(&mat->env, &mat->units[i], &exec);

        if (rc != AIRBOT_OK || !exec.success) {
            mat->entries[i].alive = 0;
            continue;
        }

        mat->entries[i].fuel_consumed = exec.fuel_consumed;
        mat->entries[i].generation++;

        /* Update the EIU's state from execution result */
        if (exec.final_state_len > 0) {
            uint16_t copy_len = exec.final_state_len;
            if (copy_len > EIU_MAX_STATE) copy_len = EIU_MAX_STATE;
            memcpy(mat->units[i].state, exec.final_state, copy_len);
            mat->units[i].state_len = copy_len;
            mat->units[i].flags |= EIU_FLAG_HAS_STATE;
        }

        /* Reset fuel for next iteration */
        mat->units[i].fuel = mat->env.fuel_limit;

        /* Recompute hash */
        eiu_compute_hash(&mat->units[i]);
        memcpy(mat->entries[i].hash, mat->units[i].content_hash, 32);
    }

    /* Count alive units */
    mat->alive_count = 0;
    for (uint16_t i = 0; i < mat->count; i++) {
        if (mat->entries[i].alive) mat->alive_count++;
    }

    /* Compute stats */
    mat->unique_states = matrix_unique_states(mat);
    mat->diversity = matrix_diversity(mat);

    mat->step++;

    return AIRBOT_OK;
}

int matrix_run(PopulationMatrix *mat, uint16_t max_steps) {
    mat->max_steps = max_steps;

    printf("\n  Population Matrix Simulation\n");
    printf("  Population: %u | Max Steps: %u\n\n", mat->count, max_steps);
    printf("  Step | Alive | Unique | Diversity\n");
    printf("  -----+-------+--------+----------\n");

    for (uint16_t t = 0; t < max_steps; t++) {
        int rc = matrix_step(mat);
        if (rc != AIRBOT_OK) return rc;

        /* Print every 10th step or last step */
        if (t % 10 == 0 || t == max_steps - 1 || mat->alive_count == 0) {
            printf("  %4u | %5u | %6u | %.4f\n",
                   mat->step, mat->alive_count,
                   mat->unique_states, mat->diversity);
        }

        /* Stop if all units are dead */
        if (mat->alive_count == 0) {
            printf("  [All units halted at step %u]\n", mat->step);
            break;
        }
    }

    printf("\n");
    return AIRBOT_OK;
}

double matrix_diversity(const PopulationMatrix *mat) {
    if (mat->alive_count <= 1) return 0.0;

    /* Count occurrences of each unique hash */
    uint8_t  hashes[MATRIX_MAX_POPULATION][32];
    uint16_t counts[MATRIX_MAX_POPULATION];
    uint16_t unique = 0;

    memset(counts, 0, sizeof(counts));

    for (uint16_t i = 0; i < mat->count; i++) {
        if (!mat->entries[i].alive) continue;

        /* Check if hash already seen */
        int found = -1;
        for (uint16_t j = 0; j < unique; j++) {
            if (memcmp(mat->entries[i].hash, hashes[j], 32) == 0) {
                found = (int)j;
                break;
            }
        }

        if (found >= 0) {
            counts[found]++;
        } else if (unique < MATRIX_MAX_POPULATION) {
            memcpy(hashes[unique], mat->entries[i].hash, 32);
            counts[unique] = 1;
            unique++;
        }
    }

    /* Shannon entropy: H = -Σ (p_i * log2(p_i)) */
    double entropy = 0.0;
    for (uint16_t i = 0; i < unique; i++) {
        double p = (double)counts[i] / (double)mat->alive_count;
        if (p > 0.0) {
            entropy -= p * (log(p) / log(2.0));
        }
    }

    return entropy;
}

uint16_t matrix_unique_states(const PopulationMatrix *mat) {
    uint8_t  seen[MATRIX_MAX_POPULATION][32];
    uint16_t count = 0;

    for (uint16_t i = 0; i < mat->count; i++) {
        if (!mat->entries[i].alive) continue;

        int is_new = 1;
        for (uint16_t j = 0; j < count; j++) {
            if (memcmp(mat->entries[i].hash, seen[j], 32) == 0) {
                is_new = 0;
                break;
            }
        }
        if (is_new && count < MATRIX_MAX_POPULATION) {
            memcpy(seen[count], mat->entries[i].hash, 32);
            count++;
        }
    }

    return count;
}

void matrix_print_summary(const PopulationMatrix *mat) {
    printf("  Population: %u/%u alive\n", mat->alive_count, mat->count);
    printf("  Step:       %u\n", mat->step);
    printf("  Unique:     %u\n", mat->unique_states);
    printf("  Diversity:  %.4f bits\n", mat->diversity);
}

void matrix_print_report(const PopulationMatrix *mat) {
    printf("\n╔═══════════════════════════════════════════════╗\n");
    printf("║       POPULATION MATRIX FINAL REPORT          ║\n");
    printf("╚═══════════════════════════════════════════════╝\n\n");
    matrix_print_summary(mat);

    printf("\n  Individual Unit Status:\n");
    printf("  ID | Alive | Gen | Fuel Used | Hash (first 8 bytes)\n");
    printf("  ---+-------+-----+-----------+---------------------\n");
    for (uint16_t i = 0; i < mat->count; i++) {
        printf("  %2u | %5s | %3u |     %5u | ",
               i,
               mat->entries[i].alive ? "YES" : "NO",
               mat->entries[i].generation,
               mat->entries[i].fuel_consumed);
        for (int j = 0; j < 8; j++) {
            printf("%02x", mat->entries[i].hash[j]);
        }
        printf("\n");
    }
    printf("\n");
}
