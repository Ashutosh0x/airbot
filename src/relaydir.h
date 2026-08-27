/*
 * Airbot — Executable Information System
 * relaydir.h — Authenticated relay key distribution
 *
 * THREAT BEING CLOSED
 *
 * onionx derives per-hop keys from a relay's published X25519 public key. If
 * an attacker can substitute that public key, they derive the hop key too and
 * the entire layering collapses. Nothing previously authenticated those keys.
 *
 * TRUST MODEL: EXPLICIT FINGERPRINT PINNING. No TOFU.
 *
 * The client is configured out-of-band with a BLAKE3-256 fingerprint for each
 * relay it will use. A directory entry is accepted only if the fingerprint of
 * its public key matches a pinned value AND the entry is inside its validity
 * window. An unknown relay is refused - it is never trusted on first use.
 *
 * This is deliberately NOT a PKI and introduces no new cryptography. It is the
 * same model as Tor bridge fingerprints or SSH host-key pinning with strict
 * checking. Its limits are stated plainly:
 *
 *   - Security rests entirely on the out-of-band fingerprint distribution.
 *     If an attacker controls that channel, they control the trust anchor.
 *   - There is no signed consensus, no revocation feed, and no directory
 *     authority. Rotation is manual: pin the new fingerprint, drop the old.
 *   - Expiry is enforced against a caller-supplied time. A client with a
 *     wrong clock can accept a stale entry.
 */
#ifndef AIRBOT_RELAYDIR_H
#define AIRBOT_RELAYDIR_H

#include <stdint.h>
#include <stddef.h>
#include "onionx.h"

#define RD_FP_SIZE       32
#define RD_MAX_ENTRIES   16
#define RD_ADDR_SIZE     OX_ADDR_SIZE

#define RD_OK              0
#define RD_ERR_UNKNOWN   -120  /* no pinned fingerprint for this relay */
#define RD_ERR_FP        -121  /* fingerprint mismatch: key substitution */
#define RD_ERR_EXPIRED   -122  /* outside the validity window */
#define RD_ERR_NOTYET    -123  /* not valid yet */
#define RD_ERR_FULL      -124
#define RD_ERR_ROLE      -125  /* entry is not usable in the requested role */
#define RD_ERR_ROLLBACK  -126  /* older generation than one already accepted */

#define RD_ROLE_RELAY  1
#define RD_ROLE_EXIT   2

typedef struct {
    uint8_t  addr[RD_ADDR_SIZE];
    uint8_t  public_key[OX_KEY_SIZE];
    uint8_t  role;
    uint32_t valid_from;
    uint32_t valid_until;
    uint32_t generation;      /* increases on rotation; rollback is refused */
} RdEntry;

typedef struct {
    uint8_t  addr[RD_ADDR_SIZE];
    uint8_t  fingerprint[RD_FP_SIZE];
    uint32_t min_generation;  /* highest generation accepted so far */
    int      used;
} RdPin;

typedef struct {
    RdPin pins[RD_MAX_ENTRIES];
    int   count;
} RdPinSet;

/* BLAKE3-256 over the public key, domain-separated. */
void rd_fingerprint(const uint8_t pk[OX_KEY_SIZE], uint8_t out[RD_FP_SIZE]);

void rd_pinset_init(RdPinSet *ps);
int  rd_pin_add(RdPinSet *ps, const char *addr, const uint8_t fp[RD_FP_SIZE]);

/*
 * Validate a directory entry against the pin set and produce a usable
 * OxIdentity. Fails closed on every anomaly. `now` is caller-supplied so
 * expiry is testable.
 */
int  rd_validate(RdPinSet *ps, const RdEntry *e, uint32_t now,
                 uint8_t required_role, OxIdentity *out);

const char *rd_strerror(int code);

/* Self-test: valid, unknown, substituted, expired, not-yet, wrong role,
   rotation, rollback. */
int rd_selftest(void);

#endif /* AIRBOT_RELAYDIR_H */
