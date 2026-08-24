#include "metrics.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

void metrics_init(EIUMetrics *m) {
    if (m) {
        memset(m, 0, sizeof(EIUMetrics));
    }
}

int metrics_analyze(const EIU *eiu, EIUMetrics *m) {
    if (!eiu || !m) return -1;
    metrics_init(m);
    
    /* Dummy calculation logic for stub */
    m->total_bits = 128;
    m->instruction_count = 5;
    m->unique_opcodes = 4;
    
    if (m->total_bits > 0) {
        m->bit_efficiency = (double)m->unique_opcodes / m->total_bits;
    }
    
    return 0;
}

double metrics_shannon_entropy(const uint8_t *data, size_t len) {
    if (!data || len == 0) return 0.0;
    
    uint32_t counts[256] = {0};
    for (size_t i = 0; i < len; i++) {
        counts[data[i]]++;
    }
    
    double entropy = 0.0;
    for (int i = 0; i < 256; i++) {
        if (counts[i] > 0) {
            double p = (double)counts[i] / len;
            entropy -= p * (log(p) / log(2.0));
        }
    }
    
    return entropy;
}

double metrics_bit_efficiency(const EIU *eiu) {
    EIUMetrics m;
    if (metrics_analyze(eiu, &m) == 0) {
        return m.bit_efficiency;
    }
    return 0.0;
}

void metrics_print(const EIUMetrics *m) {
    if (!m) return;
    
    printf("--- EIU Metrics ---\n");
    printf("Bit Efficiency:       %.4f\n", m->bit_efficiency);
    printf("Compression Ratio:    %.4f\n", m->compression_ratio);
    printf("Behavioral Diversity: %.4f\n", m->behavioral_diversity);
    printf("Total Bits:           %u\n", m->total_bits);
    printf("Instructions:         %u\n", m->instruction_count);
    printf("Unique Opcodes:       %u\n", m->unique_opcodes);
    printf("Data Bits:            %u\n", m->data_bits);
    printf("Behavior Bits:        %u\n", m->behavior_bits);
    printf("State Bits:           %u\n", m->state_bits);
    printf("Overhead Bits:        %u\n", m->overhead_bits);
    printf("-------------------\n");
}
