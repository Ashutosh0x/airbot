/*
 * Airbot — Executable Information System
 * sha256.c — SHA-256 (FIPS 180-4)
 *
 * Added to support HKDF-SHA256 (RFC 5869). The previous key derivation used
 * HKDF's construction over HMAC-BLAKE3, which is NOT a standard
 * instantiation: no published vectors apply to it and no second
 * implementation exists to differentially test against, so its correctness
 * was unverifiable (blocker C4.4).
 *
 * SHA-256 is chosen precisely because it is verifiable: FIPS 180-4 defines
 * it, RFC 5869 publishes HKDF-SHA256 vectors, and independent
 * implementations (Python hashlib, OpenSSL) exist as oracles.
 *
 * Scope note: conformance to the standard is what these tests establish.
 * They say nothing about constant-time behaviour. SHA-256 here processes
 * public-length data with no secret-dependent control flow, but that is a
 * design property, not a proven one.
 */
#include "sha256.h"
#include <string.h>

static const uint32_t K[64] = {
    0x428a2f98u,0x71374491u,0xb5c0fbcfu,0xe9b5dba5u,0x3956c25bu,0x59f111f1u,
    0x923f82a4u,0xab1c5ed5u,0xd807aa98u,0x12835b01u,0x243185beu,0x550c7dc3u,
    0x72be5d74u,0x80deb1feu,0x9bdc06a7u,0xc19bf174u,0xe49b69c1u,0xefbe4786u,
    0x0fc19dc6u,0x240ca1ccu,0x2de92c6fu,0x4a7484aau,0x5cb0a9dcu,0x76f988dau,
    0x983e5152u,0xa831c66du,0xb00327c8u,0xbf597fc7u,0xc6e00bf3u,0xd5a79147u,
    0x06ca6351u,0x14292967u,0x27b70a85u,0x2e1b2138u,0x4d2c6dfcu,0x53380d13u,
    0x650a7354u,0x766a0abbu,0x81c2c92eu,0x92722c85u,0xa2bfe8a1u,0xa81a664bu,
    0xc24b8b70u,0xc76c51a3u,0xd192e819u,0xd6990624u,0xf40e3585u,0x106aa070u,
    0x19a4c116u,0x1e376c08u,0x2748774cu,0x34b0bcb5u,0x391c0cb3u,0x4ed8aa4au,
    0x5b9cca4fu,0x682e6ff3u,0x748f82eeu,0x78a5636fu,0x84c87814u,0x8cc70208u,
    0x90befffau,0xa4506cebu,0xbef9a3f7u,0xc67178f2u
};

static uint32_t rotr(uint32_t x, int n) { return (x >> n) | (x << (32 - n)); }
static uint32_t ch(uint32_t x, uint32_t y, uint32_t z)  { return (x & y) ^ (~x & z); }
static uint32_t maj(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (x & z) ^ (y & z); }
static uint32_t bsig0(uint32_t x) { return rotr(x,2) ^ rotr(x,13) ^ rotr(x,22); }
static uint32_t bsig1(uint32_t x) { return rotr(x,6) ^ rotr(x,11) ^ rotr(x,25); }
static uint32_t ssig0(uint32_t x) { return rotr(x,7) ^ rotr(x,18) ^ (x >> 3); }
static uint32_t ssig1(uint32_t x) { return rotr(x,17) ^ rotr(x,19) ^ (x >> 10); }

static void compress(uint32_t h[8], const uint8_t block[64]) {
    uint32_t w[64], a,b,c,d,e,f,g,hh, t1, t2;
    int i;

    for (i = 0; i < 16; i++)
        w[i] = ((uint32_t)block[i*4] << 24) | ((uint32_t)block[i*4+1] << 16) |
               ((uint32_t)block[i*4+2] << 8) | (uint32_t)block[i*4+3];
    for (i = 16; i < 64; i++)
        w[i] = ssig1(w[i-2]) + w[i-7] + ssig0(w[i-15]) + w[i-16];

    a=h[0]; b=h[1]; c=h[2]; d=h[3]; e=h[4]; f=h[5]; g=h[6]; hh=h[7];
    for (i = 0; i < 64; i++) {
        t1 = hh + bsig1(e) + ch(e,f,g) + K[i] + w[i];
        t2 = bsig0(a) + maj(a,b,c);
        hh=g; g=f; f=e; e=d+t1; d=c; c=b; b=a; a=t1+t2;
    }
    h[0]+=a; h[1]+=b; h[2]+=c; h[3]+=d; h[4]+=e; h[5]+=f; h[6]+=g; h[7]+=hh;
}

void sha256_init(Sha256State *s) {
    s->h[0]=0x6a09e667u; s->h[1]=0xbb67ae85u; s->h[2]=0x3c6ef372u; s->h[3]=0xa54ff53au;
    s->h[4]=0x510e527fu; s->h[5]=0x9b05688cu; s->h[6]=0x1f83d9abu; s->h[7]=0x5be0cd19u;
    s->buf_len = 0;
    s->total   = 0;
}

void sha256_update(Sha256State *s, const uint8_t *data, size_t len) {
    s->total += (uint64_t)len;
    while (len > 0) {
        size_t want = 64 - s->buf_len;
        size_t take = len < want ? len : want;
        memcpy(s->buf + s->buf_len, data, take);
        s->buf_len += take;
        data += take;
        len  -= take;
        if (s->buf_len == 64) { compress(s->h, s->buf); s->buf_len = 0; }
    }
}

void sha256_final(Sha256State *s, uint8_t out[32]) {
    uint64_t bits = s->total * 8;
    uint8_t pad[72];
    size_t padlen;
    int i;

    /* 0x80, then zeros, so the length lands in the final 8 bytes. */
    padlen = (s->buf_len < 56) ? (56 - s->buf_len) : (120 - s->buf_len);
    memset(pad, 0, sizeof(pad));
    pad[0] = 0x80;
    for (i = 0; i < 8; i++)
        pad[padlen + i] = (uint8_t)(bits >> (56 - 8*i));
    sha256_update(s, pad, padlen + 8);

    for (i = 0; i < 8; i++) {
        out[i*4]   = (uint8_t)(s->h[i] >> 24);
        out[i*4+1] = (uint8_t)(s->h[i] >> 16);
        out[i*4+2] = (uint8_t)(s->h[i] >> 8);
        out[i*4+3] = (uint8_t)(s->h[i]);
    }
}

void sha256(const uint8_t *data, size_t len, uint8_t out[32]) {
    Sha256State s;
    sha256_init(&s);
    sha256_update(&s, data, len);
    sha256_final(&s, out);
}

/* --- HMAC-SHA256 (RFC 2104) ------------------------------- */

void hmac_sha256(const uint8_t *key, size_t key_len,
                 const uint8_t *msg, size_t msg_len,
                 uint8_t out[32]) {
    uint8_t k[64], ipad[64], opad[64], inner[32];
    Sha256State s;
    size_t i;

    memset(k, 0, sizeof(k));
    if (key_len > 64) sha256(key, key_len, k);
    else if (key_len) memcpy(k, key, key_len);

    for (i = 0; i < 64; i++) {
        ipad[i] = (uint8_t)(k[i] ^ 0x36);
        opad[i] = (uint8_t)(k[i] ^ 0x5c);
    }

    sha256_init(&s);
    sha256_update(&s, ipad, 64);
    sha256_update(&s, msg, msg_len);
    sha256_final(&s, inner);

    sha256_init(&s);
    sha256_update(&s, opad, 64);
    sha256_update(&s, inner, 32);
    sha256_final(&s, out);

    memset(k, 0, sizeof(k));
    memset(ipad, 0, sizeof(ipad));
    memset(opad, 0, sizeof(opad));
}

/* --- HKDF-SHA256 (RFC 5869) ------------------------------- */

void hkdf_sha256_extract(const uint8_t *salt, size_t salt_len,
                         const uint8_t *ikm, size_t ikm_len,
                         uint8_t prk[32]) {
    uint8_t zero[32];
    /* RFC 5869 §2.2: if salt is not provided, it is set to HashLen zeros. */
    if (!salt || salt_len == 0) {
        memset(zero, 0, sizeof(zero));
        hmac_sha256(zero, 32, ikm, ikm_len, prk);
    } else {
        hmac_sha256(salt, salt_len, ikm, ikm_len, prk);
    }
}

int hkdf_sha256_expand(const uint8_t prk[32],
                       const uint8_t *info, size_t info_len,
                       uint8_t *okm, size_t okm_len) {
    uint8_t t[32], buf[32 + 512 + 1];
    size_t done = 0, tlen = 0;
    uint8_t counter = 1;

    /* RFC 5869 §2.3: L <= 255 * HashLen. */
    if (okm_len > 255 * 32) return -1;
    if (info_len > 512) return -1;

    while (done < okm_len) {
        size_t n = 0, take;
        if (tlen) { memcpy(buf, t, tlen); n = tlen; }
        if (info_len) { memcpy(buf + n, info, info_len); n += info_len; }
        buf[n++] = counter;

        hmac_sha256(prk, 32, buf, n, t);
        tlen = 32;

        take = okm_len - done;
        if (take > 32) take = 32;
        memcpy(okm + done, t, take);
        done += take;
        counter++;
    }
    memset(t, 0, sizeof(t));
    memset(buf, 0, sizeof(buf));
    return 0;
}

int hkdf_sha256(const uint8_t *salt, size_t salt_len,
                const uint8_t *ikm, size_t ikm_len,
                const uint8_t *info, size_t info_len,
                uint8_t *okm, size_t okm_len) {
    uint8_t prk[32];
    int rc;
    hkdf_sha256_extract(salt, salt_len, ikm, ikm_len, prk);
    rc = hkdf_sha256_expand(prk, info, info_len, okm, okm_len);
    memset(prk, 0, sizeof(prk));
    return rc;
}
