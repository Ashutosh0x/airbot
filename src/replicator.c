#include "replicator.h"
#include <string.h>
#include <stdio.h>

void repl_result_init(ReplicationResult *result) {
    if (!result) return;
    memset(result, 0, sizeof(ReplicationResult));
}

int repl_check_authorization(const EIU *eiu, const Environment *env) {
    if (!eiu || !env) return 0;
    
    Capability cap;
    memset(&cap, 0, sizeof(Capability));
    cap.rights = eiu->capability.rights;
    
    /* In a full capability system, we might require cap_is_valid and cap_check */
    if ((eiu->capability.rights & CAP_REPLICATE) && (env->allowed_caps & CAP_REPLICATE)) {
        return 1;
    }
    
    return 0;
}

int repl_execute(const EIU *eiu, const Environment *env, uint16_t count, ReplicationResult *result) {
    if (!eiu || !env || !result) return -1;
    
    repl_result_init(result);
    
    if (!repl_check_authorization(eiu, env)) {
        result->authorized = 0;
        return -1;
    }
    
    result->authorized = 1;
    
    if (count > MAX_REPLICAS) {
        count = MAX_REPLICAS;
    }
    
    for (uint16_t i = 0; i < count; i++) {
        memcpy(&result->replicas[i], eiu, sizeof(EIU));
        
        /* Mutate state[0] for simple differentiation */
        if (result->replicas[i].state_len > 0) {
            result->replicas[i].state[0] += (uint8_t)(i + 1);
        }
        
        eiu_compute_hash(&result->replicas[i]);
    }
    
    result->count = count;
    result->all_valid = repl_validate_all(result);
    
    return 0;
}

int repl_validate_all(const ReplicationResult *result) {
    if (!result) return 0;
    
    for (uint16_t i = 0; i < result->count; i++) {
        if (eiu_validate(&result->replicas[i]) != 1) {
            return 0;
        }
    }
    return 1;
}

void repl_print_summary(const ReplicationResult *result) {
    if (!result) return;
    
    printf("Replication Authorized: %s\n", result->authorized ? "Yes" : "No");
    printf("Replicas created: %u\n", result->count);
    printf("All replicas valid: %s\n", result->all_valid ? "Yes" : "No");
}
