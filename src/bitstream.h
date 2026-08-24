/*
 * Airbot — Executable Information System
 * bitstream.h — Bit-level I/O engine
 *
 * The foundation of the entire system. Provides sub-byte read/write
 * operations on arbitrary bitstreams. All Airbot wire formats and
 * the VM operate through this interface.
 *
 * No dynamic allocation. All operations use caller-provided buffers.
 */

#ifndef AIRBOT_BITSTREAM_H
#define AIRBOT_BITSTREAM_H

#include <stdint.h>
#include <stddef.h>

/* Error codes */
#define AIRBOT_OK             0
#define AIRBOT_ERR_OVERFLOW  -1
#define AIRBOT_ERR_UNDERFLOW -2
#define AIRBOT_ERR_NULLPTR   -3
#define AIRBOT_ERR_BOUNDS    -4
#define AIRBOT_ERR_INVALID   -5

/* ─── BitReader ─────────────────────────────────────────────── */

/*
 * Reads bits from a byte buffer. Maintains a bit-level cursor.
 * Does NOT own the underlying buffer.
 */
typedef struct {
    const uint8_t *data;       /* Source byte buffer (not owned) */
    size_t         byte_len;   /* Total bytes in buffer */
    size_t         bit_pos;    /* Current read position in bits */
    size_t         bit_len;    /* Total bits available */
} BitReader;

/* Initialize a BitReader over an existing byte buffer */
void bitreader_init(BitReader *br, const uint8_t *data, size_t byte_len);

/* Read a single bit (0 or 1). Returns AIRBOT_ERR_OVERFLOW on end. */
int bitreader_read_bit(BitReader *br, uint8_t *out);

/* Read up to 32 bits into a uint32_t. n must be 1..32. */
int bitreader_read_bits(BitReader *br, uint32_t *out, uint8_t n);

/* Read up to 16 bits into a uint16_t. n must be 1..16. */
int bitreader_read_u16(BitReader *br, uint16_t *out, uint8_t n);

/* Read a full byte (8 bits). */
int bitreader_read_byte(BitReader *br, uint8_t *out);

/* Read n bytes into a buffer. */
int bitreader_read_bytes(BitReader *br, uint8_t *out, size_t n);

/* Peek at the next n bits without advancing the cursor. n must be 1..32. */
int bitreader_peek_bits(const BitReader *br, uint32_t *out, uint8_t n);

/* Skip n bits forward. */
int bitreader_skip(BitReader *br, size_t n);

/* Return remaining bits available to read. */
size_t bitreader_remaining(const BitReader *br);

/* Return current bit position. */
size_t bitreader_position(const BitReader *br);

/* Seek to an absolute bit position. */
int bitreader_seek(BitReader *br, size_t bit_pos);

/* Check if reader is at end. */
int bitreader_is_eof(const BitReader *br);

/* ─── BitWriter ─────────────────────────────────────────────── */

/*
 * Writes bits to a byte buffer. Maintains a bit-level cursor.
 * Does NOT own the underlying buffer.
 */
typedef struct {
    uint8_t *data;         /* Destination byte buffer (not owned) */
    size_t   byte_cap;     /* Capacity of buffer in bytes */
    size_t   bit_pos;      /* Current write position in bits */
    size_t   bit_cap;      /* Total capacity in bits */
} BitWriter;

/* Initialize a BitWriter over an existing byte buffer */
void bitwriter_init(BitWriter *bw, uint8_t *data, size_t byte_cap);

/* Write a single bit (0 or 1). */
int bitwriter_write_bit(BitWriter *bw, uint8_t bit);

/* Write up to 32 bits from a uint32_t (MSB-first). n must be 1..32. */
int bitwriter_write_bits(BitWriter *bw, uint32_t value, uint8_t n);

/* Write up to 16 bits from a uint16_t. n must be 1..16. */
int bitwriter_write_u16(BitWriter *bw, uint16_t value, uint8_t n);

/* Write a full byte. */
int bitwriter_write_byte(BitWriter *bw, uint8_t value);

/* Write n bytes from a buffer. */
int bitwriter_write_bytes(BitWriter *bw, const uint8_t *src, size_t n);

/* Return number of bits written so far. */
size_t bitwriter_position(const BitWriter *bw);

/* Return number of full bytes written (rounds up). */
size_t bitwriter_byte_count(const BitWriter *bw);

/* Return remaining capacity in bits. */
size_t bitwriter_remaining(const BitWriter *bw);

/* Flush: zero-pad the final byte if partially written. */
void bitwriter_flush(BitWriter *bw);

/* ─── Bit-Addressable Memory ────────────────────────────────── */

/* Read a single bit from a byte array at bit-address `bit_addr`. */
uint8_t bitmem_read(const uint8_t *mem, size_t bit_addr);

/* Write a single bit to a byte array at bit-address `bit_addr`. */
void bitmem_write(uint8_t *mem, size_t bit_addr, uint8_t value);

/* Flip a single bit at bit-address `bit_addr`. */
void bitmem_flip(uint8_t *mem, size_t bit_addr);

/* Copy `n_bits` from src starting at `src_bit` to dst starting at `dst_bit`. */
int bitmem_copy(uint8_t *dst, size_t dst_bit,
                const uint8_t *src, size_t src_bit, size_t n_bits);

#endif /* AIRBOT_BITSTREAM_H */
