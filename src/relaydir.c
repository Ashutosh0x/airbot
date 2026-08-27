/*
 * Airbot — Executable Information System
 * relaydir.c — Authenticated relay key distribution (fingerprint pinning)
 */
#include "relaydir.h"
#include "blake3.h"

#include <stdio.h>
#include <string.h>

void rd_fingerprint(const uint8_t pk[OX_KEY_SIZE], uint8_t out[RD_FP_SIZE]) {
    /* Domain-separated so a relay fingerprint can never collide with any
       other BLAKE3 use in the project (e.g. EIU content addressing). */
    Blake3State st;
    static const char LABEL[] = "airbot-relay-fingerprint-v1";
    blake3_init(&st);
    blake3_update(&st, (const uint8_t *)LABEL, sizeof(LABEL) - 1);
    blake3_update(&st, pk, OX_KEY_SIZE);
    blake3_finalize(&st, out);
}

void rd_pinset_init(RdPinSet *ps) {
    memset(ps, 0, sizeof(*ps));
}

int rd_pin_add(RdPinSet *ps, const char *addr, const uint8_t fp[RD_FP_SIZE]) {
    RdPin *p;
    size_t n;
    if (!ps || !addr || !fp) return RD_ERR_UNKNOWN;
    if (ps->count >= RD_MAX_ENTRIES) return RD_ERR_FULL;
    p = &ps->pins[ps->count];
    memset(p, 0, sizeof(*p));
    n = strlen(addr);
    if (n > RD_ADDR_SIZE - 1) n = RD_ADDR_SIZE - 1;
    memcpy(p->addr, addr, n);
    memcpy(p->fingerprint, fp, RD_FP_SIZE);
    p->min_generation = 0;
    p->used = 1;
    ps->count++;
    return RD_OK;
}

static RdPin *find_pin(RdPinSet *ps, const uint8_t addr[RD_ADDR_SIZE]) {
    int i;
    for (i = 0; i < ps->count; i++)
        if (ps->pins[i].used &&
            memcmp(ps->pins[i].addr, addr, RD_ADDR_SIZE) == 0)
            return &ps->pins[i];
    return 0;
}

/* Constant-time-ish comparison: no early exit on the first differing byte. */
static int fp_equal(const uint8_t a[RD_FP_SIZE], const uint8_t b[RD_FP_SIZE]) {
    uint8_t acc = 0;
    int i;
    for (i = 0; i < RD_FP_SIZE; i++) acc |= (uint8_t)(a[i] ^ b[i]);
    return acc == 0;
}

int rd_validate(RdPinSet *ps, const RdEntry *e, uint32_t now,
                uint8_t required_role, OxIdentity *out) {
    RdPin *pin;
    uint8_t fp[RD_FP_SIZE];

    if (!ps || !e || !out) return RD_ERR_UNKNOWN;

    /* Unknown relay: refuse. There is deliberately no trust-on-first-use
       path - accepting an unpinned key is exactly the substitution attack
       this module exists to stop. */
    pin = find_pin(ps, e->addr);
    if (!pin) return RD_ERR_UNKNOWN;

    /* Key substitution check. */
    rd_fingerprint(e->public_key, fp);
    if (!fp_equal(fp, pin->fingerprint)) return RD_ERR_FP;

    /* Validity window. */
    if (now < e->valid_from)  return RD_ERR_NOTYET;
    if (now >= e->valid_until) return RD_ERR_EXPIRED;

    /* Rollback: never accept a generation older than one already seen, so a
       revoked key cannot be replayed back into service. */
    if (e->generation < pin->min_generation) return RD_ERR_ROLLBACK;

    if (required_role && !(e->role & required_role)) return RD_ERR_ROLE;

    pin->min_generation = e->generation;

    memset(out, 0, sizeof(*out));
    memcpy(out->public_key, e->public_key, OX_KEY_SIZE);
    memcpy(out->addr, e->addr, RD_ADDR_SIZE);
    out->have_secret = 0;
    return RD_OK;
}

const char *rd_strerror(int code) {
    switch (code) {
        case RD_OK:           return "ok";
        case RD_ERR_UNKNOWN:  return "unknown relay: no pinned fingerprint (no TOFU)";
        case RD_ERR_FP:       return "fingerprint mismatch: public-key substitution";
        case RD_ERR_EXPIRED:  return "directory entry expired";
        case RD_ERR_NOTYET:   return "directory entry not yet valid";
        case RD_ERR_FULL:     return "pin set full";
        case RD_ERR_ROLE:     return "relay not authorised for this role";
        case RD_ERR_ROLLBACK: return "generation rollback refused";
        default:              return "unknown directory error";
    }
}

/* --- self test -------------------------------------------- */

static void mk_entry(RdEntry *e, const char *addr, const uint8_t pk[32],
                     uint8_t role, uint32_t from, uint32_t until, uint32_t gen) {
    size_t n = strlen(addr);
    memset(e, 0, sizeof(*e));
    if (n > RD_ADDR_SIZE - 1) n = RD_ADDR_SIZE - 1;
    memcpy(e->addr, addr, n);
    memcpy(e->public_key, pk, 32);
    e->role = role;
    e->valid_from = from;
    e->valid_until = until;
    e->generation = gen;
}

int rd_selftest(void) {
    OxIdentity real, attacker, out;
    RdPinSet ps;
    RdEntry e;
    uint8_t fp[RD_FP_SIZE];
    int fails = 0;

    ox_identity_generate(&real, "127.0.0.1:9401");
    ox_identity_generate(&attacker, "127.0.0.1:9401");

    rd_pinset_init(&ps);
    rd_fingerprint(real.public_key, fp);
    rd_pin_add(&ps, "127.0.0.1:9401", fp);

    /* 1. genuine key, in window, correct role */
    mk_entry(&e, "127.0.0.1:9401", real.public_key, RD_ROLE_RELAY, 100, 200, 1);
    if (rd_validate(&ps, &e, 150, RD_ROLE_RELAY, &out) != RD_OK) fails++;
    if (memcmp(out.public_key, real.public_key, 32) != 0) fails++;

    /* 2. SUBSTITUTED key at the same address must be refused */
    mk_entry(&e, "127.0.0.1:9401", attacker.public_key, RD_ROLE_RELAY, 100, 200, 1);
    if (rd_validate(&ps, &e, 150, RD_ROLE_RELAY, &out) != RD_ERR_FP) fails++;

    /* 3. single flipped bit in the key must be refused */
    {
        uint8_t tweaked[32];
        memcpy(tweaked, real.public_key, 32);
        tweaked[7] ^= 0x01;
        mk_entry(&e, "127.0.0.1:9401", tweaked, RD_ROLE_RELAY, 100, 200, 1);
        if (rd_validate(&ps, &e, 150, RD_ROLE_RELAY, &out) != RD_ERR_FP) fails++;
    }

    /* 4. unknown relay must be refused (no TOFU) */
    mk_entry(&e, "127.0.0.1:9999", real.public_key, RD_ROLE_RELAY, 100, 200, 1);
    if (rd_validate(&ps, &e, 150, RD_ROLE_RELAY, &out) != RD_ERR_UNKNOWN) fails++;

    /* 5. expired */
    mk_entry(&e, "127.0.0.1:9401", real.public_key, RD_ROLE_RELAY, 100, 200, 1);
    if (rd_validate(&ps, &e, 250, RD_ROLE_RELAY, &out) != RD_ERR_EXPIRED) fails++;

    /* 6. not yet valid */
    if (rd_validate(&ps, &e, 50, RD_ROLE_RELAY, &out) != RD_ERR_NOTYET) fails++;

    /* 7. wrong role */
    mk_entry(&e, "127.0.0.1:9401", real.public_key, RD_ROLE_RELAY, 100, 200, 1);
    if (rd_validate(&ps, &e, 150, RD_ROLE_EXIT, &out) != RD_ERR_ROLE) fails++;

    /* 8. rotation: a NEW key must be re-pinned, not silently accepted */
    {
        OxIdentity rotated;
        uint8_t fp2[RD_FP_SIZE];
        ox_identity_generate(&rotated, "127.0.0.1:9401");
        mk_entry(&e, "127.0.0.1:9401", rotated.public_key, RD_ROLE_RELAY, 100, 200, 2);
        if (rd_validate(&ps, &e, 150, RD_ROLE_RELAY, &out) != RD_ERR_FP) fails++;

        rd_pinset_init(&ps);
        rd_fingerprint(rotated.public_key, fp2);
        rd_pin_add(&ps, "127.0.0.1:9401", fp2);
        if (rd_validate(&ps, &e, 150, RD_ROLE_RELAY, &out) != RD_OK) fails++;

        /* 9. rollback to the older generation must be refused */
        mk_entry(&e, "127.0.0.1:9401", rotated.public_key, RD_ROLE_RELAY, 100, 200, 1);
        if (rd_validate(&ps, &e, 150, RD_ROLE_RELAY, &out) != RD_ERR_ROLLBACK) fails++;
        ox_identity_erase(&rotated);
    }

    ox_identity_erase(&real);
    ox_identity_erase(&attacker);
    return fails;
}
