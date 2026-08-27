/*
 * Airbot — Executable Information System
 * onionx.c — Per-hop onion encryption with forward secrecy
 */
#include "onionx.h"
#include "x25519.h"
#include "blake3.h"
#include "chacha20.h"

#include <stdio.h>
#include <string.h>

/* --- secure erase ----------------------------------------- */

/* Volatile pointer so the compiler cannot elide the clear as dead stores.
   This is best-effort: it does not defeat register spills, swap files or a
   privileged local observer. See SECURITY.md endpoint section. */
static void ox_secure_zero(void *p, size_t n) {
    volatile uint8_t *v = (volatile uint8_t *)p;
    while (n--) *v++ = 0;
}

/* --- HMAC-BLAKE3 / HKDF (RFC 5869) ------------------------ */

#define B3_BLOCKSIZE 64

static void hmac_blake3(const uint8_t *key, size_t key_len,
                        const uint8_t *msg, size_t msg_len,
                        uint8_t out[32]) {
    uint8_t k[B3_BLOCKSIZE], ipad[B3_BLOCKSIZE], opad[B3_BLOCKSIZE];
    uint8_t inner[32];
    Blake3State st;
    size_t i;

    memset(k, 0, sizeof(k));
    if (key_len > B3_BLOCKSIZE) blake3_hash(key, key_len, k);
    else if (key_len) memcpy(k, key, key_len);

    for (i = 0; i < B3_BLOCKSIZE; i++) {
        ipad[i] = (uint8_t)(k[i] ^ 0x36);
        opad[i] = (uint8_t)(k[i] ^ 0x5c);
    }

    blake3_init(&st);
    blake3_update(&st, ipad, B3_BLOCKSIZE);
    blake3_update(&st, msg, msg_len);
    blake3_finalize(&st, inner);

    blake3_init(&st);
    blake3_update(&st, opad, B3_BLOCKSIZE);
    blake3_update(&st, inner, 32);
    blake3_finalize(&st, out);

    ox_secure_zero(k, sizeof(k));
    ox_secure_zero(ipad, sizeof(ipad));
    ox_secure_zero(opad, sizeof(opad));
}

/* HKDF extract-then-expand, single 32-byte output block. */
static void hkdf32(const uint8_t *ikm, size_t ikm_len,
                   const uint8_t *salt, size_t salt_len,
                   const char *info, uint8_t out[32]) {
    uint8_t prk[32];
    uint8_t buf[256];
    size_t ilen = strlen(info), n = 0;

    hmac_blake3(salt, salt_len, ikm, ikm_len, prk);

    if (ilen > sizeof(buf) - 1) ilen = sizeof(buf) - 1;
    memcpy(buf, info, ilen); n = ilen;
    buf[n++] = 0x01;                     /* T(1) counter per RFC 5869 */
    hmac_blake3(prk, 32, buf, n, out);

    ox_secure_zero(prk, sizeof(prk));
}

/*
 * Derive the hop key. Domain-separated by a fixed label, and bound to both
 * public keys so a key derived for one relay cannot be reused for another.
 */
static int derive_hop_key(const uint8_t scalar[32], const uint8_t point[32],
                          const uint8_t eph_pk[32], const uint8_t relay_pk[32],
                          uint8_t key_out[32]) {
    uint8_t shared[32], salt[64];

    /* Both sides compute the same shared secret from opposite directions:
       the client uses (eph_sk, relay_pk), the relay uses (relay_sk, eph_pk).
       The salt must therefore be built from the same two public keys in the
       same order on both sides, not from whichever happened to be local. */
    x25519_scalarmult(shared, scalar, point);
    /* RFC 7748 §6.1: a zero shared secret means a small-order point was
       supplied. Reject rather than proceeding with a degenerate key. */
    if (x25519_is_zero(shared)) { ox_secure_zero(shared, 32); return OX_ERR_BADKEY; }

    memcpy(salt, eph_pk, 32);
    memcpy(salt + 32, relay_pk, 32);
    hkdf32(shared, 32, salt, sizeof(salt), "airbot-onion-v1-hopkey", key_out);

    ox_secure_zero(shared, sizeof(shared));   /* forward secrecy */
    return OX_OK;
}

/*
 * Public wrapper so the transport can derive a LINK key with its own domain
 * separation label. Separate label => a key that is independent of the onion
 * hop key, even though both come from the same relay identity.
 */
int ox_derive_labeled_key(const uint8_t scalar[32], const uint8_t point[32],
                          const uint8_t eph_pk[32], const uint8_t relay_pk[32],
                          const char *label, uint8_t key_out[32]) {
    uint8_t shared[32], salt[64];
    x25519_scalarmult(shared, scalar, point);
    if (x25519_is_zero(shared)) { ox_secure_zero(shared, 32); return OX_ERR_BADKEY; }
    memcpy(salt, eph_pk, 32);
    memcpy(salt + 32, relay_pk, 32);
    hkdf32(shared, 32, salt, sizeof(salt), label, key_out);
    ox_secure_zero(shared, sizeof(shared));
    return OX_OK;
}

/* --- identity --------------------------------------------- */

void ox_identity_generate(OxIdentity *id, const char *addr) {
    memset(id, 0, sizeof(*id));
    csprng_bytes(id->secret_key, OX_KEY_SIZE);
    x25519_base(id->public_key, id->secret_key);
    id->have_secret = 1;
    if (addr) {
        size_t n = strlen(addr);
        if (n > OX_ADDR_SIZE - 1) n = OX_ADDR_SIZE - 1;
        memcpy(id->addr, addr, n);
    }
}

void ox_identity_from_public(OxIdentity *id, const uint8_t pk[OX_KEY_SIZE],
                             const char *addr) {
    memset(id, 0, sizeof(*id));
    memcpy(id->public_key, pk, OX_KEY_SIZE);
    id->have_secret = 0;
    if (addr) {
        size_t n = strlen(addr);
        if (n > OX_ADDR_SIZE - 1) n = OX_ADDR_SIZE - 1;
        memcpy(id->addr, addr, n);
    }
}

void ox_identity_erase(OxIdentity *id) {
    if (!id) return;
    ox_secure_zero(id->secret_key, OX_KEY_SIZE);
    id->have_secret = 0;
}

/* --- replay window ---------------------------------------- */

void ox_replay_init(OxReplayWindow *w, uint32_t epoch) {
    memset(w, 0, sizeof(*w));
    w->epoch = epoch;
}

void ox_replay_reset(OxReplayWindow *w, uint32_t new_epoch) {
    /* Session rotation drops every recorded id, so nothing survives to act
       as a cross-session identifier. */
    memset(w->ids, 0, sizeof(w->ids));
    memset(w->used, 0, sizeof(w->used));
    w->count = 0;
    w->epoch = new_epoch;
}

int ox_replay_check(OxReplayWindow *w, const uint8_t id[OX_MSGID_SIZE]) {
    uint32_t h = 0, slot, i;
    /* Index by the id's own bytes; linear probe on collision. */
    for (i = 0; i < 4; i++) h = (h << 8) | id[i];
    slot = h % OX_REPLAY_WINDOW;

    for (i = 0; i < OX_REPLAY_WINDOW; i++) {
        uint32_t s = (slot + i) % OX_REPLAY_WINDOW;
        if (!w->used[s]) {
            memcpy(w->ids[s], id, OX_MSGID_SIZE);
            w->used[s] = 1;
            w->count++;
            /* Bounded memory: once full, rotate rather than grow. A rotation
               can admit a very old replay, which is the documented trade-off
               against unbounded state. */
            if (w->count >= OX_REPLAY_WINDOW) ox_replay_reset(w, w->epoch + 1);
            return 1;
        }
        if (memcmp(w->ids[s], id, OX_MSGID_SIZE) == 0) return 0;  /* replay */
    }
    return 0;
}

const char *ox_strerror(int code) {
    switch (code) {
        case OX_OK:            return "ok";
        case OX_ERR_HOPS:      return "invalid hop count";
        case OX_ERR_TOOBIG:    return "onion exceeds buffer";
        case OX_ERR_AUTH:      return "layer authentication failed";
        case OX_ERR_REPLAY:    return "replayed message id rejected";
        case OX_ERR_BADKEY:    return "degenerate X25519 shared secret";
        case OX_ERR_MALFORMED: return "malformed onion layer";
        default:               return "unknown onion error";
    }
}

/* --- wrap ------------------------------------------------- */

/*
 * Build layers from the innermost hop outward, so the outermost layer is the
 * one the first relay can open.
 */
int ox_wrap(const OxIdentity *path, int num_hops,
            const uint8_t *payload, uint16_t payload_len,
            uint8_t *out, size_t out_cap, size_t *out_len) {
    uint8_t bufA[8192], bufB[8192];
    uint8_t *cur = bufA, *nxt = bufB;
    uint8_t msg_id[OX_MSGID_SIZE];
    size_t cur_len;
    int hop, rc;

    if (num_hops < 1 || num_hops > OX_MAX_HOPS) return OX_ERR_HOPS;
    if (payload_len > 4096) return OX_ERR_TOOBIG;

    /* Per-message random seed. The id each relay actually sees is DERIVED
       from this seed under that hop's own key, so two colluding relays hold
       unrelated ids and cannot confirm they handled the same message by
       comparing them. The seed itself never appears on the wire. */
    csprng_bytes(msg_id, OX_MSGID_SIZE);

    memcpy(cur, payload, payload_len);
    cur_len = payload_len;

    for (hop = num_hops - 1; hop >= 0; hop--) {
        uint8_t eph_sk[32], eph_pk[32], key[32], nonce[OX_NONCE_SIZE], tag[OX_TAG_SIZE];
        uint8_t plain[8192];
        size_t plain_len, n = 0;

        /* Fresh ephemeral keypair per hop, per message. */
        csprng_bytes(eph_sk, 32);
        x25519_base(eph_pk, eph_sk);

        rc = derive_hop_key(eph_sk, path[hop].public_key,
                            eph_pk, path[hop].public_key, key);
        ox_secure_zero(eph_sk, sizeof(eph_sk));       /* forward secrecy */
        if (rc != OX_OK) { ox_secure_zero(key, 32); return rc; }

        /* Layer header + whatever this hop must forward. */
        plain[n++] = (uint8_t)(hop == num_hops - 1 ? OX_FLAG_EXIT : OX_FLAG_MORE);
        plain[n++] = (uint8_t)hop;
        {
            /* hop_id = HKDF(seed, key_i, "airbot-onion-v1-msgid") */
            uint8_t hop_id[32];
            hkdf32(msg_id, OX_MSGID_SIZE, key, 32, "airbot-onion-v1-msgid", hop_id);
            memcpy(plain + n, hop_id, OX_MSGID_SIZE);
            ox_secure_zero(hop_id, sizeof(hop_id));
        }
        n += OX_MSGID_SIZE;
        /* Next-hop label: the following relay's address, or zero at the exit.
           A relay therefore learns only its immediate successor. */
        if (hop + 1 < num_hops) memcpy(plain + n, path[hop + 1].addr, OX_ADDR_SIZE);
        else memset(plain + n, 0, OX_ADDR_SIZE);
        n += OX_ADDR_SIZE;
        plain[n++] = (uint8_t)(cur_len >> 8);
        plain[n++] = (uint8_t)(cur_len & 0xFF);
        memcpy(plain + n, cur, cur_len);
        plain_len = n + cur_len;

        csprng_bytes(nonce, OX_NONCE_SIZE);

        /* eph_pk || nonce || ciphertext || tag.
           The ephemeral public key is AAD so it cannot be swapped. */
        n = 0;
        memcpy(nxt + n, eph_pk, 32); n += 32;
        memcpy(nxt + n, nonce, OX_NONCE_SIZE); n += OX_NONCE_SIZE;
        if (chacha20_poly1305_encrypt(key, nonce, eph_pk, 32,
                                      plain, plain_len, nxt + n, tag) != 0) {
            ox_secure_zero(key, 32); ox_secure_zero(plain, sizeof(plain));
            return OX_ERR_TOOBIG;
        }
        n += plain_len;
        memcpy(nxt + n, tag, OX_TAG_SIZE); n += OX_TAG_SIZE;

        ox_secure_zero(key, sizeof(key));             /* hop key erased after use */
        ox_secure_zero(plain, plain_len);

        { uint8_t *t = cur; cur = nxt; nxt = t; }
        cur_len = n;
        if (cur_len > sizeof(bufA) - 512) return OX_ERR_TOOBIG;
    }

    if (cur_len > out_cap) return OX_ERR_TOOBIG;
    memcpy(out, cur, cur_len);
    *out_len = cur_len;
    ox_secure_zero(msg_id, sizeof(msg_id));
    return OX_OK;
}

/* --- peel ------------------------------------------------- */

int ox_peel(const OxIdentity *me, OxReplayWindow *window,
            const uint8_t *wire, size_t wire_len, OxPeeled *out) {
    uint8_t key[32], plain[8192], local[8192];
    const uint8_t *eph_pk, *nonce, *ct, *tag;
    size_t ct_len;
    uint16_t inner_len;
    int rc;

    if (!me || !me->have_secret || !out) return OX_ERR_MALFORMED;
    if (wire_len < OX_LAYER_OVERHEAD + OX_HDR_SIZE) return OX_ERR_MALFORMED;
    if (wire_len > sizeof(local)) return OX_ERR_MALFORMED;

    /* ALIAS SAFETY: a relay naturally writes
     *     ox_peel(me, w, p.inner, p.inner_len, &p);
     * reusing one OxPeeled for the whole chain. Clearing `out` first would
     * then zero the very bytes we are about to read. Copy the wire out of
     * harm's way before touching the output struct. */
    memcpy(local, wire, wire_len);
    memset(out, 0, sizeof(*out));

    eph_pk = local;
    nonce  = local + 32;
    ct     = local + 32 + OX_NONCE_SIZE;
    ct_len = wire_len - 32 - OX_NONCE_SIZE - OX_TAG_SIZE;
    tag    = local + wire_len - OX_TAG_SIZE;

    if (ct_len < OX_HDR_SIZE || ct_len > sizeof(plain)) return OX_ERR_MALFORMED;

    /* Same derivation from the other side: our long-term secret against the
       sender's ephemeral public key. */
    rc = derive_hop_key(me->secret_key, eph_pk, eph_pk, me->public_key, key);
    if (rc != OX_OK) { ox_secure_zero(key, 32); return rc; }

    if (chacha20_poly1305_decrypt(key, nonce, eph_pk, 32,
                                  ct, ct_len, tag, plain) != 0) {
        ox_secure_zero(key, sizeof(key));
        ox_secure_zero(plain, sizeof(plain));
        return OX_ERR_AUTH;
    }
    ox_secure_zero(key, sizeof(key));

    out->is_exit   = (plain[0] & OX_FLAG_EXIT) ? 1 : 0;
    out->hop_index = plain[1];
    memcpy(out->msg_id, plain + 2, OX_MSGID_SIZE);
    memcpy(out->next_addr, plain + 2 + OX_MSGID_SIZE, OX_ADDR_SIZE);
    inner_len = (uint16_t)((plain[OX_HDR_SIZE - 2] << 8) | plain[OX_HDR_SIZE - 1]);

    if ((size_t)inner_len + OX_HDR_SIZE > ct_len) {
        ox_secure_zero(plain, sizeof(plain));
        return OX_ERR_MALFORMED;
    }

    /* Replay check AFTER authentication, so an unauthenticated frame can
       never poison the window. */
    if (window && !ox_replay_check(window, out->msg_id)) {
        ox_secure_zero(plain, sizeof(plain));
        return OX_ERR_REPLAY;
    }

    memcpy(out->inner, plain + OX_HDR_SIZE, inner_len);
    out->inner_len = inner_len;
    ox_secure_zero(plain, sizeof(plain));
    return OX_OK;
}

/* --- self test -------------------------------------------- */

int ox_selftest(void) {
    OxIdentity relayA, relayB, relayC, path[3];
    OxReplayWindow wA, wB, wC;
    uint8_t onion[8192];
    size_t onion_len = 0;
    OxPeeled pA, pB, pC;
    static const uint8_t secret[] = "TOP-SECRET-EIU-PAYLOAD";
    int fails = 0, rc;

    ox_identity_generate(&relayA, "127.0.0.1:9401");
    ox_identity_generate(&relayB, "127.0.0.1:9402");
    ox_identity_generate(&relayC, "127.0.0.1:9403");

    ox_identity_from_public(&path[0], relayA.public_key, "127.0.0.1:9401");
    ox_identity_from_public(&path[1], relayB.public_key, "127.0.0.1:9402");
    ox_identity_from_public(&path[2], relayC.public_key, "127.0.0.1:9403");

    ox_replay_init(&wA, 1); ox_replay_init(&wB, 1); ox_replay_init(&wC, 1);

    if (ox_wrap(path, 3, secret, (uint16_t)sizeof(secret) - 1,
                onion, sizeof(onion), &onion_len) != OX_OK) return 1;

    /* Payload must not be readable in the wrapped onion. */
    { size_t i; int found = 0;
      for (i = 0; i + sizeof(secret) - 1 <= onion_len; i++)
          if (memcmp(onion + i, secret, sizeof(secret) - 1) == 0) found = 1;
      if (found) fails++; }

    /* Relay A peels its layer only. */
    if (ox_peel(&relayA, &wA, onion, onion_len, &pA) != OX_OK) fails++;
    if (pA.is_exit) fails++;

    /* Relay A must NOT be able to read the payload it forwards. */
    { size_t i; int found = 0;
      for (i = 0; i + sizeof(secret) - 1 <= pA.inner_len; i++)
          if (memcmp(pA.inner + i, secret, sizeof(secret) - 1) == 0) found = 1;
      if (found) fails++; }

    /* Relay A must not be able to peel B's layer with its own key. */
    if (ox_peel(&relayA, NULL, pA.inner, pA.inner_len, &pB) != OX_ERR_AUTH) fails++;

    /* Relay B peels correctly. */
    if (ox_peel(&relayB, &wB, pA.inner, pA.inner_len, &pB) != OX_OK) fails++;
    if (pB.is_exit) fails++;

    /* Relay C is the exit and recovers the payload. */
    if (ox_peel(&relayC, &wC, pB.inner, pB.inner_len, &pC) != OX_OK) fails++;
    if (!pC.is_exit) fails++;
    if (pC.inner_len != sizeof(secret) - 1) fails++;
    else if (memcmp(pC.inner, secret, pC.inner_len) != 0) fails++;

    /* Replay of the same outer layer must be rejected at A. */
    rc = ox_peel(&relayA, &wA, onion, onion_len, &pA);
    if (rc != OX_ERR_REPLAY) fails++;

    /* Two wraps of identical plaintext must differ on the wire. */
    { uint8_t o2[8192]; size_t l2 = 0;
      ox_wrap(path, 3, secret, (uint16_t)sizeof(secret) - 1, o2, sizeof(o2), &l2);
      if (l2 == onion_len && memcmp(o2, onion, l2) == 0) fails++; }

    ox_identity_erase(&relayA);
    ox_identity_erase(&relayB);
    ox_identity_erase(&relayC);
    return fails;
}
