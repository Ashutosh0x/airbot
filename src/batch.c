/*
 * Airbot — Executable Information System
 * batch.c — Bounded relay batching
 */
#include "batch.h"
#include "chacha20.h"
#include "transport.h"

#include <stdio.h>
#include <string.h>

void batch_init(BatchQueue *q, int max_frames, uint32_t wait_ms) {
    memset(q, 0, sizeof(*q));
    if (max_frames < 1) max_frames = 1;
    if (max_frames > BATCH_MAX_FRAMES) max_frames = BATCH_MAX_FRAMES;
    q->max_frames = max_frames;
    q->wait_ms = wait_ms;
    q->first_arrival_ms = -1.0;
}

const char *batch_strerror(int code) {
    switch (code) {
        case BQ_OK:        return "ok";
        case BQ_ERR_FULL:  return "batch queue full (backpressure)";
        case BQ_ERR_EMPTY: return "batch queue empty";
        case BQ_ERR_SIZE:  return "frame is not the fixed envelope size";
        default:           return "unknown batch error";
    }
}

int batch_enqueue(BatchQueue *q, const uint8_t *frame, uint16_t next_port,
                  double now_ms) {
    int i;
    if (!q || !frame) return BQ_ERR_SIZE;

    /* Bounded memory: refuse rather than grow. The caller decides whether to
       drop the frame or apply backpressure upstream; silently expanding the
       queue would turn memory into a denial-of-service surface. */
    if (q->count >= q->max_frames) { q->total_rejected++; return BQ_ERR_FULL; }

    for (i = 0; i < q->max_frames; i++) {
        if (!q->slots[i].used) {
            memcpy(q->slots[i].frame, frame, AC_ENVELOPE_WIRE);
            q->slots[i].next_port = next_port;
            q->slots[i].used = 1;
            if (q->count == 0) q->first_arrival_ms = now_ms;
            q->count++;
            q->total_enqueued++;
            return BQ_OK;
        }
    }
    q->total_rejected++;
    return BQ_ERR_FULL;
}

int batch_should_release(const BatchQueue *q, double now_ms) {
    if (!q || q->count == 0) return 0;
    if (q->count >= q->max_frames) return 1;              /* full */
    if (q->first_arrival_ms < 0) return 0;
    return (now_ms - q->first_arrival_ms) >= (double)q->wait_ms;  /* deadline */
}

/* Fisher-Yates over CSPRNG output. Release order must not reveal arrival
   order, otherwise batching adds latency without adding ambiguity. */
static void shuffle(BatchSlot *a, int n) {
    int i;
    for (i = n - 1; i > 0; i--) {
        uint8_t r[4];
        uint32_t v;
        int j;
        BatchSlot tmp;
        csprng_bytes(r, 4);
        v = ((uint32_t)r[0] << 24) | ((uint32_t)r[1] << 16) |
            ((uint32_t)r[2] << 8)  |  (uint32_t)r[3];
        j = (int)(v % (uint32_t)(i + 1));
        tmp = a[i]; a[i] = a[j]; a[j] = tmp;
    }
}

static int drain(BatchQueue *q, BatchSlot *out, int out_cap) {
    int i, n = 0;
    for (i = 0; i < q->max_frames && n < out_cap; i++) {
        if (q->slots[i].used) {
            out[n++] = q->slots[i];
            q->slots[i].used = 0;
        }
    }
    q->count = 0;
    q->first_arrival_ms = -1.0;
    q->total_released += (unsigned long)n;
    shuffle(out, n);
    return n;
}

int batch_release(BatchQueue *q, BatchSlot *out, int out_cap) {
    if (!q || !out || q->count == 0) return 0;
    return drain(q, out, out_cap);
}

int batch_flush(BatchQueue *q, BatchSlot *out, int out_cap) {
    if (!q || !out) return 0;
    return drain(q, out, out_cap);
}

/* --- self test -------------------------------------------- */

int batch_selftest(void) {
    BatchQueue q;
    BatchSlot out[BATCH_MAX_FRAMES];
    uint8_t f[AC_ENVELOPE_WIRE];
    int i, n, fails = 0;

    batch_init(&q, 8, 250);

    /* Fill to capacity. */
    for (i = 0; i < 8; i++) {
        memset(f, 0, sizeof(f));
        f[0] = (uint8_t)i;                     /* marker to track order */
        if (batch_enqueue(&q, f, 9000, 0.0) != BQ_OK) fails++;
    }

    /* Backpressure: the ninth must be refused, not queued. */
    memset(f, 0xFF, sizeof(f));
    if (batch_enqueue(&q, f, 9000, 0.0) != BQ_ERR_FULL) fails++;
    if (q.count != 8) fails++;
    if (q.total_rejected != 1) fails++;

    /* A full batch releases immediately, regardless of the deadline. */
    if (!batch_should_release(&q, 0.0)) fails++;

    n = batch_release(&q, out, BATCH_MAX_FRAMES);
    if (n != 8) fails++;
    if (q.count != 0) fails++;

    /* Every frame must survive exactly once (shuffle must not lose or
       duplicate). Markers 0..7 should each appear one time. */
    {
        int seen[8];
        memset(seen, 0, sizeof(seen));
        for (i = 0; i < n; i++)
            if (out[i].frame[0] < 8) seen[out[i].frame[0]]++;
        for (i = 0; i < 8; i++) if (seen[i] != 1) fails++;
    }

    /* Deadline behaviour: a partial batch waits, then releases. */
    batch_init(&q, 8, 250);
    memset(f, 0, sizeof(f));
    batch_enqueue(&q, f, 9000, 1000.0);
    if (batch_should_release(&q, 1100.0)) fails++;   /* 100 ms < 250 ms */
    if (!batch_should_release(&q, 1250.0)) fails++;  /* deadline reached */

    /* Flush drains regardless of deadline. */
    n = batch_flush(&q, out, BATCH_MAX_FRAMES);
    if (n != 1) fails++;
    if (q.count != 0) fails++;

    /* Shuffle must actually reorder often enough to matter. With 8 frames a
       correct shuffle leaves them in the original order 1/8! of the time. */
    {
        int reordered = 0, trial;
        for (trial = 0; trial < 20; trial++) {
            batch_init(&q, 8, 250);
            for (i = 0; i < 8; i++) {
                memset(f, 0, sizeof(f));
                f[0] = (uint8_t)i;
                batch_enqueue(&q, f, 9000, 0.0);
            }
            n = batch_release(&q, out, BATCH_MAX_FRAMES);
            for (i = 0; i < n; i++)
                if (out[i].frame[0] != (uint8_t)i) { reordered++; break; }
        }
        if (reordered < 15) fails++;   /* expect ~20/20 */
    }

    return fails;
}
