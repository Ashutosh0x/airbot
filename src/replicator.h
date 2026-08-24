#ifndef AIRBOT_REPLICATOR_H
#define AIRBOT_REPLICATOR_H

#include <stdint.h>
#include "eiu.h"
#include "environment.h"
#include "capability.h"

#define MAX_REPLICAS 16
#define CAP_REPLICATE (1ULL << 3) /* Define CAP_REPLICATE permission */

/**
 * Holds resulting replicas from a replication operation
 */
typedef struct {
    EIU      replicas[MAX_REPLICAS]; /* Array of successor EIUs */
    uint16_t count;                   /* Number of replicas created */
    int      authorized;              /* 1 if replication was authorized */
    int      all_valid;               /* 1 if all replicas pass validation */
} ReplicationResult;

/** Zero-initialize the replication result */
void repl_result_init(ReplicationResult *result);

/** Check if replication is authorized given EIU capabilities and Env */
int repl_check_authorization(const EIU *eiu, const Environment *env);

/** Create constrained replicas of the EIU */
int repl_execute(const EIU *eiu, const Environment *env, uint16_t count, ReplicationResult *result);

/** Check validity of all generated replicas */
int repl_validate_all(const ReplicationResult *result);

/** Print summary of the replication result */
void repl_print_summary(const ReplicationResult *result);

#endif /* AIRBOT_REPLICATOR_H */
