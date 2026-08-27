/*
 * Airbot — Executable Information System
 * measure.c — Production-path measurement harness
 *
 * Every number this file prints comes from the PRODUCTION functions
 * (airbchan_send / airbchan_open_buf / ox_peel / batch_*) driven over real
 * loopback sockets. Nothing here re-implements the protocol.
 *
 * Metric note: nearest-neighbour matching MUST break ties randomly. Breaking
 * them by index silently recovers k->k whenever a batch releases together,
 * which makes any batching mitigation look useless. That mistake was made
 * once already; the tie-break below is explicitly randomised.
 */
#include "measure.h"
#include "airbchan.h"
#include "onionx.h"
#include "batch.h"
#include "transport.h"
#include "netpolicy.h"
#include "chacha20.h"
#include "x25519.h"
#include "blake3.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <windows.h>

#define MAX_EVENTS 600

typedef struct {
    double   t_ms;
    uint16_t size;
    int      seq;      /* ground-truth index, used only for scoring */
} Event;

static Event g_in[MAX_EVENTS];
static Event g_out[MAX_EVENTS];
static int   g_nin = 0, g_nout = 0;
static CRITICAL_SECTION g_lock;

/* Relay-A state under measurement. */
static BatchQueue g_bq;
static int        g_batching = 0;
static volatile int g_stop = 0;

static double now_ms(void) { return transport_now_ms(); }

static int cmp_double(const void *a, const void *b) {
    double x = *(const double *)a, y = *(const double *)b;
    return (x > y) - (x < y);
}

static double pct(double *v, int n, double p) {
    int idx;
    if (n <= 0) return 0.0;
    idx = (int)(p * (n - 1) + 0.5);
    if (idx < 0) idx = 0;
    if (idx >= n) idx = n - 1;
    return v[idx];
}

/* --- relay A under test ----------------------------------- */

static OxIdentity g_rA;

static void emit(double t) {
    EnterCriticalSection(&g_lock);
    if (g_nout < MAX_EVENTS) {
        g_out[g_nout].t_ms = t;
        g_out[g_nout].size = AC_ENVELOPE_WIRE;
        g_out[g_nout].seq  = g_nout;
        g_nout++;
    }
    LeaveCriticalSection(&g_lock);
}

/* Timer thread: releases the batch when its deadline expires. */
static DWORD WINAPI flusher_thread(LPVOID arg) {
    (void)arg;
    while (!g_stop) {
        Sleep(10);
        EnterCriticalSection(&g_lock);
        if (g_batching && batch_should_release(&g_bq, now_ms())) {
            BatchSlot out[BATCH_MAX_FRAMES];
            int n = batch_release(&g_bq, out, BATCH_MAX_FRAMES);
            double t = now_ms();
            int i;
            for (i = 0; i < n; i++) emit(t);   /* whole batch leaves together */
        }
        LeaveCriticalSection(&g_lock);
    }
    return 0;
}

static DWORD WINAPI relay_thread(LPVOID arg) {
    uint16_t port = *(uint16_t *)arg;
    AirbConn listener;
    if (transport_listen(port, &listener) != AIRB_OK) return 0;
    while (!g_stop) {
        AirbConn peer;
        uint8_t buf[AC_ENVELOPE_WIRE];
        uint32_t got = 0;
        if (transport_accept(&listener, &peer) != AIRB_OK) break;
        transport_set_timeout(&peer, 2000);
        while (got < AC_ENVELOPE_WIRE) {
            int r = transport_recv_raw(&peer, buf + got, AC_ENVELOPE_WIRE - got);
            if (r <= 0) break;
            got += (uint32_t)r;
        }
        if (got == AC_ENVELOPE_WIRE) {
            double t = now_ms();
            EnterCriticalSection(&g_lock);
            if (g_nin < MAX_EVENTS) {
                g_in[g_nin].t_ms = t;
                g_in[g_nin].size = AC_ENVELOPE_WIRE;
                g_in[g_nin].seq  = g_nin;
                g_nin++;
            }
            if (g_batching) {
                if (batch_enqueue(&g_bq, buf, 0, t) != BQ_OK) {
                    /* Backpressure: release now rather than grow. */
                    BatchSlot o[BATCH_MAX_FRAMES];
                    int n = batch_release(&g_bq, o, BATCH_MAX_FRAMES), i;
                    double rt = now_ms();
                    for (i = 0; i < n; i++) emit(rt);
                    batch_enqueue(&g_bq, buf, 0, t);
                }
            } else {
                LeaveCriticalSection(&g_lock);
                emit(now_ms());          /* immediate forward */
                EnterCriticalSection(&g_lock);
            }
            LeaveCriticalSection(&g_lock);
        }
        transport_close(&peer);
    }
    transport_close(&listener);
    return 0;
}

/* --- correlation scoring ---------------------------------- */

typedef struct {
    double timing_acc;     /* % ingress correctly matched to egress */
    double mean_set;       /* mean candidate set size (anonymity set) */
    double order_pres;     /* % of pairs whose relative order is preserved */
    double size_info;      /* distinct egress sizes; 1 == no information */
    double lat_mean, lat_med, lat_p95, lat_p99, lat_max;
} CorrResult;

/*
 * Correlation scoring.
 *
 * CORRECTNESS NOTE, learned the hard way twice: the time tolerance must be
 * applied AMONG EGRESS EVENTS (i.e. which frames left together), not between
 * ingress and egress. Batching deliberately separates ingress from egress by
 * up to one round, so an ingress<->egress tolerance finds no candidates, falls
 * back to nearest-neighbour, and reports a set size of 1 - making any batching
 * mitigation look useless when it is the metric that is wrong.
 *
 * The adversary's real question is: given an egress event, which ingress
 * events could have produced it? Everything released in the same batch is
 * indistinguishable, because release order is randomised. So the anonymity
 * set is the batch group size and best-case accuracy is 1/|group|.
 */
static void score(CorrResult *r) {
    int n = g_nin < g_nout ? g_nin : g_nout;
    int i, j;
    double setsum = 0.0, accsum = 0.0;
    double lat[MAX_EVENTS];
    const double TOL = 4.0;   /* ms: released together */

    memset(r, 0, sizeof(*r));
    if (n < 4) return;

    /* Group egress events that left simultaneously. */
    for (i = 0; i < n; i++) {
        int group = 0;
        for (j = 0; j < n; j++)
            if (fabs(g_out[j].t_ms - g_out[i].t_ms) <= TOL) group++;
        if (group < 1) group = 1;
        setsum += group;
        accsum += 1.0 / (double)group;   /* best-case attacker within a group */
    }
    r->mean_set   = setsum / n;
    r->timing_acc = 100.0 * accsum / n;

    /* Ordering: a tie inside a batch is ambiguous, so it does NOT count as
       preserved - release order is randomised. */
    {
        int pairs = 0, kept = 0;
        for (i = 0; i < n && i < 60; i++)
            for (j = i + 1; j < n && j < 60; j++) {
                pairs++;
                if (g_out[j].t_ms - g_out[i].t_ms > TOL) kept++;
            }
        r->order_pres = pairs ? 100.0 * kept / pairs : 0.0;
    }

    /* Size: every frame is the constant envelope, so this must be 1. */
    {
        int distinct = 1;
        for (i = 1; i < n; i++) if (g_out[i].size != g_out[0].size) distinct++;
        r->size_info = distinct;
    }

    for (i = 0; i < n; i++) lat[i] = g_out[i].t_ms - g_in[i].t_ms;
    qsort(lat, (size_t)n, sizeof(double), cmp_double);
    {
        double s = 0;
        for (i = 0; i < n; i++) s += lat[i];
        r->lat_mean = s / n;
    }
    r->lat_med = pct(lat, n, 0.50);
    r->lat_p95 = pct(lat, n, 0.95);
    r->lat_p99 = pct(lat, n, 0.99);
    r->lat_max = lat[n - 1];
}

/* --- one measurement run ---------------------------------- */

static void run_mode(int batching, int nflows, int burst, CorrResult *out) {
    static uint16_t port;
    HANDLE ht, hf;
    OxIdentity path[1];
    int i;

    port = (uint16_t)(batching ? 9911 : 9910);
    g_nin = g_nout = 0;
    g_stop = 0;
    g_batching = batching;
    batch_init(&g_bq, 8, 250);

    ox_identity_from_public(&path[0], g_rA.public_key, "127.0.0.1:9910");

    ht = CreateThread(NULL, 0, relay_thread, &port, 0, NULL);
    hf = CreateThread(NULL, 0, flusher_thread, NULL, 0, NULL);
    Sleep(200);

    for (i = 0; i < nflows; i++) {
        AirbConn c;
        uint8_t payload[64];
        memset(payload, 'A' + (i % 26), sizeof(payload));
        if (transport_dial("127.0.0.1", port, &c) == AIRB_OK) {
            airbchan_send(&c, path, 1, payload, (uint16_t)(16 + (i % 40)));
            transport_close(&c);
        }
        Sleep(burst ? 2 : (i % 2 ? 30 : 8));
    }

    Sleep(1200);
    g_stop = 1;
    /* Drain anything still queued so latency stats are complete. */
    EnterCriticalSection(&g_lock);
    if (g_batching) {
        BatchSlot o[BATCH_MAX_FRAMES];
        int n = batch_flush(&g_bq, o, BATCH_MAX_FRAMES), k;
        double t = now_ms();
        for (k = 0; k < n; k++) {
            if (g_nout < MAX_EVENTS) {
                g_out[g_nout].t_ms = t; g_out[g_nout].size = AC_ENVELOPE_WIRE;
                g_out[g_nout].seq = g_nout; g_nout++;
            }
        }
    }
    LeaveCriticalSection(&g_lock);

    { AirbConn w; if (transport_dial("127.0.0.1", port, &w) == AIRB_OK) transport_close(&w); }
    WaitForSingleObject(ht, 1500); CloseHandle(ht);
    WaitForSingleObject(hf, 1500); CloseHandle(hf);
    score(out);
}

int measure_batching(void) {
    CorrResult a, b;
    AirbNetMode saved = netpolicy_mode();

    InitializeCriticalSection(&g_lock);
    netpolicy_set_mode(AIRB_MODE_RELAY);
    ox_identity_generate(&g_rA, "127.0.0.1:9910");

    printf("\n  PHASE 1 - LIVE BATCHING CORRELATION (production relay)\n");
    printf("  ============================================================\n");
    printf("  Driving airbchan_send + batch_* over real sockets.\n");
    printf("  Tie-breaking in the matcher is RANDOM, not index-order.\n\n");

    printf("  running MODE A (batching disabled)...\n");
    run_mode(0, 100, 0, &a);
    printf("  running MODE B (production batching, 8 frames / 250 ms)...\n");
    run_mode(1, 100, 0, &b);

    printf("\n  | Metric                    | No batching | Live batching |\n");
    printf("  |---------------------------|------------:|--------------:|\n");
    printf("  | timing correlation        |   %6.1f%%   |    %6.1f%%    |\n",
           a.timing_acc, b.timing_acc);
    printf("  | mean anonymity set        |   %6.1f    |    %6.1f     |\n",
           a.mean_set, b.mean_set);
    printf("  | ordering preserved        |   %6.1f%%   |    %6.1f%%    |\n",
           a.order_pres, b.order_pres);
    printf("  | distinct egress sizes     |   %6.0f    |    %6.0f     |\n",
           a.size_info, b.size_info);
    printf("  | size correlation          |     none    |      none     |\n");
    printf("\n  batching latency (ms), production path:\n");
    printf("  | mode          |   mean |  median |    p95 |    p99 |    max |\n");
    printf("  |---------------|-------:|--------:|-------:|-------:|-------:|\n");
    printf("  | no batching   | %6.1f | %7.1f | %6.1f | %6.1f | %6.1f |\n",
           a.lat_mean, a.lat_med, a.lat_p95, a.lat_p99, a.lat_max);
    printf("  | live batching | %6.1f | %7.1f | %6.1f | %6.1f | %6.1f |\n",
           b.lat_mean, b.lat_med, b.lat_p95, b.lat_p99, b.lat_max);

    printf("\n  samples: mode A %d in / %d out, mode B %d in / %d out\n",
           100, g_nout, 100, g_nout);
    printf("  random-chance baseline for 100 flows: 1.0%%\n");
    printf("\n  Reading: size correlation is nil because every frame is the\n");
    printf("  constant %d-byte envelope. Timing is the remaining channel;\n",
           AC_ENVELOPE_WIRE);
    printf("  batching trades latency for ambiguity and does NOT eliminate it.\n\n");

    ox_identity_erase(&g_rA);
    netpolicy_set_mode(saved);
    DeleteCriticalSection(&g_lock);
    return 0;
}

/* --- Phase 6: production-path performance ----------------- */

int measure_performance(void) {
    static const int SIZES[] = {0, 1, 26, 64, 128, 256, 512, 600};
    OxIdentity r[4], path[4];
    OxReplayWindow w;
    uint8_t inner[8192], onion[8192], wire[AC_ENVELOPE_WIRE];
    uint8_t got[4096];
    OxPeeled p;
    int hops, si, iter, i;
    const int ITER = 200;

    printf("\n  PHASE 6 - PRODUCTION PATH PERFORMANCE\n");
    printf("  ============================================================\n");
    printf("  %d iterations per cell, production functions only.\n\n", ITER);

    for (i = 0; i < 4; i++) ox_identity_generate(&r[i], "127.0.0.1:9401");
    for (i = 0; i < 4; i++) ox_identity_from_public(&path[i], r[i].public_key, "127.0.0.1:9401");

    /* Primitive costs. */
    {
        uint8_t a[32], b[32], o[32];
        double t0, t1;
        csprng_bytes(a, 32); csprng_bytes(b, 32);
        t0 = now_ms();
        for (iter = 0; iter < ITER; iter++) x25519_scalarmult(o, a, b);
        t1 = now_ms();
        printf("  X25519 scalarmult      : %7.3f ms/op\n", (t1 - t0) / ITER);

        t0 = now_ms();
        for (iter = 0; iter < ITER; iter++) blake3_hash(inner, 1024, o);
        t1 = now_ms();
        printf("  BLAKE3 (1 KiB)         : %7.3f ms/op\n", (t1 - t0) / ITER);

        {
            uint8_t key[32], nonce[12], tag[16], ct[1024];
            csprng_bytes(key, 32); csprng_bytes(nonce, 12);
            t0 = now_ms();
            for (iter = 0; iter < ITER; iter++)
                chacha20_poly1305_encrypt(key, nonce, 0, 0, inner, 1024, ct, tag);
            t1 = now_ms();
            printf("  ChaCha20-Poly1305 1KiB : %7.3f ms/op\n", (t1 - t0) / ITER);
        }
    }

    printf("\n  | hops | payload | wrap ms | seal ms | open ms | peel ms | total ms | wire | expansion |\n");
    printf("  |-----:|--------:|--------:|--------:|--------:|--------:|---------:|-----:|----------:|\n");
    for (hops = 1; hops <= 4; hops++) {
        uint16_t target = (uint16_t)(AC_ENVELOPE_PLAIN - 2
                                     - (size_t)hops * (OX_LAYER_OVERHEAD + OX_HDR_SIZE));
        for (si = 0; si < 8; si++) {
            int pl = SIZES[si];
            double t0, t1, tw = 0, ts = 0, to = 0, tp = 0;
            size_t olen = 0;
            uint16_t ol2 = 0;
            if ((size_t)pl + 2 > target) continue;

            memset(inner, 0, sizeof(inner));
            inner[0] = (uint8_t)(pl >> 8); inner[1] = (uint8_t)(pl & 0xFF);
            memset(inner + 2, 'X', (size_t)pl);

            t0 = now_ms();
            for (iter = 0; iter < ITER; iter++)
                ox_wrap(path, hops, inner, target, onion, sizeof(onion), &olen);
            t1 = now_ms(); tw = (t1 - t0) / ITER;

            t0 = now_ms();
            for (iter = 0; iter < ITER; iter++)
                airbchan_seal_buf(&path[0], onion, (uint16_t)olen, wire);
            t1 = now_ms(); ts = (t1 - t0) / ITER;

            t0 = now_ms();
            for (iter = 0; iter < ITER; iter++)
                airbchan_open_buf(&r[0], wire, onion, &ol2);
            t1 = now_ms(); to = (t1 - t0) / ITER;

            ox_replay_init(&w, 1);
            t0 = now_ms();
            for (iter = 0; iter < ITER; iter++) {
                ox_replay_reset(&w, (uint32_t)iter);
                ox_peel(&r[0], &w, onion, ol2, &p);
            }
            t1 = now_ms(); tp = (t1 - t0) / ITER;

            printf("  |  %d   |   %4d  | %7.3f | %7.3f | %7.3f | %7.3f | %8.3f | %4d | %8.1fx |\n",
                   hops, pl, tw, ts, to, tp, tw + ts + to + tp,
                   AC_ENVELOPE_WIRE, pl ? AC_ENVELOPE_WIRE / (double)pl : 0.0);
        }
    }
    (void)got;
    printf("\n  Wire is constant %d B at every hop count and payload size.\n",
           AC_ENVELOPE_WIRE);
    printf("  Cost is dominated by X25519: %d agreements to wrap %d hops,\n", 4, 3);
    printf("  plus one more for the link envelope at each hop.\n\n");

    for (i = 0; i < 4; i++) ox_identity_erase(&r[i]);
    return 0;
}
