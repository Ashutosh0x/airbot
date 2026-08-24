/*
 * Airbot — Executable Information System
 * chacha20.c — ChaCha20-Poly1305 AEAD Implementation (RFC 8439)
 *
 * Pure C99, no external dependencies.
 * Implements:
 *   - ChaCha20 stream cipher (RFC 8439 §2.1–2.3)
 *   - Poly1305 MAC (RFC 8439 §2.5)
 *   - ChaCha20-Poly1305 AEAD (RFC 8439 §2.8)
 *   - CSPRNG via OS-provided entropy
 *   - Legacy BLAKE3-MAC (deprecated, kept for compatibility)
 */

#include "chacha20.h"
#include "blake3.h"
#include <string.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
/* For BCryptGenRandom or rand_s fallback */
#endif

/* ═══════════════════════════════════════════════════════════════
 * Utility Functions
 * ═══════════════════════════════════════════════════════════════ */

/* Secure memory wipe — use volatile pointer to prevent optimization */
void secure_zero(void *ptr, size_t len) {
    volatile uint8_t *p = (volatile uint8_t *)ptr;
    while (len--) *p++ = 0;
}

/* Constant-time comparison: returns 0 if equal */
int ct_compare(const uint8_t *a, const uint8_t *b, size_t len) {
    uint8_t diff = 0;
    for (size_t i = 0; i < len; i++) {
        diff |= a[i] ^ b[i];
    }
    return (int)diff;
}

/* OS-provided cryptographically secure random bytes */
void csprng_bytes(uint8_t *out, size_t len) {
#ifdef _WIN32
    /* Use RtlGenRandom (SystemFunction036) — available on all Windows versions */
    /* Declared as BOOLEAN NTAPI SystemFunction036(PVOID, ULONG) in advapi32.dll */
    /* Also accessible via rand_s for smaller quantities */
    typedef int (WINAPI *RtlGenRandomFn)(void *, unsigned long);
    static RtlGenRandomFn fn = NULL;
    static int initialized = 0;

    if (!initialized) {
        HMODULE advapi = LoadLibraryA("advapi32.dll");
        if (advapi) {
            fn = (RtlGenRandomFn)(void *)GetProcAddress(advapi, "SystemFunction036");
        }
        initialized = 1;
    }

    if (fn) {
        /* RtlGenRandom can handle up to ULONG_MAX bytes at once */
        size_t remaining = len;
        uint8_t *p = out;
        while (remaining > 0) {
            unsigned long chunk = remaining > 0xFFFFFFFFUL ? 0xFFFFFFFFUL : (unsigned long)remaining;
            if (!fn(p, chunk)) {
                /* Fallback: should never happen on a working Windows system */
                break;
            }
            p += chunk;
            remaining -= chunk;
        }
    } else {
        /* Last-resort fallback using volatile seed + QueryPerformanceCounter */
        /* This is NOT cryptographically secure — indicates broken OS state */
        LARGE_INTEGER pc;
        QueryPerformanceCounter(&pc);
        uint32_t seed = (uint32_t)(pc.QuadPart ^ (uint64_t)(uintptr_t)out);
        for (size_t i = 0; i < len; i++) {
            seed = seed * 1103515245 + 12345;
            out[i] = (uint8_t)(seed >> 16);
        }
    }
#else
    /* POSIX: read from /dev/urandom */
    FILE *f = fopen("/dev/urandom", "rb");
    if (f) {
        size_t got = fread(out, 1, len, f);
        fclose(f);
        if (got == len) return;
    }
    /* Fallback — NOT cryptographically secure */
    for (size_t i = 0; i < len; i++) {
        out[i] = (uint8_t)(i ^ 0x5A);
    }
#endif
}

/* ═══════════════════════════════════════════════════════════════
 * ChaCha20 Core (RFC 8439 §2.1–2.3)
 * ═══════════════════════════════════════════════════════════════ */

/* Rotate left */
static uint32_t rotl32(uint32_t v, int n) {
    return (v << n) | (v >> (32 - n));
}

/* Quarter round (RFC 8439 §2.1) */
static void quarter_round(uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d) {
    *a += *b; *d ^= *a; *d = rotl32(*d, 16);
    *c += *d; *b ^= *c; *b = rotl32(*b, 12);
    *a += *b; *d ^= *a; *d = rotl32(*d, 8);
    *c += *d; *b ^= *c; *b = rotl32(*b, 7);
}

/* Read 32-bit little-endian */
static uint32_t load32_le(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

/* Write 32-bit little-endian */
static void store32_le(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)(v);
    p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16);
    p[3] = (uint8_t)(v >> 24);
}

/* Write 64-bit little-endian */
static void store64_le(uint8_t *p, uint64_t v) {
    store32_le(p, (uint32_t)v);
    store32_le(p + 4, (uint32_t)(v >> 32));
}

/*
 * Initialize ChaCha20 state (RFC 8439 §2.3)
 *
 * State layout (16 x 32-bit words):
 *   [0..3]   = constants "expand 32-byte k"
 *   [4..11]  = 256-bit key
 *   [12]     = block counter
 *   [13..15] = 96-bit nonce
 */
void chacha20_init(ChaCha20State *ctx, const uint8_t key[32],
                   const uint8_t nonce[12], uint32_t counter) {
    /* Constants: "expand 32-byte k" */
    ctx->state[0]  = 0x61707865;
    ctx->state[1]  = 0x3320646e;
    ctx->state[2]  = 0x79622d32;
    ctx->state[3]  = 0x6b206574;

    /* Key (8 words) */
    for (int i = 0; i < 8; i++) {
        ctx->state[4 + i] = load32_le(key + 4 * i);
    }

    /* Counter */
    ctx->state[12] = counter;

    /* Nonce (3 words) */
    for (int i = 0; i < 3; i++) {
        ctx->state[13 + i] = load32_le(nonce + 4 * i);
    }
}

/*
 * Generate one 64-byte keystream block (RFC 8439 §2.3)
 * 20 rounds = 10 iterations of (4 column rounds + 4 diagonal rounds)
 */
void chacha20_block(ChaCha20State *ctx, uint8_t out[64]) {
    uint32_t working[16];
    memcpy(working, ctx->state, sizeof(working));

    /* 20 rounds (10 double-rounds) */
    for (int i = 0; i < 10; i++) {
        /* Column rounds */
        quarter_round(&working[0], &working[4], &working[8],  &working[12]);
        quarter_round(&working[1], &working[5], &working[9],  &working[13]);
        quarter_round(&working[2], &working[6], &working[10], &working[14]);
        quarter_round(&working[3], &working[7], &working[11], &working[15]);
        /* Diagonal rounds */
        quarter_round(&working[0], &working[5], &working[10], &working[15]);
        quarter_round(&working[1], &working[6], &working[11], &working[12]);
        quarter_round(&working[2], &working[7], &working[8],  &working[13]);
        quarter_round(&working[3], &working[4], &working[9],  &working[14]);
    }

    /* Add original state */
    for (int i = 0; i < 16; i++) {
        working[i] += ctx->state[i];
    }

    /* Serialize as little-endian */
    for (int i = 0; i < 16; i++) {
        store32_le(out + 4 * i, working[i]);
    }

    /* Increment counter */
    ctx->state[12]++;
}

/*
 * Encrypt/decrypt data using ChaCha20 keystream
 */
void chacha20_crypt(ChaCha20State *ctx, uint8_t *data, size_t len) {
    uint8_t keystream[64];
    size_t pos = 0;

    while (pos < len) {
        chacha20_block(ctx, keystream);

        size_t block_len = len - pos;
        if (block_len > 64) block_len = 64;

        for (size_t i = 0; i < block_len; i++) {
            data[pos + i] ^= keystream[i];
        }
        pos += block_len;
    }

    /* Clear keystream from stack */
    secure_zero(keystream, sizeof(keystream));
}

/* ═══════════════════════════════════════════════════════════════
 * Poly1305 MAC (RFC 8439 §2.5)
 *
 * Poly1305 computes a 16-byte authenticator of a message using
 * a one-time 32-byte key (r || s).
 *
 * The polynomial evaluation is done modulo 2^130 - 5.
 * We use a radix-2^26 representation with 5 limbs for the
 * accumulator and r key, which keeps all intermediate values
 * within 64-bit arithmetic.
 *
 * Reference: D.J. Bernstein, "The Poly1305-AES message-
 * authentication code", FSE 2005.
 * ═══════════════════════════════════════════════════════════════ */

void poly1305_init(Poly1305State *st, const uint8_t key[32]) {
    /* Decompose r = key[0..15] into radix-2^26 limbs with clamping.
     * Uses the poly1305-donna approach: overlapping 32-bit reads at
     * byte offsets 0,3,6,9,12 with built-in clamping masks.
     * r &= 0x0ffffffc0ffffffc0ffffffc0fffffff */
    st->r[0] = (load32_le(key +  0)     ) & 0x03ffffff;
    st->r[1] = (load32_le(key +  3) >> 2) & 0x03ffff03;
    st->r[2] = (load32_le(key +  6) >> 4) & 0x03ffc0ff;
    st->r[3] = (load32_le(key +  9) >> 6) & 0x03f03fff;
    st->r[4] = (load32_le(key + 12) >> 8) & 0x000fffff;

    /* s = key[16..31] (no clamping) */
    st->s[0] = load32_le(key + 16);
    st->s[1] = load32_le(key + 20);
    st->s[2] = load32_le(key + 24);
    st->s[3] = load32_le(key + 28);

    /* Zero accumulator */
    st->h[0] = st->h[1] = st->h[2] = st->h[3] = st->h[4] = 0;

    /* Zero buffer */
    st->buf_len = 0;
    memset(st->buf, 0, sizeof(st->buf));
}

/* Process one 16-byte block (with hibit flag for padding) */
static void poly1305_block(Poly1305State *st, const uint8_t block[16], int hibit) {
    uint32_t r0 = st->r[0], r1 = st->r[1], r2 = st->r[2], r3 = st->r[3], r4 = st->r[4];
    uint32_t h0 = st->h[0], h1 = st->h[1], h2 = st->h[2], h3 = st->h[3], h4 = st->h[4];

    /* Add message block to accumulator (donna-style overlapping reads) */
    h0 += (load32_le(block +  0)     ) & 0x03ffffff;
    h1 += (load32_le(block +  3) >> 2) & 0x03ffffff;
    h2 += (load32_le(block +  6) >> 4) & 0x03ffffff;
    h3 += (load32_le(block +  9) >> 6) & 0x03ffffff;
    h4 += (load32_le(block + 12) >> 8) | (hibit ? (1 << 24) : 0);

    /* Multiply h by r (schoolbook, with reduction modulo 2^130-5) */
    /* s_i = r_i * 5 for reduction */
    uint32_t s1 = r1 * 5, s2 = r2 * 5, s3 = r3 * 5, s4 = r4 * 5;

    uint64_t d0 = (uint64_t)h0*r0 + (uint64_t)h1*s4 + (uint64_t)h2*s3 + (uint64_t)h3*s2 + (uint64_t)h4*s1;
    uint64_t d1 = (uint64_t)h0*r1 + (uint64_t)h1*r0 + (uint64_t)h2*s4 + (uint64_t)h3*s3 + (uint64_t)h4*s2;
    uint64_t d2 = (uint64_t)h0*r2 + (uint64_t)h1*r1 + (uint64_t)h2*r0 + (uint64_t)h3*s4 + (uint64_t)h4*s3;
    uint64_t d3 = (uint64_t)h0*r3 + (uint64_t)h1*r2 + (uint64_t)h2*r1 + (uint64_t)h3*r0 + (uint64_t)h4*s4;
    uint64_t d4 = (uint64_t)h0*r4 + (uint64_t)h1*r3 + (uint64_t)h2*r2 + (uint64_t)h3*r1 + (uint64_t)h4*r0;

    /* Partial reduction mod 2^130-5 */
    uint32_t c;
    c = (uint32_t)(d0 >> 26); h0 = (uint32_t)d0 & 0x03ffffff; d1 += c;
    c = (uint32_t)(d1 >> 26); h1 = (uint32_t)d1 & 0x03ffffff; d2 += c;
    c = (uint32_t)(d2 >> 26); h2 = (uint32_t)d2 & 0x03ffffff; d3 += c;
    c = (uint32_t)(d3 >> 26); h3 = (uint32_t)d3 & 0x03ffffff; d4 += c;
    c = (uint32_t)(d4 >> 26); h4 = (uint32_t)d4 & 0x03ffffff;
    h0 += c * 5;
    c = h0 >> 26; h0 &= 0x03ffffff; h1 += c;

    st->h[0] = h0; st->h[1] = h1; st->h[2] = h2; st->h[3] = h3; st->h[4] = h4;
}

void poly1305_update(Poly1305State *st, const uint8_t *data, size_t len) {
    size_t i = 0;

    /* Fill partial block buffer first */
    if (st->buf_len > 0) {
        size_t want = 16 - st->buf_len;
        if (want > len) want = len;
        memcpy(st->buf + st->buf_len, data, want);
        st->buf_len += want;
        i += want;

        if (st->buf_len == 16) {
            poly1305_block(st, st->buf, 1);
            st->buf_len = 0;
        }
    }

    /* Process full 16-byte blocks */
    while (i + 16 <= len) {
        poly1305_block(st, data + i, 1);
        i += 16;
    }

    /* Buffer remaining partial block */
    if (i < len) {
        size_t rem = len - i;
        memcpy(st->buf, data + i, rem);
        st->buf_len = rem;
    }
}

void poly1305_finish(Poly1305State *st, uint8_t tag[16]) {
    /* Process final partial block (if any) */
    if (st->buf_len > 0) {
        /* Pad with 0x01 followed by zeros */
        st->buf[st->buf_len] = 0x01;
        for (size_t i = st->buf_len + 1; i < 16; i++) {
            st->buf[i] = 0x00;
        }
        poly1305_block(st, st->buf, 0); /* hibit=0 for partial blocks */
    }

    /* Full reduction mod 2^130-5 */
    uint32_t h0 = st->h[0], h1 = st->h[1], h2 = st->h[2], h3 = st->h[3], h4 = st->h[4];

    uint32_t c;
    c = h1 >> 26; h1 &= 0x03ffffff; h2 += c;
    c = h2 >> 26; h2 &= 0x03ffffff; h3 += c;
    c = h3 >> 26; h3 &= 0x03ffffff; h4 += c;
    c = h4 >> 26; h4 &= 0x03ffffff; h0 += c * 5;
    c = h0 >> 26; h0 &= 0x03ffffff; h1 += c;

    /* Compute h - p (= h - (2^130 - 5)) */
    uint32_t g0 = h0 + 5; c = g0 >> 26; g0 &= 0x03ffffff;
    uint32_t g1 = h1 + c; c = g1 >> 26; g1 &= 0x03ffffff;
    uint32_t g2 = h2 + c; c = g2 >> 26; g2 &= 0x03ffffff;
    uint32_t g3 = h3 + c; c = g3 >> 26; g3 &= 0x03ffffff;
    uint32_t g4 = h4 + c - (1 << 26);

    /* Select h or g based on carry (if g < 2^130, use g; else use h) */
    uint32_t mask = (g4 >> 31) - 1; /* 0xFFFFFFFF if g4 >= 0, 0 if negative */
    g0 &= mask; g1 &= mask; g2 &= mask; g3 &= mask; g4 &= mask;
    mask = ~mask;
    h0 = (h0 & mask) | g0;
    h1 = (h1 & mask) | g1;
    h2 = (h2 & mask) | g2;
    h3 = (h3 & mask) | g3;
    h4 = (h4 & mask) | g4;

    /* Reassemble h from radix-2^26 into 4 x 32-bit words.
     * Each expression is wider than 32 bits due to overlapping limbs,
     * so we MUST mask to 32 bits before the carry-chain (donna pattern).
     * This implicitly truncates h to 128 bits (tag = (h+s) mod 2^128). */
    uint32_t w0 = (uint32_t)(((uint64_t)h0      ) | ((uint64_t)h1 << 26)) & 0xffffffffU;
    uint32_t w1 = (uint32_t)(((uint64_t)h1 >>  6) | ((uint64_t)h2 << 20)) & 0xffffffffU;
    uint32_t w2 = (uint32_t)(((uint64_t)h2 >> 12) | ((uint64_t)h3 << 14)) & 0xffffffffU;
    uint32_t w3 = (uint32_t)(((uint64_t)h3 >> 18) | ((uint64_t)h4 <<  8)) & 0xffffffffU;

    /* Add s with carry propagation: tag = (h + s) mod 2^128 */
    uint64_t f;
    f = (uint64_t)w0 + st->s[0];             w0 = (uint32_t)f;
    f = (uint64_t)w1 + st->s[1] + (f >> 32); w1 = (uint32_t)f;
    f = (uint64_t)w2 + st->s[2] + (f >> 32); w2 = (uint32_t)f;
    f = (uint64_t)w3 + st->s[3] + (f >> 32); w3 = (uint32_t)f;

    /* Write tag (little-endian) */
    store32_le(tag +  0, w0);
    store32_le(tag +  4, w1);
    store32_le(tag +  8, w2);
    store32_le(tag + 12, w3);

    /* Wipe state */
    secure_zero(st, sizeof(*st));
}

/* One-shot Poly1305 */
void poly1305_auth(uint8_t tag[16], const uint8_t *msg, size_t msg_len,
                   const uint8_t key[32]) {
    Poly1305State st;
    poly1305_init(&st, key);
    poly1305_update(&st, msg, msg_len);
    poly1305_finish(&st, tag);
}

/* ═══════════════════════════════════════════════════════════════
 * ChaCha20-Poly1305 AEAD (RFC 8439 §2.8)
 *
 * Construction:
 *   1. Generate Poly1305 one-time key: ChaCha20(key, nonce, counter=0)
 *      Use first 32 bytes of the 64-byte keystream block.
 *   2. Encrypt plaintext with ChaCha20(key, nonce, counter=1)
 *   3. Construct Poly1305 MAC input:
 *      AAD || pad16(AAD) || ciphertext || pad16(CT) || len(AAD) u64le || len(CT) u64le
 *   4. Tag = Poly1305(mac_input, one-time key)
 * ═══════════════════════════════════════════════════════════════ */

/* Pad length to 16-byte boundary — returns number of zero bytes to add */
static size_t pad16(size_t len) {
    size_t rem = len % 16;
    return rem == 0 ? 0 : 16 - rem;
}

int chacha20_poly1305_encrypt(const uint8_t key[32], const uint8_t nonce[12],
                              const uint8_t *aad, size_t aad_len,
                              const uint8_t *plaintext, size_t pt_len,
                              uint8_t *ciphertext, uint8_t tag[16]) {
    if (!key || !nonce || !tag) return -1;
    if (pt_len > 0 && (!plaintext || !ciphertext)) return -1;

    /* Step 1: Generate Poly1305 one-time key (counter=0) */
    ChaCha20State ctx;
    uint8_t poly_key_block[64];
    chacha20_init(&ctx, key, nonce, 0);
    chacha20_block(&ctx, poly_key_block);
    uint8_t poly_key[32];
    memcpy(poly_key, poly_key_block, 32);
    secure_zero(poly_key_block, sizeof(poly_key_block));

    /* Step 2: Encrypt plaintext (counter=1) */
    chacha20_init(&ctx, key, nonce, 1);
    if (pt_len > 0) {
        memcpy(ciphertext, plaintext, pt_len);
        chacha20_crypt(&ctx, ciphertext, pt_len);
    }

    /* Step 3: Compute Poly1305 tag over AAD || pad || CT || pad || lens */
    Poly1305State mac;
    poly1305_init(&mac, poly_key);

    /* AAD */
    if (aad_len > 0) {
        poly1305_update(&mac, aad, aad_len);
    }
    /* Pad AAD to 16-byte boundary */
    {
        size_t p = pad16(aad_len);
        uint8_t zeros[16] = {0};
        if (p > 0) poly1305_update(&mac, zeros, p);
    }

    /* Ciphertext */
    if (pt_len > 0) {
        poly1305_update(&mac, ciphertext, pt_len);
    }
    /* Pad ciphertext to 16-byte boundary */
    {
        size_t p = pad16(pt_len);
        uint8_t zeros[16] = {0};
        if (p > 0) poly1305_update(&mac, zeros, p);
    }

    /* Lengths as 64-bit little-endian */
    {
        uint8_t len_block[16];
        store64_le(len_block, (uint64_t)aad_len);
        store64_le(len_block + 8, (uint64_t)pt_len);
        poly1305_update(&mac, len_block, 16);
    }

    poly1305_finish(&mac, tag);

    /* Wipe key material */
    secure_zero(poly_key, sizeof(poly_key));
    secure_zero(&ctx, sizeof(ctx));

    return 0;
}

int chacha20_poly1305_decrypt(const uint8_t key[32], const uint8_t nonce[12],
                              const uint8_t *aad, size_t aad_len,
                              const uint8_t *ciphertext, size_t ct_len,
                              const uint8_t tag[16], uint8_t *plaintext) {
    if (!key || !nonce || !tag) return -1;
    if (ct_len > 0 && (!ciphertext || !plaintext)) return -1;

    /* Step 1: Generate Poly1305 one-time key (counter=0) */
    ChaCha20State ctx;
    uint8_t poly_key_block[64];
    chacha20_init(&ctx, key, nonce, 0);
    chacha20_block(&ctx, poly_key_block);
    uint8_t poly_key[32];
    memcpy(poly_key, poly_key_block, 32);
    secure_zero(poly_key_block, sizeof(poly_key_block));

    /* Step 2: Verify tag BEFORE decryption */
    Poly1305State mac;
    poly1305_init(&mac, poly_key);

    /* AAD */
    if (aad_len > 0) {
        poly1305_update(&mac, aad, aad_len);
    }
    {
        size_t p = pad16(aad_len);
        uint8_t zeros[16] = {0};
        if (p > 0) poly1305_update(&mac, zeros, p);
    }

    /* Ciphertext */
    if (ct_len > 0) {
        poly1305_update(&mac, ciphertext, ct_len);
    }
    {
        size_t p = pad16(ct_len);
        uint8_t zeros[16] = {0};
        if (p > 0) poly1305_update(&mac, zeros, p);
    }

    /* Lengths */
    {
        uint8_t len_block[16];
        store64_le(len_block, (uint64_t)aad_len);
        store64_le(len_block + 8, (uint64_t)ct_len);
        poly1305_update(&mac, len_block, 16);
    }

    uint8_t computed_tag[16];
    poly1305_finish(&mac, computed_tag);

    /* Constant-time tag comparison */
    if (ct_compare(computed_tag, tag, 16) != 0) {
        /* Authentication failed — do NOT decrypt */
        secure_zero(poly_key, sizeof(poly_key));
        secure_zero(computed_tag, sizeof(computed_tag));
        secure_zero(&ctx, sizeof(ctx));
        return -1;
    }

    /* Step 3: Decrypt (only after authentication succeeds) */
    chacha20_init(&ctx, key, nonce, 1);
    if (ct_len > 0) {
        memcpy(plaintext, ciphertext, ct_len);
        chacha20_crypt(&ctx, plaintext, ct_len);
    }

    /* Wipe key material */
    secure_zero(poly_key, sizeof(poly_key));
    secure_zero(computed_tag, sizeof(computed_tag));
    secure_zero(&ctx, sizeof(ctx));

    return 0;
}

/* ═══════════════════════════════════════════════════════════════
 * Legacy API: Authenticated Encryption with BLAKE3-MAC
 * (DEPRECATED — use chacha20_poly1305_encrypt/decrypt instead)
 *
 * Kept for backward compatibility. This construction uses an
 * unkeyed BLAKE3 hash as a MAC, which is weaker than Poly1305.
 * ═══════════════════════════════════════════════════════════════ */

int chacha20_encrypt_authenticate(const uint8_t key[32], const uint8_t nonce[12],
                                  const uint8_t *plaintext, size_t plain_len,
                                  uint8_t *out, size_t *out_len) {
    if (!key || !nonce || !plaintext || !out || !out_len) return -1;
    if (plain_len > 4000) return -1; /* Sanity limit for prototype */

    /* Encrypt */
    ChaCha20State ctx;
    chacha20_init(&ctx, key, nonce, 1); /* Counter starts at 1 for data */
    memcpy(out, plaintext, plain_len);
    chacha20_crypt(&ctx, out, plain_len);

    /* MAC = BLAKE3(key || nonce || ciphertext) */
    Blake3State mac_ctx;
    blake3_init(&mac_ctx);
    blake3_update(&mac_ctx, key, 32);
    blake3_update(&mac_ctx, nonce, 12);
    blake3_update(&mac_ctx, out, plain_len);
    blake3_finalize(&mac_ctx, out + plain_len);

    *out_len = plain_len + 32; /* ciphertext + 32-byte MAC */

    secure_zero(&ctx, sizeof(ctx));
    return 0;
}

int chacha20_verify_decrypt(const uint8_t key[32], const uint8_t nonce[12],
                            const uint8_t *ciphertext, size_t cipher_len,
                            uint8_t *out, size_t *out_len) {
    if (!key || !nonce || !ciphertext || !out || !out_len) return -1;
    if (cipher_len < 32) return -1; /* Must have at least MAC */

    size_t data_len = cipher_len - 32;
    const uint8_t *mac_received = ciphertext + data_len;

    /* Verify MAC */
    uint8_t mac_computed[32];
    Blake3State mac_ctx;
    blake3_init(&mac_ctx);
    blake3_update(&mac_ctx, key, 32);
    blake3_update(&mac_ctx, nonce, 12);
    blake3_update(&mac_ctx, ciphertext, data_len);
    blake3_finalize(&mac_ctx, mac_computed);

    /* Constant-time comparison */
    if (ct_compare(mac_computed, mac_received, 32) != 0) {
        secure_zero(mac_computed, sizeof(mac_computed));
        return -1; /* Authentication failed */
    }

    /* Decrypt */
    ChaCha20State ctx;
    chacha20_init(&ctx, key, nonce, 1);
    memcpy(out, ciphertext, data_len);
    chacha20_crypt(&ctx, out, data_len);

    *out_len = data_len;

    secure_zero(mac_computed, sizeof(mac_computed));
    secure_zero(&ctx, sizeof(ctx));
    return 0;
}
