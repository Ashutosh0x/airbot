#include "state.h"
#include <string.h>
#include <stdio.h>

void trace_init(EvolutionTrace *trace) {
    if (!trace) return;
    memset(trace, 0, sizeof(EvolutionTrace));
    trace->fixed_point = -1;
    trace->cycle_start = -1;
    trace->cycle_length = 0;
}

int trace_record(EvolutionTrace *trace, const uint8_t hash[32], uint16_t fuel_used) {
    if (!trace || trace->length >= MAX_TRACE_LENGTH) return -1;
    memcpy(trace->entries[trace->length].state_hash, hash, 32);
    trace->entries[trace->length].fuel_used = fuel_used;
    trace->entries[trace->length].step = trace->length;
    trace->length++;
    return 0;
}

int trace_detect_fixed_point(EvolutionTrace *trace) {
    if (!trace || trace->length < 2) return -1;
    for (uint16_t i = 1; i < trace->length; i++) {
        if (memcmp(trace->entries[i].state_hash, trace->entries[i-1].state_hash, 32) == 0) {
            trace->fixed_point = i;
            return i;
        }
    }
    return -1;
}

int trace_detect_cycle(EvolutionTrace *trace) {
    if (!trace || trace->length < 2) return -1;
    
    for (uint16_t i = 0; i < trace->length; i++) {
        for (uint16_t j = i + 1; j < trace->length; j++) {
            if (memcmp(trace->entries[i].state_hash, trace->entries[j].state_hash, 32) == 0) {
                trace->cycle_start = i;
                trace->cycle_length = j - i;
                return trace->cycle_start;
            }
        }
    }
    return -1;
}

int state_evolve(EIU *eiu, const Environment *env, uint16_t steps, EvolutionTrace *trace) {
    if (!eiu || !env || !trace) return -1;
    
    trace_init(trace);
    
    for (uint16_t i = 0; i < steps; i++) {
        ExecutionResult result;
        int ret = env_execute(env, eiu, &result);
        if (ret < 0) {
            return ret;
        }
        
        uint16_t update_len = result.final_state_len;
        if (update_len > sizeof(eiu->state)) {
            update_len = sizeof(eiu->state);
        }
        memcpy(eiu->state, result.final_state, update_len);
        eiu->state_len = update_len;
        
        eiu_compute_hash(eiu);
        
        if (trace_record(trace, eiu->content_hash, result.fuel_consumed) < 0) {
            break;
        }
    }
    
    trace_detect_fixed_point(trace);
    trace_detect_cycle(trace);
    
    return 0;
}

void trace_print_summary(const EvolutionTrace *trace) {
    if (!trace) return;
    printf("Evolution Trace Length: %u\n", trace->length);
    if (trace->fixed_point != -1) {
        printf("Fixed point reached at step %d\n", trace->fixed_point);
    }
    if (trace->cycle_start != -1) {
        printf("Cycle detected at step %d (length %d)\n", trace->cycle_start, trace->cycle_length);
    }
}
