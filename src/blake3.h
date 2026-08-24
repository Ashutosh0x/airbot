/*
 * Airbot — Executable Information System
 * blake3.h — Simplified BLAKE3-256 Hash
 *
 * Implements a simplified BLAKE3-inspired hash producing 32-byte digests.
 */
#ifndef AIRBOT_BLAKE3_H
#define AIRBOT_BLAKE3_H

#include <stdint.h>
#include <stddef.h>

/**
 * @brief Represents the internal state of the BLAKE3 hash.
 */
typedef struct {
    uint32_t state[8];
    uint8_t buffer[64];
    size_t buffer_len;
} Blake3State;

/**
 * @brief Initializes a BLAKE3 state context.
 * 
 * @param state Pointer to the Blake3State to initialize.
 */
void blake3_init(Blake3State *state);

/**
 * @brief Updates the hash state with new data.
 * 
 * @param state Pointer to the initialized Blake3State.
 * @param data Data buffer to process.
 * @param len Length of data in bytes.
 */
void blake3_update(Blake3State *state, const uint8_t *data, size_t len);

/**
 * @brief Finalizes the hash and writes the 32-byte digest.
 * 
 * @param state Pointer to the initialized Blake3State.
 * @param out 32-byte buffer to store the hash output.
 */
void blake3_finalize(Blake3State *state, uint8_t out[32]);

/**
 * @brief Convenience function to hash a buffer in one shot.
 * 
 * @param input Data buffer to hash.
 * @param len Length of input in bytes.
 * @param out 32-byte buffer to store the hash output.
 */
void blake3_hash(const uint8_t *input, size_t len, uint8_t out[32]);

#endif /* AIRBOT_BLAKE3_H */
