#include "capability.h"
#include <string.h>

/* Initialize a capability with specified rights and zero token/generation. */
void cap_init(Capability *cap, uint64_t rights) {
    if (!cap) return;
    cap->rights = rights;
    memset(cap->auth_token, 0, sizeof(cap->auth_token));
    cap->expiry = 0;
    cap->generation = 0;
}

/* Check if the capability possesses the required rights. */
int cap_check(const Capability *cap, uint64_t required) {
    if (!cap) return 0;
    return (cap->rights & required) == required;
}

/* Attenuate a parent capability into a child capability by removing specific rights. */
int cap_attenuate(const Capability *parent, Capability *child, uint64_t remove_rights) {
    if (!parent || !child) return -1;
    if (parent->generation >= 255) return -2; /* Cannot attenuate past max generation */

    child->rights = parent->rights & ~remove_rights;
    memcpy(child->auth_token, parent->auth_token, sizeof(parent->auth_token));
    child->expiry = parent->expiry;
    child->generation = parent->generation + 1;

    return 0;
}

/* Validates that a capability is active and hasn't hit generation limits. */
int cap_is_valid(const Capability *cap) {
    if (!cap) return 0;
    /* Expiration logic (simplified: assuming expiry 0 = never expires) */
    /* Generation must be < 255 to allow further use/attenuation context logic if desired */
    if (cap->generation == 255) return 0;
    return 1;
}

/* Compute simple token by XORing rights bytes with the key and doing a left rotation. */
void cap_compute_token(Capability *cap, const uint8_t root_key[16]) {
    if (!cap || !root_key) return;

    for (int i = 0; i < 16; i++) {
        /* Extract right byte to XOR, cycling through 8 bytes of uint64_t */
        uint8_t right_byte = (cap->rights >> ((i % 8) * 8)) & 0xFF;
        
        cap->auth_token[i] = right_byte ^ root_key[i];
        /* Rotate left by 1 */
        cap->auth_token[i] = (cap->auth_token[i] << 1) | (cap->auth_token[i] >> 7);
    }
}

/* Verify if the existing token matches a freshly computed one against the root_key. */
int cap_verify_token(const Capability *cap, const uint8_t root_key[16]) {
    if (!cap || !root_key) return 0;
    
    uint8_t expected_token[16];
    for (int i = 0; i < 16; i++) {
        uint8_t right_byte = (cap->rights >> ((i % 8) * 8)) & 0xFF;
        expected_token[i] = right_byte ^ root_key[i];
        expected_token[i] = (expected_token[i] << 1) | (expected_token[i] >> 7);
    }
    
    return memcmp(cap->auth_token, expected_token, 16) == 0;
}
