#ifndef AIRBOT_METRICS_H
#define AIRBOT_METRICS_H

#include <stdint.h>
#include <stddef.h>
#include "eiu.h"

typedef struct {
    double   bit_efficiency;      /* η = behavioral_complexity / |U| */
    double   compression_ratio;   /* ρ = conventional_size / EIU_size */
    double   behavioral_diversity; /* H(B) = Shannon entropy */
    uint32_t total_bits;          /* |U| */
    uint16_t instruction_count;   /* Number of instructions in behavior */
    uint16_t unique_opcodes;      /* Number of distinct opcodes used */
    uint16_t data_bits;           /* Bits used for data */
    uint16_t behavior_bits;       /* Bits used for behavior */
    uint16_t state_bits;          /* Bits used for state */
    uint16_t overhead_bits;       /* Bits used for header/metadata */
} EIUMetrics;

/* Initialize metrics state */
void metrics_init(EIUMetrics *m);

/* Analyze EIU and populate metrics */
int metrics_analyze(const EIU *eiu, EIUMetrics *m);

/* Calculate Shannon Entropy of data */
double metrics_shannon_entropy(const uint8_t *data, size_t len);

/* Calculate bit efficiency */
double metrics_bit_efficiency(const EIU *eiu);

/* Print metrics to stdout */
void metrics_print(const EIUMetrics *m);

#endif /* AIRBOT_METRICS_H */
