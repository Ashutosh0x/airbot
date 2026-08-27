/*
 * Airbot — Executable Information System
 * blake3.h — BLAKE3-256 (portable reference implementation)
 *
 * Genuine BLAKE3, implemented from the BLAKE3 specification and the
 * public-domain reference implementation. Verified against the official
 * test_vectors.json by blake3_selftest().
 *
 * Replaces an earlier "simplified BLAKE3-inspired" routine that failed the
 * official vectors. That construction has been removed with no fallback.
 *
 * USAGE POLICY: BLAKE3 is for EIU content addressing and local integrity
 * only. It must NOT be used as a network identity, a routing tag, or any
 * externally visible identifier - a stable content digest on the wire is a
 * cross-hop correlation token. See SECURITY.md.
 */
#ifndef AIRBOT_BLAKE3_H
#define AIRBOT_BLAKE3_H

#include <stdint.h>
#include <stddef.h>

/**
 * @brief Represents the internal state of the BLAKE3 hash.
 */
/* Chunk-local state: one 1 KiB BLAKE3 chunk in progress. */
typedef struct {
    uint32_t cv[8];
    uint64_t chunk_counter;
    uint8_t  block[64];
    uint8_t  block_len;
    uint8_t  blocks_compressed;
} Blake3ChunkState;

/*
 * Full BLAKE3 hasher state.
 *
 * The stack holds chaining values of completed subtrees; 54 entries covers
 * the maximum tree depth for any input a 64-bit length can express.
 * Treat this struct as opaque - fields are implementation detail.
 */
typedef struct {
    uint32_t         key[8];
    Blake3ChunkState chunk;
    uint32_t         cv_stack[54][8];
    uint8_t          cv_stack_len;
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

/**
 * @brief Verify this implementation against official BLAKE3 test vectors.
 *
 * Covers empty input, sub-block, exact block, chunk boundary, exact chunk,
 * multi-chunk Merkle tree and an unbalanced tree requiring stack folding.
 *
 * @return Number of failing vectors. 0 means the implementation is correct.
 *         A non-zero result must fail the build.
 */
int blake3_selftest(void);

#endif /* AIRBOT_BLAKE3_H */
