/*
 * Airbot — Executable Information System
 * tlv.c — TLV Wire Format
 */

#include "tlv.h"
#include "bitstream.h"

int tlv_encode_tag(BitWriter *bw, uint8_t tag) {
    return bitwriter_write_byte(bw, tag);
}

int tlv_encode_length(BitWriter *bw, uint64_t length) {
    if (length <= 0x3F) {
        return bitwriter_write_byte(bw, (uint8_t)(length));
    } else if (length <= 0x3FFF) {
        uint16_t val = (0x01 << 14) | length;
        if (bitwriter_write_byte(bw, (val >> 8) & 0xFF) != AIRBOT_OK) return AIRBOT_ERR_OVERFLOW;
        return bitwriter_write_byte(bw, val & 0xFF);
    } else if (length <= 0x3FFFFFFF) {
        uint32_t val = (0x02U << 30) | (uint32_t)length;
        if (bitwriter_write_byte(bw, (val >> 24) & 0xFF) != AIRBOT_OK) return AIRBOT_ERR_OVERFLOW;
        if (bitwriter_write_byte(bw, (val >> 16) & 0xFF) != AIRBOT_OK) return AIRBOT_ERR_OVERFLOW;
        if (bitwriter_write_byte(bw, (val >> 8) & 0xFF) != AIRBOT_OK) return AIRBOT_ERR_OVERFLOW;
        return bitwriter_write_byte(bw, val & 0xFF);
    } else {
        uint64_t val = (0x03ULL << 62) | length;
        if (bitwriter_write_byte(bw, (val >> 56) & 0xFF) != AIRBOT_OK) return AIRBOT_ERR_OVERFLOW;
        if (bitwriter_write_byte(bw, (val >> 48) & 0xFF) != AIRBOT_OK) return AIRBOT_ERR_OVERFLOW;
        if (bitwriter_write_byte(bw, (val >> 40) & 0xFF) != AIRBOT_OK) return AIRBOT_ERR_OVERFLOW;
        if (bitwriter_write_byte(bw, (val >> 32) & 0xFF) != AIRBOT_OK) return AIRBOT_ERR_OVERFLOW;
        if (bitwriter_write_byte(bw, (val >> 24) & 0xFF) != AIRBOT_OK) return AIRBOT_ERR_OVERFLOW;
        if (bitwriter_write_byte(bw, (val >> 16) & 0xFF) != AIRBOT_OK) return AIRBOT_ERR_OVERFLOW;
        if (bitwriter_write_byte(bw, (val >> 8) & 0xFF) != AIRBOT_OK) return AIRBOT_ERR_OVERFLOW;
        return bitwriter_write_byte(bw, val & 0xFF);
    }
}

int tlv_encode(BitWriter *bw, uint8_t tag, const uint8_t *value, size_t len) {
    if (tlv_encode_tag(bw, tag) != AIRBOT_OK) return AIRBOT_ERR_OVERFLOW;
    if (tlv_encode_length(bw, len) != AIRBOT_OK) return AIRBOT_ERR_OVERFLOW;
    for (size_t i = 0; i < len; i++) {
        if (bitwriter_write_byte(bw, value[i]) != AIRBOT_OK) return AIRBOT_ERR_OVERFLOW;
    }
    return AIRBOT_OK;
}

int tlv_decode_tag(BitReader *br, uint8_t *tag) {
    return bitreader_read_byte(br, tag);
}

int tlv_decode_length(BitReader *br, uint64_t *length) {
    uint8_t first_byte;
    if (bitreader_read_byte(br, &first_byte) != AIRBOT_OK) return AIRBOT_ERR_UNDERFLOW;
    
    uint8_t prefix = first_byte >> 6;
    if (prefix == 0) {
        *length = first_byte & 0x3F;
    } else if (prefix == 1) {
        uint8_t b1;
        if (bitreader_read_byte(br, &b1) != AIRBOT_OK) return AIRBOT_ERR_UNDERFLOW;
        *length = ((uint16_t)(first_byte & 0x3F) << 8) | b1;
    } else if (prefix == 2) {
        uint8_t b1, b2, b3;
        if (bitreader_read_byte(br, &b1) != AIRBOT_OK) return AIRBOT_ERR_UNDERFLOW;
        if (bitreader_read_byte(br, &b2) != AIRBOT_OK) return AIRBOT_ERR_UNDERFLOW;
        if (bitreader_read_byte(br, &b3) != AIRBOT_OK) return AIRBOT_ERR_UNDERFLOW;
        *length = ((uint32_t)(first_byte & 0x3F) << 24) | ((uint32_t)b1 << 16) | ((uint32_t)b2 << 8) | b3;
    } else {
        uint8_t b1, b2, b3, b4, b5, b6, b7;
        if (bitreader_read_byte(br, &b1) != AIRBOT_OK) return AIRBOT_ERR_UNDERFLOW;
        if (bitreader_read_byte(br, &b2) != AIRBOT_OK) return AIRBOT_ERR_UNDERFLOW;
        if (bitreader_read_byte(br, &b3) != AIRBOT_OK) return AIRBOT_ERR_UNDERFLOW;
        if (bitreader_read_byte(br, &b4) != AIRBOT_OK) return AIRBOT_ERR_UNDERFLOW;
        if (bitreader_read_byte(br, &b5) != AIRBOT_OK) return AIRBOT_ERR_UNDERFLOW;
        if (bitreader_read_byte(br, &b6) != AIRBOT_OK) return AIRBOT_ERR_UNDERFLOW;
        if (bitreader_read_byte(br, &b7) != AIRBOT_OK) return AIRBOT_ERR_UNDERFLOW;
        *length = ((uint64_t)(first_byte & 0x3F) << 56) | ((uint64_t)b1 << 48) |
                  ((uint64_t)b2 << 40) | ((uint64_t)b3 << 32) |
                  ((uint64_t)b4 << 24) | ((uint64_t)b5 << 16) |
                  ((uint64_t)b6 << 8)  | b7;
    }
    return AIRBOT_OK;
}

int tlv_decode(BitReader *br, uint8_t *tag, uint64_t *length, const uint8_t **value_ptr) {
    if (tlv_decode_tag(br, tag) != AIRBOT_OK) return AIRBOT_ERR_UNDERFLOW;
    if (tlv_decode_length(br, length) != AIRBOT_OK) return AIRBOT_ERR_UNDERFLOW;
    
    /* Ensure byte alignment before pointing to buffer */
    if (br->bit_pos % 8 != 0) return AIRBOT_ERR_INVALID;
    
    size_t byte_pos = br->bit_pos / 8;
    if (byte_pos + *length > br->byte_len) return AIRBOT_ERR_UNDERFLOW;
    
    *value_ptr = br->data + byte_pos;
    return bitreader_skip(br, *length * 8);
}

int tlv_skip(BitReader *br) {
    uint8_t tag;
    uint64_t length;
    if (tlv_decode_tag(br, &tag) != AIRBOT_OK) return AIRBOT_ERR_UNDERFLOW;
    if (tlv_decode_length(br, &length) != AIRBOT_OK) return AIRBOT_ERR_UNDERFLOW;
    return bitreader_skip(br, length * 8);
}
