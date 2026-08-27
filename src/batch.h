/*
 * Airbot — Executable Information System
 * batch.h — Bounded relay batching (traffic-analysis mitigation)
 *
 * MEASURED BASELINE (harness, previous round):
 *   immediate forwarding  100.0% ingress->egress matching
 *   250 ms batch+shuffle   18.8%
 *   random chance           2.5%
 *
 * That 18.8% described a harness, not this relay. This module puts the same
 * mechanism into the production relay so the number can be re-measured
 * against the real implementation.
 *
 * DESIGN CONSTRAINTS (all enforced, none optional):
 *   - bounded memory: at most BATCH_MAX_FRAMES queued, each of fixed size
 *   - release on EITHER a full batch OR the wait deadline, whichever first
 *   - random release order within a batch (Fisher-Yates over a CSPRNG)
 *   - backpressure: a full queue rejects rather than growing without limit
 *   - deterministic shutdown: flush drains everything and frees nothing
 *
 * PRIVACY CONSTRAINTS:
 *   - no batch identifier is ever placed on the wire
 *   - no timestamp is transmitted
 *   - no per-frame sequence number or global counter is transmitted
 *   - queue position is never observable: order is randomised at release
 *   - each frame keeps its own independent AEAD envelope, so batching adds
 *     no shared cryptographic state between frames
 *
 * COST: up to `wait_ms` of added latency per hop, and queue memory of
 * BATCH_MAX_FRAMES * AC_ENVELOPE_WIRE bytes per relay.
 */
#ifndef AIRBOT_BATCH_H
#define AIRBOT_BATCH_H

#include <stdint.h>
#include <stddef.h>
#include "airbchan.h"

#define BATCH_MAX_FRAMES 32

#define BQ_OK            0
#define BQ_ERR_FULL    -140   /* backpressure: caller must drop or block */
#define BQ_ERR_EMPTY   -141
#define BQ_ERR_SIZE    -142

typedef struct {
    uint8_t  frame[AC_ENVELOPE_WIRE];
    uint16_t next_port;        /* where this frame must go */
    int      used;
} BatchSlot;

typedef struct {
    BatchSlot slots[BATCH_MAX_FRAMES];
    int       count;
    int       max_frames;      /* release when reached */
    uint32_t  wait_ms;         /* release when exceeded */
    double    first_arrival_ms;
    unsigned long total_enqueued;
    unsigned long total_released;
    unsigned long total_rejected;
} BatchQueue;

void batch_init(BatchQueue *q, int max_frames, uint32_t wait_ms);

/* Enqueue one fixed-size frame. Returns BQ_ERR_FULL under backpressure. */
int  batch_enqueue(BatchQueue *q, const uint8_t *frame, uint16_t next_port,
                   double now_ms);

/* 1 when the batch should be released (full, or the deadline passed). */
int  batch_should_release(const BatchQueue *q, double now_ms);

/*
 * Drain the queue into `out` in RANDOM order. Returns the number of frames
 * written. `out` must hold at least max_frames slots.
 */
int  batch_release(BatchQueue *q, BatchSlot *out, int out_cap);

/* Drain everything regardless of deadline (clean shutdown). */
int  batch_flush(BatchQueue *q, BatchSlot *out, int out_cap);

const char *batch_strerror(int code);

/* Self-test: bounds, backpressure, shuffling, deadline, flush. */
int batch_selftest(void);

#endif /* AIRBOT_BATCH_H */
