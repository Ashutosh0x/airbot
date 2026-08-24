#ifndef AIRBOT_CAPABILITY_H
#define AIRBOT_CAPABILITY_H

#include <stdint.h>

/* Capability rights bitmask (64-bit) */
#define CAP_READ       (1ULL << 0)
#define CAP_WRITE      (1ULL << 1)
#define CAP_EXECUTE    (1ULL << 2)
#define CAP_REPLICATE  (1ULL << 3)
#define CAP_MUTATE     (1ULL << 4)
#define CAP_NETWORK    (1ULL << 5)
#define CAP_SPAWN      (1ULL << 6)
#define CAP_ADMIN      (1ULL << 7)
#define CAP_SELF_READ  (1ULL << 8)
#define CAP_ALL        0xFFFFFFFFFFFFFFFFULL

typedef struct {
    uint64_t rights;        /* Bitmask of granted capabilities */
    uint8_t  auth_token[16]; /* Authorization token (HMAC-based) */
    uint64_t expiry;        /* Expiration timestamp (0 = never) */
    uint8_t  generation;    /* Attenuation generation counter */
} Capability;

/* Create with rights, zero token */
void cap_init(Capability *cap, uint64_t rights);

/* Returns 1 if all required rights granted, 0 otherwise */
int cap_check(const Capability *cap, uint64_t required);

/* Child = parent minus removed rights. Can only restrict, never expand. Returns 0 on success. */
int cap_attenuate(const Capability *parent, Capability *child, uint64_t remove_rights);

/* Check if not expired, generation < 255. Returns 1 if valid, 0 otherwise. */
int cap_is_valid(const Capability *cap);

/* Compute simple HMAC-like token: XOR rights bytes with root_key, then rotate */
void cap_compute_token(Capability *cap, const uint8_t root_key[16]);

/* Verify token matches. Returns 1 if matching, 0 otherwise. */
int cap_verify_token(const Capability *cap, const uint8_t root_key[16]);

#endif /* AIRBOT_CAPABILITY_H */
