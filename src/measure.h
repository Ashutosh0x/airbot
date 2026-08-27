/*
 * Airbot — Executable Information System
 * measure.h — Production-path measurement harness
 */
#ifndef AIRBOT_MEASURE_H
#define AIRBOT_MEASURE_H

/* Phase 1: live batching correlation, measured on the production relay. */
int measure_batching(void);

/* Phase 6: end-to-end performance of the production path. */
int measure_performance(void);

#endif /* AIRBOT_MEASURE_H */
