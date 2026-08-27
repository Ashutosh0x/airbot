/*
 * Airbot — Executable Information System
 * onionx.h — Per-hop onion encryption with forward secrecy
 *
 * Replaces the single shared pre-shared key. Under the PSK design any relay
 * holding the key could decrypt the application payload; here each relay can
 * remove only its own layer.
 *
 * ---------------------------------------------------------------------
 * KEY ESTABLISHMENT
 *
 * Each relay publishes a long-term X25519 identity public key. For every
 * message the client generates a FRESH ephemeral X25519 keypair per hop:
 *
 *     shared_i = X25519(eph_sk_i, relay_pk_i)
 *     k_i      = KDF(shared_i, "airbot-onion-v1-hopkey", eph_pk_i || relay_pk_i)
 *
 * The KDF is HKDF-SHA256 (RFC 5869). It was previously HKDF's shape over
 * HMAC-BLAKE3, which is not a standard instantiation and had no published
 * vectors to check against; it was replaced for that reason. No new
 * cryptographic primitive is introduced: X25519 (RFC 7748), HKDF-SHA256
 * (RFC 5869 over FIPS 180-4 / RFC 2104) and ChaCha20-Poly1305 (RFC 8439)
 * are all standard and verified against published vectors.
 *
 * FORWARD SECRECY. The ephemeral secret is erased immediately after the
 * shared secret is derived, and the derived hop key is erased after use.
 * Compromising a relay's LONG-TERM identity key afterwards does not recover
 * past hop keys, because recovering shared_i additionally requires the
 * ephemeral secret, which no longer exists anywhere.
 *
 * What forward secrecy does NOT protect:
 *   - traffic captured while the process still holds the keys in memory
 *   - a relay that was already compromised at the time of transmission
 *   - the endpoint itself (plaintext exists there before any encryption)
 *
 * ---------------------------------------------------------------------
 * LAYERING
 *
 *   client builds:  E_A( hdr_A || E_B( hdr_B || E_C( hdr_C || payload )))
 *
 *   relay A: removes its layer, learns next hop = B, forwards the rest.
 *   relay B: removes its layer, learns next hop = C.
 *   relay C: removes its layer, learns it is the exit and sees the payload.
 *
 * Relay A cannot decrypt B's or C's layer: it never possesses k_B or k_C.
 * Each relay learns ONLY its immediate predecessor (from the transport) and
 * its immediate successor (from its own header). No relay sees the full path.
 * The exit relay sees the payload — that is inherent to onion routing, which
 * is why the payload should carry its own end-to-end encryption when the
 * destination is not the exit.
 *
 * ---------------------------------------------------------------------
 * REPLAY PROTECTION
 *
 * Each layer header carries a 16-byte random message id and a session epoch.
 * A relay keeps a bounded sliding window of recently seen ids per epoch and
 * rejects duplicates. The id is INSIDE the encrypted layer, so it is not an
 * externally visible correlation token, and it is per-message random rather
 * than a persistent counter, so it cannot act as a tracking identifier
 * across sessions.
 */
#ifndef AIRBOT_ONIONX_H
#define AIRBOT_ONIONX_H

#include <stdint.h>
#include <stddef.h>

#define OX_MAX_HOPS        4
#define OX_KEY_SIZE       32
#define OX_NONCE_SIZE     12
#define OX_TAG_SIZE       16
#define OX_MSGID_SIZE     16
#define OX_ADDR_SIZE      18   /* next-hop label, opaque to other relays */

/* Per-layer cleartext header, encrypted under that hop's key. */
#define OX_HDR_SIZE       (1 + 1 + OX_MSGID_SIZE + OX_ADDR_SIZE + 2)
/*   flags(1) | hop_index(1) | msg_id(16) | next_addr(18) | inner_len(2)  */

#define OX_FLAG_MORE      0x01   /* another layer follows */
#define OX_FLAG_EXIT      0x02   /* this relay is the exit */

/* Per-layer wire overhead: ephemeral pubkey + nonce + tag. */
#define OX_LAYER_OVERHEAD (OX_KEY_SIZE + OX_NONCE_SIZE + OX_TAG_SIZE)

#define OX_OK               0
#define OX_ERR_HOPS       -80
#define OX_ERR_TOOBIG     -81
#define OX_ERR_AUTH       -82   /* layer failed authentication */
#define OX_ERR_REPLAY     -83   /* message id already seen this epoch */
#define OX_ERR_BADKEY     -84   /* degenerate X25519 result */
#define OX_ERR_MALFORMED  -85

/* A relay's long-term identity. */
typedef struct {
    uint8_t public_key[OX_KEY_SIZE];
    uint8_t secret_key[OX_KEY_SIZE];   /* zero on a client-side descriptor */
    uint8_t addr[OX_ADDR_SIZE];        /* transport label, e.g. "127.0.0.1:9401" */
    int     have_secret;
} OxIdentity;

/* Replay window. Session-scoped, bounded, and holds no persistent state. */
#define OX_REPLAY_WINDOW 512
typedef struct {
    uint8_t  ids[OX_REPLAY_WINDOW][OX_MSGID_SIZE];
    uint8_t  used[OX_REPLAY_WINDOW];
    uint32_t epoch;
    uint32_t count;
} OxReplayWindow;

/* Result of peeling one layer at a relay. */
typedef struct {
    uint8_t  is_exit;
    uint8_t  hop_index;
    uint8_t  next_addr[OX_ADDR_SIZE];
    uint8_t  msg_id[OX_MSGID_SIZE];
    uint8_t  inner[8192];
    uint16_t inner_len;
} OxPeeled;

/* --- identity lifecycle ----------------------------------- */

/* Generate a fresh long-term identity (relay side). */
void ox_identity_generate(OxIdentity *id, const char *addr);

/* Build a client-side descriptor from a relay's published public key. */
void ox_identity_from_public(OxIdentity *id, const uint8_t pk[OX_KEY_SIZE],
                             const char *addr);

/* Securely erase secret material. Call when a key leaves service. */
void ox_identity_erase(OxIdentity *id);

/* --- replay window ---------------------------------------- */

void ox_replay_init(OxReplayWindow *w, uint32_t epoch);
/* 1 if fresh (and records it), 0 if this id was already seen. */
int  ox_replay_check(OxReplayWindow *w, const uint8_t id[OX_MSGID_SIZE]);
/* Drop all state — used on session rotation so ids never persist. */
void ox_replay_reset(OxReplayWindow *w, uint32_t new_epoch);

/* --- onion construction ----------------------------------- */

/*
 * Wrap `payload` for the given path. path[0] is the first relay.
 * Ephemeral secrets are generated and erased inside this call.
 */
int ox_wrap(const OxIdentity *path, int num_hops,
            const uint8_t *payload, uint16_t payload_len,
            uint8_t *out, size_t out_cap, size_t *out_len);

/*
 * Remove exactly one layer using this relay's secret key.
 * Rejects a duplicate message id via `window` when non-NULL.
 *
 * Alias-safe: `wire` may point inside `out` (the usual relay pattern of
 * peeling repeatedly into one OxPeeled), because the wire is copied before
 * the output struct is cleared.
 */
int ox_peel(const OxIdentity *me, OxReplayWindow *window,
            const uint8_t *wire, size_t wire_len, OxPeeled *out);

/*
 * Derive a key from an X25519 agreement under an explicit domain-separation
 * label. Used by the transport for the link envelope key, which must be
 * independent of the onion hop key derived from the same relay identity.
 */
int ox_derive_labeled_key(const uint8_t scalar[32], const uint8_t point[32],
                          const uint8_t eph_pk[32], const uint8_t relay_pk[32],
                          const char *label, uint8_t key_out[32]);

const char *ox_strerror(int code);

/* Self-test: layering, per-hop key isolation, forward secrecy, replay. */
int ox_selftest(void);

#endif /* AIRBOT_ONIONX_H */
