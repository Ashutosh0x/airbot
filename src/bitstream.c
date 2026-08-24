/*
 * Airbot — Executable Information System
 * bitstream.c — Bit-level I/O engine implementation
 *
 * Pure C99. No dynamic allocation. No external dependencies.
 */

#include "bitstream.h"
#include <string.h>

/* ═══════════════════════════════════════════════════════════════
 * BitReader Implementation
 * ═══════════════════════════════════════════════════════════════ */

void bitreader_init(BitReader *br, const uint8_t *data, size_t byte_len) {
    br->data     = data;
    br->byte_len = byte_len;
    br->bit_pos  = 0;
    br->bit_len  = byte_len * 8;
}

int bitreader_read_bit(BitReader *br, uint8_t *out) {
    if (!br || !out) return AIRBOT_ERR_NULLPTR;
    if (br->bit_pos >= br->bit_len) return AIRBOT_ERR_OVERFLOW;

    size_t byte_idx = br->bit_pos / 8;
    uint8_t bit_idx = 7 - (uint8_t)(br->bit_pos % 8); /* MSB-first */
    *out = (br->data[byte_idx] >> bit_idx) & 1;
    br->bit_pos++;
    return AIRBOT_OK;
}

int bitreader_read_bits(BitReader *br, uint32_t *out, uint8_t n) {
    if (!br || !out) return AIRBOT_ERR_NULLPTR;
    if (n == 0 || n > 32) return AIRBOT_ERR_INVALID;
    if (br->bit_pos + n > br->bit_len) return AIRBOT_ERR_OVERFLOW;

    uint32_t result = 0;
    for (uint8_t i = 0; i < n; i++) {
        size_t byte_idx = br->bit_pos / 8;
        uint8_t bit_idx = 7 - (uint8_t)(br->bit_pos % 8);
        uint32_t bit = (br->data[byte_idx] >> bit_idx) & 1;
        result = (result << 1) | bit;
        br->bit_pos++;
    }
    *out = result;
    return AIRBOT_OK;
}

int bitreader_read_u16(BitReader *br, uint16_t *out, uint8_t n) {
    if (n > 16) return AIRBOT_ERR_INVALID;
    uint32_t tmp;
    int rc = bitreader_read_bits(br, &tmp, n);
    if (rc == AIRBOT_OK) *out = (uint16_t)tmp;
    return rc;
}

int bitreader_read_byte(BitReader *br, uint8_t *out) {
    uint32_t tmp;
    int rc = bitreader_read_bits(br, &tmp, 8);
    if (rc == AIRBOT_OK) *out = (uint8_t)tmp;
    return rc;
}

int bitreader_read_bytes(BitReader *br, uint8_t *out, size_t n) {
    if (!br || !out) return AIRBOT_ERR_NULLPTR;
    for (size_t i = 0; i < n; i++) {
        int rc = bitreader_read_byte(br, &out[i]);
        if (rc != AIRBOT_OK) return rc;
    }
    return AIRBOT_OK;
}

int bitreader_peek_bits(const BitReader *br, uint32_t *out, uint8_t n) {
    if (!br || !out) return AIRBOT_ERR_NULLPTR;
    if (n == 0 || n > 32) return AIRBOT_ERR_INVALID;
    if (br->bit_pos + n > br->bit_len) return AIRBOT_ERR_OVERFLOW;

    uint32_t result = 0;
    size_t pos = br->bit_pos;
    for (uint8_t i = 0; i < n; i++) {
        size_t byte_idx = pos / 8;
        uint8_t bit_idx = 7 - (uint8_t)(pos % 8);
        uint32_t bit = (br->data[byte_idx] >> bit_idx) & 1;
        result = (result << 1) | bit;
        pos++;
    }
    *out = result;
    return AIRBOT_OK;
}

int bitreader_skip(BitReader *br, size_t n) {
    if (!br) return AIRBOT_ERR_NULLPTR;
    if (br->bit_pos + n > br->bit_len) return AIRBOT_ERR_OVERFLOW;
    br->bit_pos += n;
    return AIRBOT_OK;
}

size_t bitreader_remaining(const BitReader *br) {
    if (!br || br->bit_pos >= br->bit_len) return 0;
    return br->bit_len - br->bit_pos;
}

size_t bitreader_position(const BitReader *br) {
    return br ? br->bit_pos : 0;
}

int bitreader_seek(BitReader *br, size_t bit_pos) {
    if (!br) return AIRBOT_ERR_NULLPTR;
    if (bit_pos > br->bit_len) return AIRBOT_ERR_BOUNDS;
    br->bit_pos = bit_pos;
    return AIRBOT_OK;
}

int bitreader_is_eof(const BitReader *br) {
    return (!br || br->bit_pos >= br->bit_len) ? 1 : 0;
}

/* ═══════════════════════════════════════════════════════════════
 * BitWriter Implementation
 * ═══════════════════════════════════════════════════════════════ */

void bitwriter_init(BitWriter *bw, uint8_t *data, size_t byte_cap) {
    bw->data     = data;
    bw->byte_cap = byte_cap;
    bw->bit_pos  = 0;
    bw->bit_cap  = byte_cap * 8;
    memset(data, 0, byte_cap); /* Zero-initialize the buffer */
}

int bitwriter_write_bit(BitWriter *bw, uint8_t bit) {
    if (!bw) return AIRBOT_ERR_NULLPTR;
    if (bw->bit_pos >= bw->bit_cap) return AIRBOT_ERR_OVERFLOW;

    size_t byte_idx = bw->bit_pos / 8;
    uint8_t bit_idx = 7 - (uint8_t)(bw->bit_pos % 8); /* MSB-first */
    if (bit & 1) {
        bw->data[byte_idx] |= (uint8_t)(1 << bit_idx);
    } else {
        bw->data[byte_idx] &= (uint8_t)~(1 << bit_idx);
    }
    bw->bit_pos++;
    return AIRBOT_OK;
}

int bitwriter_write_bits(BitWriter *bw, uint32_t value, uint8_t n) {
    if (!bw) return AIRBOT_ERR_NULLPTR;
    if (n == 0 || n > 32) return AIRBOT_ERR_INVALID;
    if (bw->bit_pos + n > bw->bit_cap) return AIRBOT_ERR_OVERFLOW;

    /* Write MSB-first */
    for (uint8_t i = 0; i < n; i++) {
        uint8_t bit = (uint8_t)((value >> (n - 1 - i)) & 1);
        int rc = bitwriter_write_bit(bw, bit);
        if (rc != AIRBOT_OK) return rc;
    }
    return AIRBOT_OK;
}

int bitwriter_write_u16(BitWriter *bw, uint16_t value, uint8_t n) {
    if (n > 16) return AIRBOT_ERR_INVALID;
    return bitwriter_write_bits(bw, (uint32_t)value, n);
}

int bitwriter_write_byte(BitWriter *bw, uint8_t value) {
    return bitwriter_write_bits(bw, (uint32_t)value, 8);
}

int bitwriter_write_bytes(BitWriter *bw, const uint8_t *src, size_t n) {
    if (!bw || !src) return AIRBOT_ERR_NULLPTR;
    for (size_t i = 0; i < n; i++) {
        int rc = bitwriter_write_byte(bw, src[i]);
        if (rc != AIRBOT_OK) return rc;
    }
    return AIRBOT_OK;
}

size_t bitwriter_position(const BitWriter *bw) {
    return bw ? bw->bit_pos : 0;
}

size_t bitwriter_byte_count(const BitWriter *bw) {
    if (!bw) return 0;
    return (bw->bit_pos + 7) / 8;
}

size_t bitwriter_remaining(const BitWriter *bw) {
    if (!bw || bw->bit_pos >= bw->bit_cap) return 0;
    return bw->bit_cap - bw->bit_pos;
}

void bitwriter_flush(BitWriter *bw) {
    /* Nothing special needed — buffer is zero-initialized,
     * so partial bytes are already zero-padded. */
    (void)bw;
}

/* ═══════════════════════════════════════════════════════════════
 * Bit-Addressable Memory Operations
 * ═══════════════════════════════════════════════════════════════ */

uint8_t bitmem_read(const uint8_t *mem, size_t bit_addr) {
    size_t byte_idx = bit_addr / 8;
    uint8_t bit_idx = 7 - (uint8_t)(bit_addr % 8);
    return (mem[byte_idx] >> bit_idx) & 1;
}

void bitmem_write(uint8_t *mem, size_t bit_addr, uint8_t value) {
    size_t byte_idx = bit_addr / 8;
    uint8_t bit_idx = 7 - (uint8_t)(bit_addr % 8);
    if (value & 1) {
        mem[byte_idx] |= (uint8_t)(1 << bit_idx);
    } else {
        mem[byte_idx] &= (uint8_t)~(1 << bit_idx);
    }
}

void bitmem_flip(uint8_t *mem, size_t bit_addr) {
    size_t byte_idx = bit_addr / 8;
    uint8_t bit_idx = 7 - (uint8_t)(bit_addr % 8);
    mem[byte_idx] ^= (uint8_t)(1 << bit_idx);
}

int bitmem_copy(uint8_t *dst, size_t dst_bit,
                const uint8_t *src, size_t src_bit, size_t n_bits) {
    if (!dst || !src) return AIRBOT_ERR_NULLPTR;
    for (size_t i = 0; i < n_bits; i++) {
        uint8_t val = bitmem_read(src, src_bit + i);
        bitmem_write(dst, dst_bit + i, val);
    }
    return AIRBOT_OK;
}
