/*
 * Airbot — Executable Information System
 * matrix.h — Population Matrix System
 *
 * Implements: U_{t+1} = F(U_t, E_t) for populations of EIUs.
 * Tracks population-level evolution, diversity, and emergent patterns.
 */

#ifndef AIRBOT_MATRIX_H
#define AIRBOT_MATRIX_H

#include <stdint.h>
#include "eiu.h"
#include "environment.h"
#include "metrics.h"

#define MATRIX_MAX_POPULATION 64
#define MATRIX_MAX_STEPS      256

/* ─── Population State ──────────────────────────────────────── */

typedef struct {
    uint8_t  hash[32];       /* BLAKE3 hash of this EIU */
    uint16_t fuel_consumed;  /* Fuel used in last execution */
    uint8_t  alive;          /* 1 if still active */
    uint8_t  generation;     /* Generation counter */
} PopulationEntry;

typedef struct {
    EIU             units[MATRIX_MAX_POPULATION];
    PopulationEntry entries[MATRIX_MAX_POPULATION];
    uint16_t        count;       /* Current population size */
    uint16_t        step;        /* Current time step */
    uint16_t        max_steps;   /* Maximum steps to simulate */
    Environment     env;         /* Shared execution environment */

    /* Statistics per step */
    uint16_t alive_count;        /* Active units this step */
    uint16_t unique_states;      /* Distinct states this step */
    double   diversity;          /* Shannon entropy of state distribution */
} PopulationMatrix;

/* ─── Functions ─────────────────────────────────────────────── */

/* Initialize a population matrix with N identical EIUs */
void matrix_init(PopulationMatrix *mat, uint16_t population_size,
                 const EIU *template_eiu, const Environment *env);

/* Advance the entire population by one step */
int matrix_step(PopulationMatrix *mat);

/* Run the full simulation for max_steps */
int matrix_run(PopulationMatrix *mat, uint16_t max_steps);

/* Compute population diversity (Shannon entropy of state hashes) */
double matrix_diversity(const PopulationMatrix *mat);

/* Count unique states in the population */
uint16_t matrix_unique_states(const PopulationMatrix *mat);

/* Print population summary to stdout */
void matrix_print_summary(const PopulationMatrix *mat);

/* Print full simulation report */
void matrix_print_report(const PopulationMatrix *mat);

#endif /* AIRBOT_MATRIX_H */
