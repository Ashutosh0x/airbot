/*
 * Airbot — Executable Information System
 * chacha20.h — ChaCha20-Poly1305 AEAD (RFC 8439)
 *
 * Standard authenticated encryption for the onion layer.
 * ChaCha20 is an ARX cipher (add-rotate-XOR) — no lookup tables,
 * constant-time, simple to implement in pure C99.
 *
 * Poly1305 provides authentication/integrity (16-byte tag).
 * Combined as ChaCha20-Poly1305 AEAD per RFC 8439 §2.8.
 */

#ifndef AIRBOT_CHACHA20_H
#define AIRBOT_CHACHA20_H

#include <stdint.h>
#include <stddef.h>

/* ═══════════════════════════════════════════════════════════════
 * Constants
 * ═══════════════════════════════════════════════════════════════ */

#define CHACHA20_KEY_SIZE       32
#define CHACHA20_NONCE_SIZE     12
#define CHACHA20_BLOCK_SIZE     64
#define POLY1305_KEY_SIZE       32
#define POLY1305_TAG_SIZE       16

/* ═══════════════════════════════════════════════════════════════
 * ChaCha20 Core (RFC 8439 §2.1–2.3)
 * ═══════════════════════════════════════════════════════════════ */

/* ChaCha20 state: 16 x 32-bit words */
typedef struct {
    uint32_t state[16];
} ChaCha20State;

/*
 * Initialize ChaCha20 with a 256-bit key and 96-bit nonce.
 * key:   32 bytes
 * nonce: 12 bytes
 * counter: initial block counter (usually 0 or 1)
 */
void chacha20_init(ChaCha20State *ctx, const uint8_t key[32],
                   const uint8_t nonce[12], uint32_t counter);

/*
 * Encrypt/decrypt data in-place using ChaCha20 keystream.
 * XOR is its own inverse, so encrypt == decrypt.
 */
void chacha20_crypt(ChaCha20State *ctx, uint8_t *data, size_t len);

/*
 * Generate a keystream block (64 bytes) without encrypting data.
 * Useful for generating MAC keys (RFC 8439 §2.6).
 */
void chacha20_block(ChaCha20State *ctx, uint8_t out[64]);

/* ═══════════════════════════════════════════════════════════════
 * Poly1305 MAC (RFC 8439 §2.5)
 * ═══════════════════════════════════════════════════════════════ */

/* Poly1305 state for incremental processing */
typedef struct {
    uint32_t r[5];      /* Clamped r key (base-2^26 limbs) */
    uint32_t s[4];      /* s key (second half of one-time key) */
    uint32_t h[5];      /* Accumulator (base-2^26 limbs) */
    uint8_t  buf[16];   /* Partial block buffer */
    size_t   buf_len;   /* Bytes in partial block */
} Poly1305State;

/*
 * One-shot Poly1305 MAC computation.
 * key: 32-byte one-time key (r[16] || s[16])
 * msg: message to authenticate
 * tag: 16-byte output tag
 */
void poly1305_auth(uint8_t tag[16], const uint8_t *msg, size_t msg_len,
                   const uint8_t key[32]);

/* Incremental Poly1305 API */
void poly1305_init(Poly1305State *st, const uint8_t key[32]);
void poly1305_update(Poly1305State *st, const uint8_t *data, size_t len);
void poly1305_finish(Poly1305State *st, uint8_t tag[16]);

/* ═══════════════════════════════════════════════════════════════
 * ChaCha20-Poly1305 AEAD (RFC 8439 §2.8)
 *
 * Provides both confidentiality and integrity/authentication.
 *
 * encrypt:
 *   1. Derive Poly1305 one-time key from ChaCha20(key, nonce, counter=0)
 *   2. Encrypt plaintext with ChaCha20(key, nonce, counter=1)
 *   3. Construct Poly1305 input per RFC 8439:
 *      AAD || pad(AAD) || ciphertext || pad(CT) || len(AAD) || len(CT)
 *   4. Compute tag = Poly1305(construction, one-time key)
 *
 * decrypt:
 *   1. Derive same one-time key
 *   2. Verify tag before decrypting
 *   3. If tag mismatch, return -1 (authentication failure)
 *   4. Decrypt only if authenticated
 *
 * Returns 0 on success, -1 on failure.
 * ═══════════════════════════════════════════════════════════════ */

int chacha20_poly1305_encrypt(const uint8_t key[32], const uint8_t nonce[12],
                              const uint8_t *aad, size_t aad_len,
                              const uint8_t *plaintext, size_t pt_len,
                              uint8_t *ciphertext, uint8_t tag[16]);

int chacha20_poly1305_decrypt(const uint8_t key[32], const uint8_t nonce[12],
                              const uint8_t *aad, size_t aad_len,
                              const uint8_t *ciphertext, size_t ct_len,
                              const uint8_t tag[16], uint8_t *plaintext);

/* ═══════════════════════════════════════════════════════════════
 * Legacy BLAKE3-MAC API (deprecated — kept for compatibility)
 * Use chacha20_poly1305_encrypt/decrypt instead.
 * ═══════════════════════════════════════════════════════════════ */

int chacha20_encrypt_authenticate(const uint8_t key[32], const uint8_t nonce[12],
                                  const uint8_t *plaintext, size_t plain_len,
                                  uint8_t *out, size_t *out_len);

int chacha20_verify_decrypt(const uint8_t key[32], const uint8_t nonce[12],
                            const uint8_t *ciphertext, size_t cipher_len,
                            uint8_t *out, size_t *out_len);

/* ═══════════════════════════════════════════════════════════════
 * Utilities
 * ═══════════════════════════════════════════════════════════════ */

/* Cryptographically secure random bytes (OS-provided) */
void csprng_bytes(uint8_t *out, size_t len);

/* Secure memory wipe (not optimized away by compiler) */
void secure_zero(void *ptr, size_t len);

/* Constant-time comparison (returns 0 if equal, non-zero otherwise) */
int ct_compare(const uint8_t *a, const uint8_t *b, size_t len);

#endif /* AIRBOT_CHACHA20_H */
