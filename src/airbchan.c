/*
 * Airbot — Executable Information System
 * airbchan.c — The sanctioned application data path
 */
#include "airbchan.h"
#include "privframe.h"
#include "netpolicy.h"
#include "chacha20.h"
#include "x25519.h"

#include <stdio.h>
#include <string.h>

/* Per-hop onion overhead: ephemeral pubkey + nonce + tag + layer header. */
#define AC_HOP_OVERHEAD (OX_LAYER_OVERHEAD + OX_HDR_SIZE)
/* Length prefix carried inside the innermost layer. */
#define AC_LENPFX 2

const char *airbchan_strerror(int code) {
    switch (code) {
        case AC_OK:           return "ok";
        case AC_ERR_POLICY:   return "refused: would bypass the onion path";
        case AC_ERR_TOOBIG:   return "payload too large for the largest bucket";
        case AC_ERR_SEND:     return "send failed";
        case AC_ERR_RECV:     return "receive failed";
        case AC_ERR_BUCKET:   return "invalid bucket index";
        case AC_ERR_ONION:    return "onion construction or peel rejected";
        case AC_ERR_TRUNC:    return "truncated link frame";
        default:              return "unknown channel error";
    }
}

/* Smallest bucket that can hold `n` bytes of onion. */
static int bucket_index_for(size_t n) {
    int i;
    for (i = 0; i < PF_NUM_BUCKETS; i++)
        if (n <= PF_BUCKETS[i]) return i;
    return -1;
}

uint16_t airbchan_max_payload(int hops) {
    size_t biggest = PF_BUCKETS[PF_NUM_BUCKETS - 1];
    size_t overhead = (size_t)hops * AC_HOP_OVERHEAD + AC_LENPFX;
    if (overhead >= biggest) return 0;
    return (uint16_t)(biggest - overhead);
}

/* --- fixed-size link envelope ----------------------------- */

/*
 * Seal `onion` into a constant-size envelope addressed to `to`.
 * Outer transport padding lives INSIDE the AEAD, so it is authenticated and
 * cannot be stripped or extended without failing the tag.
 */
static int envelope_seal(const OxIdentity *to, const uint8_t *onion,
                         uint16_t onion_len, uint8_t *wire) {
    uint8_t eph_sk[32], eph_pk[32], key[32], nonce[12], tag[16];
    uint8_t plain[AC_ENVELOPE_PLAIN];
    int rc;

    if ((size_t)onion_len + 2 > AC_ENVELOPE_PLAIN) return AC_ERR_TOOBIG;

    csprng_bytes(eph_sk, 32);
    x25519_base(eph_pk, eph_sk);
    rc = ox_derive_labeled_key(eph_sk, to->public_key, eph_pk, to->public_key,
                               "airbot-link-v1", key);
    memset(eph_sk, 0, sizeof(eph_sk));          /* forward secrecy */
    if (rc != OX_OK) { memset(key, 0, 32); return AC_ERR_ENVELOPE; }

    /* len || onion || random padding, always the same total length */
    plain[0] = (uint8_t)(onion_len >> 8);
    plain[1] = (uint8_t)(onion_len & 0xFF);
    memcpy(plain + 2, onion, onion_len);
    csprng_bytes(plain + 2 + onion_len,
                 (size_t)(AC_ENVELOPE_PLAIN - 2 - onion_len));

    csprng_bytes(nonce, 12);
    memcpy(wire, eph_pk, 32);
    memcpy(wire + 32, nonce, 12);
    /* eph_pk as AAD binds the envelope to this ephemeral key. */
    if (chacha20_poly1305_encrypt(key, nonce, eph_pk, 32,
                                  plain, AC_ENVELOPE_PLAIN,
                                  wire + 44, tag) != 0) {
        memset(key, 0, 32); memset(plain, 0, sizeof(plain));
        return AC_ERR_ENVELOPE;
    }
    memcpy(wire + 44 + AC_ENVELOPE_PLAIN, tag, 16);
    memset(key, 0, sizeof(key));
    memset(plain, 0, sizeof(plain));
    return AC_OK;
}

/* Open an envelope addressed to us and recover the onion it carries. */
static int envelope_open(const OxIdentity *me, const uint8_t *wire,
                         uint8_t *onion, uint16_t *onion_len) {
    uint8_t key[32], plain[AC_ENVELOPE_PLAIN];
    uint16_t len;
    int rc;

    if (!me->have_secret) return AC_ERR_ENVELOPE;
    rc = ox_derive_labeled_key(me->secret_key, wire, wire, me->public_key,
                               "airbot-link-v1", key);
    if (rc != OX_OK) { memset(key, 0, 32); return AC_ERR_ENVELOPE; }

    if (chacha20_poly1305_decrypt(key, wire + 32, wire, 32,
                                  wire + 44, AC_ENVELOPE_PLAIN,
                                  wire + 44 + AC_ENVELOPE_PLAIN, plain) != 0) {
        memset(key, 0, 32); memset(plain, 0, sizeof(plain));
        return AC_ERR_ENVELOPE;
    }
    memset(key, 0, sizeof(key));

    len = (uint16_t)((plain[0] << 8) | plain[1]);
    if ((size_t)len + 2 > AC_ENVELOPE_PLAIN) {
        memset(plain, 0, sizeof(plain));
        return AC_ERR_ENVELOPE;
    }
    memcpy(onion, plain + 2, len);
    *onion_len = len;
    memset(plain, 0, sizeof(plain));
    return AC_OK;
}

/* --- send ------------------------------------------------- */

int airbchan_send(const AirbConn *conn, const OxIdentity *path, int hops,
                  const uint8_t *payload, uint16_t len) {
    uint8_t inner[8192], onion[8192], wire[AC_ENVELOPE_WIRE];
    size_t onion_len = 0;
    int rc;

    if (!conn || !path || hops < 1) return AC_ERR_SEND;
    if (len > airbchan_max_payload(hops)) return AC_ERR_TOOBIG;

    /* true_len || payload || random padding, sized so the onion is a constant
       for a given hop count regardless of the payload size. */
    {
        uint16_t target = (uint16_t)(AC_ENVELOPE_PLAIN - 2
                                     - (size_t)hops * AC_HOP_OVERHEAD);
        if ((size_t)len + AC_LENPFX > target) return AC_ERR_TOOBIG;
        memset(inner, 0, sizeof(inner));
        inner[0] = (uint8_t)(len >> 8);
        inner[1] = (uint8_t)(len & 0xFF);
        if (len) memcpy(inner + AC_LENPFX, payload, len);
        if (target > AC_LENPFX + len)
            csprng_bytes(inner + AC_LENPFX + len,
                         (size_t)(target - AC_LENPFX - len));
        rc = ox_wrap(path, hops, inner, target, onion, sizeof(onion), &onion_len);
    }
    if (rc != OX_OK) return AC_ERR_ONION;

    rc = envelope_seal(&path[0], onion, (uint16_t)onion_len, wire);
    if (rc != AC_OK) return rc;

    return transport_send_raw(conn, wire, (uint32_t)AC_ENVELOPE_WIRE) == AIRB_OK
           ? AC_OK : AC_ERR_SEND;
}

/* --- receive ---------------------------------------------- */

static int recv_exact(const AirbConn *c, uint8_t *b, uint32_t n) {
    uint32_t got = 0;
    while (got < n) {
        int r = transport_recv_raw(c, b + got, n - got);
        if (r <= 0) return -1;
        got += (uint32_t)r;
    }
    return 0;
}

int airbchan_recv(const AirbConn *conn, const OxIdentity *me,
                  OxReplayWindow *window, OxPeeled *out) {
    uint8_t wire[AC_ENVELOPE_WIRE], onion[8192];
    uint16_t onion_len = 0;
    int rc;

    if (!conn || !me || !out) return AC_ERR_RECV;
    if (recv_exact(conn, wire, AC_ENVELOPE_WIRE) != 0) return AC_ERR_TRUNC;
    rc = envelope_open(me, wire, onion, &onion_len);
    if (rc != AC_OK) return rc;
    if (ox_peel(me, window, onion, onion_len, out) != OX_OK) return AC_ERR_ONION;
    return AC_OK;
}

int airbchan_exit_payload(const OxPeeled *peeled,
                          uint8_t *out, size_t cap, uint16_t *out_len) {
    uint16_t true_len = 0;
    if (!peeled || !out || !out_len) return AC_ERR_RECV;
    if (peeled->inner_len < AC_LENPFX) return AC_ERR_TRUNC;
    true_len = (uint16_t)((peeled->inner[0] << 8) | peeled->inner[1]);
    if ((size_t)true_len + AC_LENPFX > peeled->inner_len) return AC_ERR_TRUNC;
    if ((size_t)true_len > cap) return AC_ERR_TOOBIG;
    memcpy(out, peeled->inner + AC_LENPFX, true_len);
    *out_len = true_len;
    return AC_OK;
}

/* Re-seal the peeled inner onion for the next relay. The outgoing envelope is
   the same constant size as the incoming one, so the frame does not shrink. */
int airbchan_forward_to(const AirbConn *conn, const OxIdentity *next,
                        const uint8_t *inner, uint16_t inner_len) {
    uint8_t wire[AC_ENVELOPE_WIRE];
    int rc;
    if (!conn || !next || !inner) return AC_ERR_SEND;
    rc = envelope_seal(next, inner, inner_len, wire);
    if (rc != AC_OK) return rc;
    return transport_send_raw(conn, wire, (uint32_t)AC_ENVELOPE_WIRE) == AIRB_OK
           ? AC_OK : AC_ERR_SEND;
}

/* Buffer-level helpers for tests that need the wire image without a socket. */
int airbchan_seal_buf(const OxIdentity *to, const uint8_t *onion,
                      uint16_t onion_len, uint8_t *wire) {
    return envelope_seal(to, onion, onion_len, wire);
}
int airbchan_open_buf(const OxIdentity *me, const uint8_t *wire,
                      uint8_t *onion, uint16_t *onion_len) {
    return envelope_open(me, wire, onion, onion_len);
}

/* --- self test -------------------------------------------- */

int airbchan_selftest(void) {
    OxIdentity rA, rB, rC, path[3];
    OxReplayWindow wA, wB, wC;
    uint8_t inner[8192], onion[8192];
    size_t onion_len = 0;
    OxPeeled p;
    uint8_t got[4096];
    uint16_t glen = 0;
    static const uint8_t msg[] = "LIVE-PATH-PAYLOAD";
    int fails = 0, bi;
    uint16_t target;

    ox_identity_generate(&rA, "127.0.0.1:9701");
    ox_identity_generate(&rB, "127.0.0.1:9702");
    ox_identity_generate(&rC, "127.0.0.1:9703");
    ox_identity_from_public(&path[0], rA.public_key, "127.0.0.1:9701");
    ox_identity_from_public(&path[1], rB.public_key, "127.0.0.1:9702");
    ox_identity_from_public(&path[2], rC.public_key, "127.0.0.1:9703");
    ox_replay_init(&wA, 1); ox_replay_init(&wB, 1); ox_replay_init(&wC, 1);

    /* Mirror airbchan_send's framing without a socket. */
    {
        size_t need = sizeof(msg) - 1 + AC_LENPFX + 3 * AC_HOP_OVERHEAD;
        bi = bucket_index_for(need);
        if (bi < 0) return 1;
        target = (uint16_t)(PF_BUCKETS[bi] - 3 * AC_HOP_OVERHEAD);
    }
    memset(inner, 0, sizeof(inner));
    inner[0] = 0; inner[1] = (uint8_t)(sizeof(msg) - 1);
    memcpy(inner + AC_LENPFX, msg, sizeof(msg) - 1);
    csprng_bytes(inner + AC_LENPFX + sizeof(msg) - 1,
                 target - AC_LENPFX - (sizeof(msg) - 1));

    if (ox_wrap(path, 3, inner, target, onion, sizeof(onion), &onion_len) != OX_OK)
        return 1;

    /* Onion must land exactly on a bucket. */
    if (bucket_index_for(onion_len) < 0) fails++;
    if (onion_len != PF_BUCKETS[bi]) fails++;

    /* Peel through all three hops. */
    if (ox_peel(&rA, &wA, onion, onion_len, &p) != OX_OK) fails++;
    if (p.is_exit) fails++;
    if (ox_peel(&rB, &wB, p.inner, p.inner_len, &p) != OX_OK) fails++;
    if (p.is_exit) fails++;
    if (ox_peel(&rC, &wC, p.inner, p.inner_len, &p) != OX_OK) fails++;
    if (!p.is_exit) fails++;

    /* Exit recovers the true payload, padding stripped. */
    if (airbchan_exit_payload(&p, got, sizeof(got), &glen) != AC_OK) fails++;
    if (glen != sizeof(msg) - 1) fails++;
    else if (memcmp(got, msg, glen) != 0) fails++;

    ox_identity_erase(&rA); ox_identity_erase(&rB); ox_identity_erase(&rC);
    return fails;
}
