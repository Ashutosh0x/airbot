/*
 * Airbot — Executable Information System
 * tlv.h — TLV Wire Format
 *
 * Implements a Type-Length-Value encoding system.
 */
#ifndef AIRBOT_TLV_H
#define AIRBOT_TLV_H

#include <stdint.h>
#include <stddef.h>
#include "bitstream.h"

/* Tag definitions */
#define TLV_TAG_BEHAVIOR   0x01
#define TLV_TAG_DATA       0x02
#define TLV_TAG_STATE      0x03
#define TLV_TAG_CAPABILITY 0x04
#define TLV_TAG_ADDRESS    0x05
#define TLV_TAG_METADATA   0x06

/**
 * @brief Encode a TLV tag.
 */
int tlv_encode_tag(BitWriter *bw, uint8_t tag);

/**
 * @brief Encode a TLV varint length.
 */
int tlv_encode_length(BitWriter *bw, uint64_t length);

/**
 * @brief Encode a complete TLV block.
 */
int tlv_encode(BitWriter *bw, uint8_t tag, const uint8_t *value, size_t len);

/**
 * @brief Decode a TLV tag.
 */
int tlv_decode_tag(BitReader *br, uint8_t *tag);

/**
 * @brief Decode a TLV varint length.
 */
int tlv_decode_length(BitReader *br, uint64_t *length);

/**
 * @brief Decode a complete TLV block, pointing value_ptr into the buffer.
 */
int tlv_decode(BitReader *br, uint8_t *tag, uint64_t *length, const uint8_t **value_ptr);

/**
 * @brief Skip a TLV block in the reader.
 */
int tlv_skip(BitReader *br);

#endif /* AIRBOT_TLV_H */
