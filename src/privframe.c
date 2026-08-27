/*
 * Airbot — Executable Information System
 * privframe.c — Padded AEAD wire frame for privacy mode
 */
#include "privframe.h"
#include "chacha20.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

const uint16_t PF_BUCKETS[PF_NUM_BUCKETS] = { 256, 512, 1024, 2048, 4096, 8192 };

static uint8_t g_key[32];
static int     g_have_key = 0;

/* --- key handling ----------------------------------------- */

int privframe_have_key(void) { return g_have_key; }

int privframe_set_key(const uint8_t key[32]) {
    if (!key) return PF_ERR_NOKEY;
    memcpy(g_key, key, 32);
    g_have_key = 1;
    return PF_OK;
}

static int hexval(int c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

int privframe_set_key_from_env(void) {
    const char *hex = getenv("AIRBOT_PSK_HEX");
    uint8_t k[32];
    int i, hi, lo;

    if (!hex || strlen(hex) != 64) return PF_ERR_NOKEY;
    for (i = 0; i < 32; i++) {
        hi = hexval((unsigned char)hex[i * 2]);
        lo = hexval((unsigned char)hex[i * 2 + 1]);
        if (hi < 0 || lo < 0) return PF_ERR_NOKEY;
        k[i] = (uint8_t)((hi << 4) | lo);
    }
    return privframe_set_key(k);
}

/* --- buckets ---------------------------------------------- */

uint16_t privframe_bucket_for(uint16_t payload_len) {
    uint32_t need = (uint32_t)payload_len + PF_INNER_HDR;
    int i;
    for (i = 0; i < PF_NUM_BUCKETS; i++)
        if (need <= PF_BUCKETS[i]) return PF_BUCKETS[i];
    return 0;
}

const char *privframe_strerror(int code) {
    switch (code) {
        case PF_OK:              return "ok";
        case PF_ERR_NOKEY:       return "no session key (set AIRBOT_PSK_HEX to 64 hex chars)";
        case PF_ERR_TOOBIG:      return "payload exceeds largest size bucket";
        case PF_ERR_SEND:        return "send failed";
        case PF_ERR_RECV:        return "receive failed";
        case PF_ERR_AUTH:        return "authentication failed (forged or corrupted frame)";
        case PF_ERR_MALFORMED:   return "malformed frame length";
        default:                 return "unknown privframe error";
    }
}

/* --- send ------------------------------------------------- */

int privframe_encode(uint8_t type, uint8_t hops_left,
                     const uint8_t *payload, uint16_t length,
                     uint8_t *wire, size_t cap, size_t *out_len) {
    uint8_t inner[8192];
    uint8_t nonce[PF_NONCE_SIZE], tag[PF_TAG_SIZE];
    uint8_t aad[2];
    uint16_t bucket, outer_len;
    size_t n = 0;

    if (!wire || !out_len) return PF_ERR_SEND;
    /* Fail closed: never fall back to an unencrypted frame. */
    if (!g_have_key) return PF_ERR_NOKEY;
    if (length > PF_MAX_PAYLOAD) return PF_ERR_TOOBIG;

    bucket = privframe_bucket_for(length);
    if (bucket == 0) return PF_ERR_TOOBIG;

    /* Inner plaintext, zero-padded to the bucket. Type and hop counter live
       here so neither is visible to a passive observer or a relay. */
    memset(inner, 0, bucket);
    inner[0] = type;
    inner[1] = hops_left;
    inner[2] = (uint8_t)(length >> 8);
    inner[3] = (uint8_t)(length & 0xFF);
    if (length && payload) memcpy(inner + PF_INNER_HDR, payload, length);

    /* Fresh nonce per frame: identical plaintext never produces identical
       wire bytes, which is what breaks cross-hop digest correlation. */
    csprng_bytes(nonce, PF_NONCE_SIZE);

    outer_len = (uint16_t)(PF_NONCE_SIZE + bucket + PF_TAG_SIZE);
    aad[0] = (uint8_t)(outer_len >> 8);
    aad[1] = (uint8_t)(outer_len & 0xFF);

    wire[n++] = aad[0];
    wire[n++] = aad[1];
    memcpy(wire + n, nonce, PF_NONCE_SIZE); n += PF_NONCE_SIZE;

    /* AAD binds the declared length to the ciphertext, so a truncation or
       length-rewrite attempt fails authentication rather than silently
       changing how the receiver parses the stream. */
    if (chacha20_poly1305_encrypt(g_key, nonce, aad, 2,
                                  inner, bucket, wire + n, tag) != 0)
        return PF_ERR_SEND;
    n += bucket;
    memcpy(wire + n, tag, PF_TAG_SIZE); n += PF_TAG_SIZE;

    if (n > cap) return PF_ERR_TOOBIG;
    *out_len = n;
    return PF_OK;
}

int privframe_send(const AirbConn *conn, uint8_t type, uint8_t hops_left,
                   const uint8_t *payload, uint16_t length) {
    uint8_t wire[2 + PF_NONCE_SIZE + 8192 + PF_TAG_SIZE];
    size_t n = 0;
    int rc;
    if (!conn) return PF_ERR_SEND;
    rc = privframe_encode(type, hops_left, payload, length, wire, sizeof(wire), &n);
    if (rc != PF_OK) return rc;
    return transport_send_raw(conn, wire, (uint32_t)n) == AIRB_OK
           ? PF_OK : PF_ERR_SEND;
}

/* Decode a complete frame already held in memory. */
int privframe_decode(const uint8_t *w, size_t wire_len, PrivFrame *out) {
    uint8_t inner[8192], aad[2];
    uint16_t outer_len, bucket, real_len;
    int i, valid_bucket = 0;

    if (!w || !out) return PF_ERR_RECV;
    if (!g_have_key) return PF_ERR_NOKEY;
    memset(out, 0, sizeof(*out));

    if (wire_len < PF_OVERHEAD) return PF_ERR_MALFORMED;
    outer_len = (uint16_t)((w[0] << 8) | w[1]);
    if (outer_len < PF_NONCE_SIZE + PF_TAG_SIZE + 1) return PF_ERR_MALFORMED;
    bucket = (uint16_t)(outer_len - PF_NONCE_SIZE - PF_TAG_SIZE);
    for (i = 0; i < PF_NUM_BUCKETS; i++)
        if (bucket == PF_BUCKETS[i]) { valid_bucket = 1; break; }
    if (!valid_bucket) return PF_ERR_MALFORMED;
    if (wire_len < (size_t)PF_LENPFX_SIZE + outer_len) return PF_ERR_MALFORMED;

    aad[0] = w[0]; aad[1] = w[1];
    if (chacha20_poly1305_decrypt(g_key, w + 2, aad, 2,
                                  w + 2 + PF_NONCE_SIZE, bucket,
                                  w + 2 + PF_NONCE_SIZE + bucket, inner) != 0)
        return PF_ERR_AUTH;

    real_len = (uint16_t)((inner[2] << 8) | inner[3]);
    if ((uint32_t)real_len + PF_INNER_HDR > bucket) return PF_ERR_MALFORMED;
    out->type = inner[0]; out->hops_left = inner[1];
    out->length = real_len; out->bucket = bucket;
    if (real_len) memcpy(out->payload, inner + PF_INNER_HDR, real_len);
    return PF_OK;
}

/* --- receive ---------------------------------------------- */

static int recv_exact(const AirbConn *c, uint8_t *b, uint32_t n) {
    uint32_t got = 0;
    while (got < n) {
        int r = transport_recv_raw(c, b + got, n - got);
        if (r <= 0) return -1;
        got += (uint32_t)r;
    }
    return 0;
}

int privframe_recv(const AirbConn *conn, PrivFrame *out) {
    uint8_t lenpfx[2], nonce[PF_NONCE_SIZE], tag[PF_TAG_SIZE];
    uint8_t ct[8192], inner[8192];
    uint8_t aad[2];
    uint16_t outer_len, bucket, real_len;
    int i, valid_bucket = 0;

    if (!conn || !out) return PF_ERR_RECV;
    if (!g_have_key) return PF_ERR_NOKEY;
    memset(out, 0, sizeof(*out));

    if (recv_exact(conn, lenpfx, 2) != 0) return PF_ERR_RECV;
    outer_len = (uint16_t)((lenpfx[0] << 8) | lenpfx[1]);
    if (outer_len < PF_NONCE_SIZE + PF_TAG_SIZE + 1) return PF_ERR_MALFORMED;

    bucket = (uint16_t)(outer_len - PF_NONCE_SIZE - PF_TAG_SIZE);
    /* Only the six advertised bucket sizes are accepted; anything else is a
       malformed or probing frame and is rejected before any crypto work. */
    for (i = 0; i < PF_NUM_BUCKETS; i++)
        if (bucket == PF_BUCKETS[i]) { valid_bucket = 1; break; }
    if (!valid_bucket) return PF_ERR_MALFORMED;

    if (recv_exact(conn, nonce, PF_NONCE_SIZE) != 0) return PF_ERR_RECV;
    if (recv_exact(conn, ct, bucket) != 0) return PF_ERR_RECV;
    if (recv_exact(conn, tag, PF_TAG_SIZE) != 0) return PF_ERR_RECV;

    aad[0] = lenpfx[0];
    aad[1] = lenpfx[1];

    /* Authenticate before interpreting a single inner byte. */
    if (chacha20_poly1305_decrypt(g_key, nonce, aad, 2, ct, bucket, tag, inner) != 0)
        return PF_ERR_AUTH;

    real_len = (uint16_t)((inner[2] << 8) | inner[3]);
    if ((uint32_t)real_len + PF_INNER_HDR > bucket) return PF_ERR_MALFORMED;

    out->type      = inner[0];
    out->hops_left = inner[1];
    out->length    = real_len;
    out->bucket    = bucket;
    if (real_len) memcpy(out->payload, inner + PF_INNER_HDR, real_len);
    return PF_OK;
}
