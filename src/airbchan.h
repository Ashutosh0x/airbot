/*
 * Airbot — Executable Information System
 * airbchan.h — The sanctioned application data path
 *
 * WHY THIS MODULE EXISTS
 *
 * onionx and privframe were previously implemented, tested and NOT CALLED.
 * A socket capture of the production `net-send` showed the plaintext AIRB
 * frame still on the wire: magic at offset 0, cleartext payload, cleartext
 * hop counter, and a stable BLAKE3 content digest. The security properties
 * lived in modules the data path never touched.
 *
 * The lesson is that passing unit tests do not protect a socket. So this is
 * not simply "another helper": it is the ONLY route by which application
 * data may reach a socket in privacy mode, and the raw plaintext framer is
 * refused by policy rather than by convention.
 *
 * LAYERING (production path)
 *
 *   application payload
 *     |
 *     |  true_len(2) || payload || random padding      <- inside innermost layer
 *     v
 *   ox_wrap: per-hop X25519 + HKDF + ChaCha20-Poly1305, one layer per hop
 *     |
 *     v
 *   link frame: bucket_index(1) || onion bytes         <- exactly one bucket
 *     |
 *     v
 *   transport_send_raw -> Tor -> relay
 *
 * Every link is independently size-bucketed, so a relay forwarding a peeled
 * onion re-pads to the next bucket and the observed size on link N reveals
 * nothing about the size on link N+1 beyond its bucket.
 *
 * WHAT IS VISIBLE ON A LINK
 *   - one byte: the bucket index (0..5)
 *   - that many bytes of ciphertext
 * Nothing else. No magic, no version, no type, no hop counter, no digest.
 */
#ifndef AIRBOT_AIRBCHAN_H
#define AIRBOT_AIRBCHAN_H

#include <stdint.h>
#include <stddef.h>
#include "transport.h"
#include "onionx.h"

/*
 * FIXED-SIZE LINK ENVELOPE (hop-index concealment)
 *
 * Previously every link carried "2-byte length || onion", so the frame shrank
 * by exactly one hop's overhead per link (514 / 416 / 318) and an observer
 * could read the remaining layer count straight off the wire.
 *
 * Each link is now wrapped in its own constant-size AEAD envelope:
 *
 *   link_eph_pk(32) || nonce(12) || AEAD_k_link( len(2) || onion || pad ) || tag(16)
 *   ------------------------------ AC_ENVELOPE_WIRE, constant --------------
 *
 * k_link = HKDF(X25519(link_eph_sk, relay_pk), "airbot-link-v1", ...)
 *
 * This is a SEPARATE key from the onion hop key (different domain-separation
 * label), so the transport envelope and the onion layer are cryptographically
 * independent: opening the envelope does not help peel the onion.
 *
 * The onion length now lives INSIDE the envelope ciphertext, so the true
 * onion size - and therefore the hop index - is not visible on the wire.
 * A relay still learns the remaining onion length after it opens its own
 * envelope; that is inherent, and is stated in SECURITY.md rather than
 * claimed away.
 *
 * No new primitive: X25519 (RFC 7748), HKDF (RFC 5869), ChaCha20-Poly1305
 * (RFC 8439), all already verified against published vectors.
 */
#define AC_ENVELOPE_PLAIN  1024                      /* constant plaintext region */
#define AC_ENVELOPE_WIRE   (32 + 12 + AC_ENVELOPE_PLAIN + 16)

#define AC_OK              0
#define AC_ERR_POLICY    -100  /* refused: would bypass the onion path */
#define AC_ERR_TOOBIG    -101
#define AC_ERR_SEND      -102
#define AC_ERR_RECV      -103
#define AC_ERR_BUCKET    -104  /* bucket index not one of the six */
#define AC_ERR_ONION     -105  /* ox_wrap / ox_peel rejected the frame */
#define AC_ERR_TRUNC     -106
#define AC_ERR_ENVELOPE  -107  /* link envelope failed to authenticate */

/* Largest application payload that fits the biggest bucket for `hops` hops. */
uint16_t airbchan_max_payload(int hops);

/*
 * Send an application payload down a path of `hops` relays.
 * Builds the onion, pads to a bucket, and writes it to the socket.
 * This is the only sanctioned application send in privacy mode.
 */
int airbchan_send(const AirbConn *conn, const OxIdentity *path, int hops,
                  const uint8_t *payload, uint16_t len);

/*
 * Receive one link frame and remove exactly this relay's layer.
 * `out->inner` is what must be forwarded to the next hop; when
 * `out->is_exit` the true application payload is returned via
 * airbchan_exit_payload().
 */
int airbchan_recv(const AirbConn *conn, const OxIdentity *me,
                  OxReplayWindow *window, OxPeeled *out);

/*
 * Recover the true application payload from an exit-hop peel result,
 * stripping the length prefix and padding.
 */
int airbchan_exit_payload(const OxPeeled *peeled,
                          uint8_t *out, size_t cap, uint16_t *out_len);

/* Re-seal a peeled inner onion for the next relay in a fresh constant-size
   envelope, so the frame size does not shrink along the chain. */
int airbchan_forward_to(const AirbConn *conn, const OxIdentity *next,
                        const uint8_t *inner, uint16_t inner_len);

/* Buffer-level envelope access, for tests that inspect the wire image. */
int airbchan_seal_buf(const OxIdentity *to, const uint8_t *onion,
                      uint16_t onion_len, uint8_t *wire);
int airbchan_open_buf(const OxIdentity *me, const uint8_t *wire,
                      uint8_t *onion, uint16_t *onion_len);

const char *airbchan_strerror(int code);

/* Self-test of the codec (no sockets). The socket-level proof is the
   live integration test in livetest.c. */
int airbchan_selftest(void);

#endif /* AIRBOT_AIRBCHAN_H */
