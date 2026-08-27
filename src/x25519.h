/*
 * Airbot — Executable Information System
 * x25519.h — X25519 (RFC 7748) key agreement
 *
 * Standard primitive, not a custom construction. Used to establish per-hop
 * ephemeral keys so no universal pre-shared key exists. See SECURITY.md.
 */
#ifndef AIRBOT_X25519_H
#define AIRBOT_X25519_H

#include <stdint.h>

/* out = scalarmult(scalar, point). Scalar is clamped per RFC 7748 §5. */
void x25519_scalarmult(uint8_t out[32], const uint8_t scalar[32], const uint8_t point[32]);

/* out = scalarmult(scalar, basepoint) — derive a public key. */
void x25519_base(uint8_t out[32], const uint8_t scalar[32]);

/* 1 if the value is all zero. A zero shared secret must be rejected
   (RFC 7748 §6.1 contributory-behaviour check). */
int  x25519_is_zero(const uint8_t p[32]);

#endif /* AIRBOT_X25519_H */
