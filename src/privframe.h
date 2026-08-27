/*
 * Airbot — Executable Information System
 * privframe.h — Padded AEAD wire frame for privacy mode
 *
 * Replaces the cleartext AIRB frame on the privacy path. The plaintext
 * frame put "AIRB", a version byte, a message type, a hop counter and a
 * stable BLAKE3 content digest on the wire at fixed offsets — a five-byte
 * DPI signature plus a cross-hop correlation beacon. This format removes
 * all of it from the visible layer.
 *
 * Wire layout:
 *
 *   off  size  field        visible to a passive observer
 *   0    2     outer_len    yes — but quantized to one of six bucket sizes
 *   2    12    nonce        yes — uniformly random, carries no information
 *   14   B     ciphertext   yes as bytes, opaque without the key
 *   14+B 16    tag          yes — Poly1305 tag, changes every frame
 *
 * Inner plaintext (encrypted, never visible on the wire):
 *
 *   0    1     type
 *   1    1     hops_left
 *   2    2     real_len
 *   4    N     payload
 *   4+N  pad   zero padding out to the bucket size
 *
 * Properties this buys, and their limits:
 *
 *  - NO PROTOCOL MAGIC. Every visible byte after the length prefix is
 *    ciphertext or a random nonce. There is no fixed value at a fixed
 *    offset to signature-match on. This raises the cost of identifying
 *    Airbot traffic; it does not make it unidentifiable, since the bucket
 *    size distribution and connection behaviour remain observable.
 *
 *  - NO STABLE PER-MESSAGE IDENTIFIER. The nonce is fresh CSPRNG output per
 *    frame, so the same EIU sent twice produces entirely different bytes and
 *    a different tag. This removes the digest-matching correlation that the
 *    plaintext frame allowed between hops. It does NOT defeat correlation by
 *    size or timing.
 *
 *  - HOP COUNTER HIDDEN. hops_left moves inside the ciphertext, so a relay
 *    learns its own position only, not the circuit length.
 *
 *  - SIZE QUANTIZATION. Payloads are padded up to the next bucket, so exact
 *    payload length is not revealed — only which bucket it fell in. Padding
 *    costs bandwidth and does not hide timing; see SECURITY.md.
 *
 * Integrity note: authentication here is Poly1305 from the RFC 8439 AEAD,
 * which passes the published test vectors. The project's blake3.c does NOT
 * match the official BLAKE3 vectors and is therefore not used as a security
 * primitive on this path. It is retained for EIU content addressing.
 */
#ifndef AIRBOT_PRIVFRAME_H
#define AIRBOT_PRIVFRAME_H

#include <stdint.h>
#include <stddef.h>
#include "transport.h"

#define PF_NONCE_SIZE   12
#define PF_TAG_SIZE     16
#define PF_LENPFX_SIZE   2
#define PF_INNER_HDR     4
#define PF_OVERHEAD     (PF_LENPFX_SIZE + PF_NONCE_SIZE + PF_TAG_SIZE)

/* Size buckets. A payload is padded up to the smallest bucket that fits it,
   so outer_len takes one of six values rather than tracking payload size. */
#define PF_NUM_BUCKETS   6
extern const uint16_t PF_BUCKETS[PF_NUM_BUCKETS];

#define PF_MAX_PAYLOAD  (8192 - PF_INNER_HDR)

#define PF_OK             0
#define PF_ERR_NOKEY    -60   /* no session key configured */
#define PF_ERR_TOOBIG   -61   /* payload exceeds the largest bucket */
#define PF_ERR_SEND     -62
#define PF_ERR_RECV     -63
#define PF_ERR_AUTH     -64   /* Poly1305 tag rejected: forged or corrupted */
#define PF_ERR_MALFORMED -65  /* length prefix implausible */

typedef struct {
    uint8_t  type;
    uint8_t  hops_left;
    uint16_t length;
    uint8_t  payload[PF_MAX_PAYLOAD];
    uint16_t bucket;          /* which bucket carried it, for measurement */
} PrivFrame;

/*
 * Install the 32-byte session key. Read from AIRBOT_PSK_HEX (64 hex chars).
 *
 * This is a PRE-SHARED key, deliberately: no key-agreement protocol is
 * implemented, and none is invented here. Real deployments need an
 * authenticated exchange (X25519 + HKDF or the Tor onion-service handshake).
 * Returns PF_ERR_NOKEY if the variable is absent or malformed — privacy
 * mode then refuses to send rather than falling back to plaintext.
 */
int  privframe_set_key_from_env(void);
int  privframe_set_key(const uint8_t key[32]);
int  privframe_have_key(void);

/* Smallest bucket that fits `payload_len` bytes, or 0 if too large. */
uint16_t privframe_bucket_for(uint16_t payload_len);

/* Buffer-level codec. send/recv are thin wrappers over these, and the leak
   harness asserts directly on the encoded bytes. */
int privframe_encode(uint8_t type, uint8_t hops_left,
                     const uint8_t *payload, uint16_t length,
                     uint8_t *out, size_t cap, size_t *out_len);
int privframe_decode(const uint8_t *wire, size_t wire_len, PrivFrame *out);

int privframe_send(const AirbConn *conn, uint8_t type, uint8_t hops_left,
                   const uint8_t *payload, uint16_t length);

int privframe_recv(const AirbConn *conn, PrivFrame *out);

const char *privframe_strerror(int code);

#endif /* AIRBOT_PRIVFRAME_H */
