#ifndef AIRBOT_STATE_H
#define AIRBOT_STATE_H

#include <stdint.h>
#include "eiu.h"
#include "environment.h"
#include "blake3.h"

#define MAX_TRACE_LENGTH 256

/**
 * Trace entry recording the state transition
 */
typedef struct {
    uint8_t  state_hash[32];   /* BLAKE3 hash of the EIU at this step */
    uint16_t fuel_used;        /* Fuel consumed in this transition */
    uint16_t step;             /* Step number */
} TraceEntry;

/**
 * Evolution trace recording state trajectory and cycle detection
 */
typedef struct {
    TraceEntry entries[MAX_TRACE_LENGTH];
    uint16_t   length;          /* Number of entries recorded */
    int        fixed_point;     /* Step at which F(U)=U was detected (-1 if none) */
    int        cycle_start;     /* Step where cycle begins (-1 if none) */
    int        cycle_length;    /* Length of detected cycle (0 if none) */
} EvolutionTrace;

/** Initialize a trace structure */
void trace_init(EvolutionTrace *trace);

/** Record an evolution step in the trace */
int trace_record(EvolutionTrace *trace, const uint8_t hash[32], uint16_t fuel_used);

/** Scan trace for fixed point F(U) = U */
int trace_detect_fixed_point(EvolutionTrace *trace);

/** Scan trace for cycles */
int trace_detect_cycle(EvolutionTrace *trace);

/** Run the EIU for multiple steps and track evolution */
int state_evolve(EIU *eiu, const Environment *env, uint16_t steps, EvolutionTrace *trace);

/** Print trace summary to stdout */
void trace_print_summary(const EvolutionTrace *trace);

#endif /* AIRBOT_STATE_H */
