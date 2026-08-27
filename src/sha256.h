/*
 * Airbot — Executable Information System
 * sha256.h — SHA-256 (FIPS 180-4), HMAC-SHA256 (RFC 2104),
 *            HKDF-SHA256 (RFC 5869)
 *
 * Exists so the key derivation can be a STANDARD, verifiable construction.
 * The previous KDF used HKDF's shape over HMAC-BLAKE3, which no published
 * vector covers and no second implementation can check.
 *
 * These functions establish FUNCTIONAL CONFORMANCE to the published
 * standards when the vectors pass. They do not establish constant-time
 * behaviour or security; those are separate questions (see C4.3).
 */
#ifndef AIRBOT_SHA256_H
#define AIRBOT_SHA256_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint32_t h[8];
    uint8_t  buf[64];
    size_t   buf_len;
    uint64_t total;
} Sha256State;

void sha256_init(Sha256State *s);
void sha256_update(Sha256State *s, const uint8_t *data, size_t len);
void sha256_final(Sha256State *s, uint8_t out[32]);
void sha256(const uint8_t *data, size_t len, uint8_t out[32]);

void hmac_sha256(const uint8_t *key, size_t key_len,
                 const uint8_t *msg, size_t msg_len, uint8_t out[32]);

/* RFC 5869 §2.2. A NULL/empty salt becomes HashLen zero bytes, per the RFC. */
void hkdf_sha256_extract(const uint8_t *salt, size_t salt_len,
                         const uint8_t *ikm, size_t ikm_len, uint8_t prk[32]);

/* RFC 5869 §2.3. Returns -1 if okm_len > 255*32 or info_len > 512. */
int  hkdf_sha256_expand(const uint8_t prk[32],
                        const uint8_t *info, size_t info_len,
                        uint8_t *okm, size_t okm_len);

int  hkdf_sha256(const uint8_t *salt, size_t salt_len,
                 const uint8_t *ikm, size_t ikm_len,
                 const uint8_t *info, size_t info_len,
                 uint8_t *okm, size_t okm_len);

#endif /* AIRBOT_SHA256_H */
