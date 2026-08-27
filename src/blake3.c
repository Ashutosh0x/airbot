/*
 * Airbot — Executable Information System
 * blake3.c — BLAKE3-256 (portable reference implementation)
 *
 * Source: implemented from the BLAKE3 specification and the public-domain
 * reference implementation published with the BLAKE3 paper
 * (https://github.com/BLAKE3-team/BLAKE3, reference_impl/reference_impl.rs).
 * Algorithm version: BLAKE3 1.x, unkeyed hashing, 32-byte output.
 *
 * This REPLACES a prior "simplified BLAKE3-inspired" routine that reused the
 * BLAKE3 IV with an ad-hoc compression function. That construction failed the
 * official BLAKE3 test vectors and was homebrew cryptography. It has been
 * removed; there is no fallback to it. Correctness is asserted at runtime by
 * blake3_selftest(), which the build gate runs against official vectors.
 *
 * Scope note: this is the portable, single-threaded, non-SIMD variant. It is
 * correct but not fast. It is used for EIU content addressing only, never as a
 * network identity or a cross-hop correlation token — see SECURITY.md.
 */

#include "blake3.h"
#include <string.h>

#define B3_BLOCK_LEN  64
#define B3_CHUNK_LEN  1024

#define B3_CHUNK_START (1u << 0)
#define B3_CHUNK_END   (1u << 1)
#define B3_PARENT      (1u << 2)
#define B3_ROOT        (1u << 3)

static const uint32_t B3_IV[8] = {
    0x6A09E667u, 0xBB67AE85u, 0x3C6EF372u, 0xA54FF53Au,
    0x510E527Fu, 0x9B05688Cu, 0x1F83D9ABu, 0x5BE0CD19u
};

static const uint8_t B3_MSG_PERMUTATION[16] = {
    2, 6, 3, 10, 7, 0, 4, 13, 1, 11, 12, 5, 9, 14, 15, 8
};

static uint32_t rotr32(uint32_t x, int n) {
    return (x >> n) | (x << (32 - n));
}

static uint32_t load32_le(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static void store32_le(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)(v);       p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16); p[3] = (uint8_t)(v >> 24);
}

static void g(uint32_t *st, int a, int b, int c, int d, uint32_t mx, uint32_t my) {
    st[a] = st[a] + st[b] + mx;
    st[d] = rotr32(st[d] ^ st[a], 16);
    st[c] = st[c] + st[d];
    st[b] = rotr32(st[b] ^ st[c], 12);
    st[a] = st[a] + st[b] + my;
    st[d] = rotr32(st[d] ^ st[a], 8);
    st[c] = st[c] + st[d];
    st[b] = rotr32(st[b] ^ st[c], 7);
}

static void round_fn(uint32_t st[16], const uint32_t m[16]) {
    /* columns */
    g(st, 0, 4,  8, 12, m[0],  m[1]);
    g(st, 1, 5,  9, 13, m[2],  m[3]);
    g(st, 2, 6, 10, 14, m[4],  m[5]);
    g(st, 3, 7, 11, 15, m[6],  m[7]);
    /* diagonals */
    g(st, 0, 5, 10, 15, m[8],  m[9]);
    g(st, 1, 6, 11, 12, m[10], m[11]);
    g(st, 2, 7,  8, 13, m[12], m[13]);
    g(st, 3, 4,  9, 14, m[14], m[15]);
}

static void permute(uint32_t m[16]) {
    uint32_t tmp[16];
    int i;
    for (i = 0; i < 16; i++) tmp[i] = m[B3_MSG_PERMUTATION[i]];
    memcpy(m, tmp, sizeof(tmp));
}

/* Full 16-word compression output. */
static void compress(const uint32_t cv[8], const uint32_t block[16],
                     uint64_t counter, uint32_t block_len, uint32_t flags,
                     uint32_t out[16]) {
    uint32_t st[16], m[16];
    int i;

    st[0] = cv[0]; st[1] = cv[1]; st[2] = cv[2]; st[3] = cv[3];
    st[4] = cv[4]; st[5] = cv[5]; st[6] = cv[6]; st[7] = cv[7];
    st[8]  = B3_IV[0]; st[9]  = B3_IV[1];
    st[10] = B3_IV[2]; st[11] = B3_IV[3];
    st[12] = (uint32_t)(counter & 0xFFFFFFFFu);
    st[13] = (uint32_t)(counter >> 32);
    st[14] = block_len;
    st[15] = flags;

    memcpy(m, block, sizeof(m));

    for (i = 0; i < 7; i++) {
        round_fn(st, m);
        if (i < 6) permute(m);
    }

    for (i = 0; i < 8; i++) {
        st[i]     ^= st[i + 8];
        st[i + 8] ^= cv[i];
    }
    memcpy(out, st, sizeof(st));
}

static void words_from_block(const uint8_t blk[64], uint32_t out[16]) {
    int i;
    for (i = 0; i < 16; i++) out[i] = load32_le(blk + i * 4);
}

/* --- chunk state ------------------------------------------ */

static void chunk_reset(Blake3ChunkState *c, const uint32_t key[8],
                        uint64_t chunk_counter) {
    memcpy(c->cv, key, 8 * sizeof(uint32_t));
    c->chunk_counter = chunk_counter;
    memset(c->block, 0, B3_BLOCK_LEN);
    c->block_len = 0;
    c->blocks_compressed = 0;
}

static size_t chunk_len(const Blake3ChunkState *c) {
    return (size_t)B3_BLOCK_LEN * c->blocks_compressed + c->block_len;
}

static uint32_t chunk_start_flag(const Blake3ChunkState *c) {
    return c->blocks_compressed == 0 ? B3_CHUNK_START : 0;
}

static void chunk_update(Blake3ChunkState *c, const uint8_t *in, size_t len) {
    while (len > 0) {
        if (c->block_len == B3_BLOCK_LEN) {
            uint32_t bw[16], out[16];
            words_from_block(c->block, bw);
            compress(c->cv, bw, c->chunk_counter, B3_BLOCK_LEN,
                     chunk_start_flag(c), out);
            memcpy(c->cv, out, 8 * sizeof(uint32_t));
            c->blocks_compressed++;
            memset(c->block, 0, B3_BLOCK_LEN);
            c->block_len = 0;
        }
        {
            size_t want = (size_t)B3_BLOCK_LEN - c->block_len;
            size_t take = len < want ? len : want;
            memcpy(c->block + c->block_len, in, take);
            c->block_len += (uint8_t)take;
            in += take;
            len -= take;
        }
    }
}

/* An Output is a deferred compression: it can yield either a chaining value
   (interior node) or extended root bytes (final node). */
typedef struct {
    uint32_t input_cv[8];
    uint32_t block_words[16];
    uint64_t counter;
    uint32_t block_len;
    uint32_t flags;
} B3Output;

static void chunk_output(const Blake3ChunkState *c, B3Output *o) {
    memcpy(o->input_cv, c->cv, 8 * sizeof(uint32_t));
    words_from_block(c->block, o->block_words);
    o->counter   = c->chunk_counter;
    o->block_len = c->block_len;
    o->flags     = chunk_start_flag(c) | B3_CHUNK_END;
}

static void parent_output(const uint32_t left[8], const uint32_t right[8],
                          const uint32_t key[8], B3Output *o) {
    memcpy(o->input_cv, key, 8 * sizeof(uint32_t));
    memcpy(o->block_words,     left,  8 * sizeof(uint32_t));
    memcpy(o->block_words + 8, right, 8 * sizeof(uint32_t));
    o->counter   = 0;
    o->block_len = B3_BLOCK_LEN;
    o->flags     = B3_PARENT;
}

static void output_chaining_value(const B3Output *o, uint32_t cv[8]) {
    uint32_t out[16];
    compress(o->input_cv, o->block_words, o->counter, o->block_len, o->flags, out);
    memcpy(cv, out, 8 * sizeof(uint32_t));
}

static void output_root_bytes(const B3Output *o, uint8_t *out, size_t out_len) {
    uint64_t block_counter = 0;
    while (out_len > 0) {
        uint32_t words[16];
        size_t take, i;
        uint8_t wide[64];
        compress(o->input_cv, o->block_words, block_counter, o->block_len,
                 o->flags | B3_ROOT, words);
        for (i = 0; i < 16; i++) store32_le(wide + i * 4, words[i]);
        take = out_len < 64 ? out_len : 64;
        memcpy(out, wide, take);
        out += take;
        out_len -= take;
        block_counter++;
    }
}

/* --- public API ------------------------------------------- */

void blake3_init(Blake3State *st) {
    memcpy(st->key, B3_IV, 8 * sizeof(uint32_t));
    chunk_reset(&st->chunk, st->key, 0);
    st->cv_stack_len = 0;
}

/* Merge completed subtrees. The number of trailing zero bits in
   total_chunks says how many parent merges are due. */
static void push_chunk_cv(Blake3State *st, uint32_t new_cv[8],
                          uint64_t total_chunks) {
    while ((total_chunks & 1) == 0) {
        B3Output po;
        st->cv_stack_len--;
        parent_output(st->cv_stack[st->cv_stack_len], new_cv, st->key, &po);
        output_chaining_value(&po, new_cv);
        total_chunks >>= 1;
    }
    memcpy(st->cv_stack[st->cv_stack_len], new_cv, 8 * sizeof(uint32_t));
    st->cv_stack_len++;
}

void blake3_update(Blake3State *st, const uint8_t *data, size_t len) {
    while (len > 0) {
        if (chunk_len(&st->chunk) == B3_CHUNK_LEN) {
            B3Output o;
            uint32_t cv[8];
            uint64_t total_chunks;
            chunk_output(&st->chunk, &o);
            output_chaining_value(&o, cv);
            total_chunks = st->chunk.chunk_counter + 1;
            push_chunk_cv(st, cv, total_chunks);
            chunk_reset(&st->chunk, st->key, total_chunks);
        }
        {
            size_t want = (size_t)B3_CHUNK_LEN - chunk_len(&st->chunk);
            size_t take = len < want ? len : want;
            chunk_update(&st->chunk, data, take);
            data += take;
            len  -= take;
        }
    }
}

void blake3_finalize(Blake3State *st, uint8_t out[32]) {
    B3Output o;
    int i;
    chunk_output(&st->chunk, &o);
    /* Fold the stack right-to-left into the root node. */
    for (i = (int)st->cv_stack_len - 1; i >= 0; i--) {
        uint32_t right_cv[8];
        B3Output po;
        output_chaining_value(&o, right_cv);
        parent_output(st->cv_stack[i], right_cv, st->key, &po);
        o = po;
    }
    output_root_bytes(&o, out, 32);
}

void blake3_hash(const uint8_t *input, size_t len, uint8_t out[32]) {
    Blake3State st;
    blake3_init(&st);
    blake3_update(&st, input, len);
    blake3_finalize(&st, out);
}

/* --- self test -------------------------------------------- */

/* Official BLAKE3 test-vector input: the repeating byte pattern
   0,1,2,...,250,0,1,2,... as specified in test_vectors.json. */
static void b3_fill_pattern(uint8_t *buf, size_t len) {
    size_t i;
    for (i = 0; i < len; i++) buf[i] = (uint8_t)(i % 251);
}

static int hex_eq(const uint8_t d[32], const char *hex) {
    static const char *H = "0123456789abcdef";
    int i;
    for (i = 0; i < 32; i++) {
        char hi = H[(d[i] >> 4) & 0xF], lo = H[d[i] & 0xF];
        if (hex[i * 2] != hi || hex[i * 2 + 1] != lo) return 0;
    }
    return 1;
}

int blake3_selftest(void) {
    /* Vectors from the official BLAKE3 test_vectors.json. The lengths are
       chosen to exercise: empty input, sub-block, exact block, chunk
       boundary, exact chunk, multi-chunk Merkle tree, and an unbalanced
       tree that forces stack folding. */
    static const struct { size_t len; const char *hex; } V[] = {
        {     0, "af1349b9f5f9a1a6a0404dea36dcc9499bcb25c9adc112b7cc9a93cae41f3262" },
        {     1, "2d3adedff11b61f14c886e35afa036736dcd87a74d27b5c1510225d0f592e213" },
        {     2, "7b7015bb92cf0b318037702a6cdd81dee41224f734684c2c122cd6359cb1ee63" },
        {     3, "e1be4d7a8ab5560aa4199eea339849ba8e293d55ca0a81006726d184519e647f" },
        {    64, "4eed7141ea4a5cd4b788606bd23f46e212af9cacebacdc7d1f4c6dc7f2511b98" },
        {  1023, "10108970eeda3eb932baac1428c7a2163b0e924c9a9e25b35bba72b28f70bd11" },
        {  1024, "42214739f095a406f3fc83deb889744ac00df831c10daa55189b5d121c855af7" },
        {  1025, "d00278ae47eb27b34faecf67b4fe263f82d5412916c1ffd97c8cb7fb814b8444" },
        {  2048, "e776b6028c7cd22a4d0ba182a8bf62205d2ef576467e838ed6f2529b85fba24a" },
        {  3072, "b98cb0ff3623be03326b373de6b9095218513e64f1ee2edd2525c7ad1e5cffd2" },
        {  4096, "015094013f57a5277b59d8475c0501042c0b642e531b0a1c8f58d2163229e969" }
    };
    static uint8_t buf[4096];
    uint8_t out[32];
    size_t i;
    int fails = 0;

    for (i = 0; i < sizeof(V) / sizeof(V[0]); i++) {
        b3_fill_pattern(buf, V[i].len);
        blake3_hash(buf, V[i].len, out);
        if (!hex_eq(out, V[i].hex)) fails++;
    }
    return fails;
}
