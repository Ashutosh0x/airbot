/*
 * Airbot — Executable Information System
 * blake3.c — Simplified BLAKE3-256 Hash
 */

#include "blake3.h"

static const uint32_t BLAKE3_IV[8] = {
    0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
    0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
};

static inline uint32_t rotr32(uint32_t x, int c) {
    return (x >> c) | (x << (32 - c));
}

/**
 * @brief ChaCha-style quarter round for mixing state.
 */
static void quarter_round(uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d, uint32_t x, uint32_t y) {
    *a = *a + *b + x;
    *d = rotr32(*d ^ *a, 16);
    *c = *c + *d;
    *b = rotr32(*b ^ *c, 12);
    *a = *a + *b + y;
    *d = rotr32(*d ^ *a, 8);
    *c = *c + *d;
    *b = rotr32(*b ^ *c, 7);
}

/**
 * @brief Compresses a single 64-byte block into the hash state.
 */
static void blake3_compress(Blake3State *state) {
    uint32_t v[16];
    for (int i = 0; i < 8; i++) {
        v[i] = state->state[i];
        v[i + 8] = BLAKE3_IV[i];
    }
    
    uint32_t m[16];
    for (int i = 0; i < 16; i++) {
        m[i] = ((uint32_t)state->buffer[i * 4]) |
               (((uint32_t)state->buffer[i * 4 + 1]) << 8) |
               (((uint32_t)state->buffer[i * 4 + 2]) << 16) |
               (((uint32_t)state->buffer[i * 4 + 3]) << 24);
    }
    
    for (int i = 0; i < 7; i++) {
        quarter_round(&v[0], &v[4], &v[8],  &v[12], m[0],  m[1]);
        quarter_round(&v[1], &v[5], &v[9],  &v[13], m[2],  m[3]);
        quarter_round(&v[2], &v[6], &v[10], &v[14], m[4],  m[5]);
        quarter_round(&v[3], &v[7], &v[11], &v[15], m[6],  m[7]);
        
        quarter_round(&v[0], &v[5], &v[10], &v[15], m[8],  m[9]);
        quarter_round(&v[1], &v[6], &v[11], &v[12], m[10], m[11]);
        quarter_round(&v[2], &v[7], &v[8],  &v[13], m[12], m[13]);
        quarter_round(&v[3], &v[4], &v[9],  &v[14], m[14], m[15]);
    }
    
    for (int i = 0; i < 8; i++) {
        state->state[i] ^= v[i] ^ v[i + 8];
    }
}

void blake3_init(Blake3State *state) {
    for (int i = 0; i < 8; i++) {
        state->state[i] = BLAKE3_IV[i];
    }
    state->buffer_len = 0;
}

void blake3_update(Blake3State *state, const uint8_t *data, size_t len) {
    size_t offset = 0;
    while (offset < len) {
        size_t take = 64 - state->buffer_len;
        if (take > len - offset) {
            take = len - offset;
        }
        
        for (size_t i = 0; i < take; i++) {
            state->buffer[state->buffer_len + i] = data[offset + i];
        }
        
        state->buffer_len += take;
        offset += take;
        
        if (state->buffer_len == 64) {
            blake3_compress(state);
            state->buffer_len = 0;
        }
    }
}

void blake3_finalize(Blake3State *state, uint8_t out[32]) {
    state->buffer[state->buffer_len] = 0x80; /* Padding bit */
    for (size_t i = state->buffer_len + 1; i < 64; i++) {
        state->buffer[i] = 0;
    }
    
    blake3_compress(state);
    
    for (int i = 0; i < 8; i++) {
        out[i * 4] = state->state[i] & 0xFF;
        out[i * 4 + 1] = (state->state[i] >> 8) & 0xFF;
        out[i * 4 + 2] = (state->state[i] >> 16) & 0xFF;
        out[i * 4 + 3] = (state->state[i] >> 24) & 0xFF;
    }
}

void blake3_hash(const uint8_t *input, size_t len, uint8_t out[32]) {
    Blake3State state;
    blake3_init(&state);
    blake3_update(&state, input, len);
    blake3_finalize(&state, out);
}
