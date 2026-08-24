#include "eiu.h"
#include "blake3.h"
#include <string.h>

/* Standard initializations */
void eiu_init(EIU *eiu) {
    if (!eiu) return;
    memset(eiu, 0, sizeof(EIU));
    eiu->magic = EIU_MAGIC;
    eiu->version = EIU_VERSION;
    eiu->flags = 0;
}

int eiu_set_behavior(EIU *eiu, const uint8_t *code, uint16_t len) {
    if (!eiu || len > EIU_MAX_BEHAVIOR) return -1;
    if (code && len > 0) {
        memcpy(eiu->behavior, code, len);
        eiu->behavior_len = len;
        eiu->flags |= EIU_FLAG_HAS_BEHAVIOR;
    }
    return 0;
}

int eiu_set_data(EIU *eiu, const uint8_t *data, uint16_t len) {
    if (!eiu || len > EIU_MAX_DATA) return -1;
    if (data && len > 0) {
        memcpy(eiu->data, data, len);
        eiu->data_len = len;
        eiu->flags |= EIU_FLAG_HAS_DATA;
    }
    return 0;
}

int eiu_set_state(EIU *eiu, const uint8_t *state, uint16_t len) {
    if (!eiu || len > EIU_MAX_STATE) return -1;
    if (state && len > 0) {
        memcpy(eiu->state, state, len);
        eiu->state_len = len;
        eiu->flags |= EIU_FLAG_HAS_STATE;
    }
    return 0;
}

int eiu_set_capability(EIU *eiu, const Capability *cap) {
    if (!eiu || !cap) return -1;
    eiu->capability = *cap;
    eiu->flags |= EIU_FLAG_HAS_CAPABILITY;
    return 0;
}

int eiu_set_fuel(EIU *eiu, uint16_t fuel) {
    if (!eiu) return -1;
    eiu->fuel = fuel;
    return 0;
}

uint8_t eiu_compute_crc8(const uint8_t *data, size_t len) {
    uint8_t crc = 0;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 0x80)
                crc = (crc << 1) ^ 0x07;
            else
                crc <<= 1;
        }
    }
    return crc;
}

void eiu_compute_hash(EIU *eiu) {
    if (!eiu) return;
    /* Hash all EIU content using BLAKE3 */
    Blake3State hasher;
    blake3_init(&hasher);
    /* Hash header fields */
    uint8_t header[8];
    header[0] = (uint8_t)(eiu->magic >> 8);
    header[1] = (uint8_t)(eiu->magic & 0xFF);
    header[2] = eiu->version;
    header[3] = eiu->flags;
    header[4] = (uint8_t)(eiu->fuel >> 8);
    header[5] = (uint8_t)(eiu->fuel & 0xFF);
    header[6] = 0;
    header[7] = 0;
    blake3_update(&hasher, header, 8);
    /* Hash behavior section */
    if (eiu->behavior_len > 0) {
        blake3_update(&hasher, eiu->behavior, eiu->behavior_len);
    }
    /* Hash data section */
    if (eiu->data_len > 0) {
        blake3_update(&hasher, eiu->data, eiu->data_len);
    }
    /* Hash state section — this is what changes during evolution */
    if (eiu->state_len > 0) {
        blake3_update(&hasher, eiu->state, eiu->state_len);
    }
    blake3_finalize(&hasher, eiu->content_hash);
    eiu->hash_valid = 1;
}

int eiu_compare(const EIU *a, const EIU *b) {
    if (!a || !b) return -1;
    if (a->hash_valid && b->hash_valid) {
        return memcmp(a->content_hash, b->content_hash, 32);
    }
    return -1;
}

size_t eiu_total_bits(const EIU *eiu) {
    if (!eiu) return 0;
    size_t bytes = 8; /* Header size */
    
    if (eiu->flags & EIU_FLAG_HAS_BEHAVIOR) bytes += 3 + eiu->behavior_len; /* 1 byte tag + 2 bytes len + data */
    if (eiu->flags & EIU_FLAG_HAS_DATA)     bytes += 3 + eiu->data_len;
    if (eiu->flags & EIU_FLAG_HAS_STATE)    bytes += 3 + eiu->state_len;
    if (eiu->flags & EIU_FLAG_HAS_CAPABILITY) bytes += 3 + sizeof(Capability);
    
    return bytes * 8;
}

int eiu_serialize(const EIU *eiu, uint8_t *buf, size_t buf_cap, size_t *out_len) {
    if (!eiu || !buf || !out_len) return -1;
    
    size_t req_bytes = eiu_total_bits(eiu) / 8;
    if (buf_cap < req_bytes) return -1;
    
    /* 8-byte header: magic BE, version, flags, fuel BE, total_len, crc */
    buf[0] = (eiu->magic >> 8) & 0xFF;
    buf[1] = eiu->magic & 0xFF;
    buf[2] = eiu->version;
    buf[3] = eiu->flags;
    buf[4] = (eiu->fuel >> 8) & 0xFF;
    buf[5] = eiu->fuel & 0xFF;
    buf[6] = (req_bytes <= 254) ? (uint8_t)req_bytes : 0xFF;
    
    buf[7] = eiu_compute_crc8(buf, 7);
    
    size_t offset = 8;
    
    if (eiu->flags & EIU_FLAG_HAS_BEHAVIOR) {
        buf[offset++] = TLV_TAG_BEHAVIOR;
        buf[offset++] = (eiu->behavior_len >> 8) & 0xFF;
        buf[offset++] = eiu->behavior_len & 0xFF;
        memcpy(buf + offset, eiu->behavior, eiu->behavior_len);
        offset += eiu->behavior_len;
    }
    
    if (eiu->flags & EIU_FLAG_HAS_DATA) {
        buf[offset++] = TLV_TAG_DATA;
        buf[offset++] = (eiu->data_len >> 8) & 0xFF;
        buf[offset++] = eiu->data_len & 0xFF;
        memcpy(buf + offset, eiu->data, eiu->data_len);
        offset += eiu->data_len;
    }
    
    if (eiu->flags & EIU_FLAG_HAS_STATE) {
        buf[offset++] = TLV_TAG_STATE;
        buf[offset++] = (eiu->state_len >> 8) & 0xFF;
        buf[offset++] = eiu->state_len & 0xFF;
        memcpy(buf + offset, eiu->state, eiu->state_len);
        offset += eiu->state_len;
    }
    
    if (eiu->flags & EIU_FLAG_HAS_CAPABILITY) {
        size_t cap_len = sizeof(Capability);
        buf[offset++] = TLV_TAG_CAPABILITY;
        buf[offset++] = (cap_len >> 8) & 0xFF;
        buf[offset++] = cap_len & 0xFF;
        memcpy(buf + offset, &eiu->capability, cap_len);
        offset += cap_len;
    }
    
    *out_len = offset;
    return 0;
}

int eiu_deserialize(EIU *eiu, const uint8_t *buf, size_t buf_len) {
    if (!eiu || !buf || buf_len < 8) return -1;
    
    uint8_t calc_crc = eiu_compute_crc8(buf, 7);
    if (calc_crc != buf[7]) return -1;
    
    eiu_init(eiu);
    
    eiu->magic = (buf[0] << 8) | buf[1];
    eiu->version = buf[2];
    eiu->flags = buf[3];
    eiu->fuel = (buf[4] << 8) | buf[5];
    eiu->total_len = buf[6];
    eiu->header_crc = buf[7];
    
    if (eiu->magic != EIU_MAGIC) return -1;
    
    size_t offset = 8;
    while (offset + 3 <= buf_len) {
        uint8_t tag = buf[offset++];
        uint16_t len = (buf[offset] << 8) | buf[offset + 1];
        offset += 2;
        
        if (offset + len > buf_len) return -1; /* Invalid structure */
        
        switch (tag) {
            case TLV_TAG_BEHAVIOR:
                eiu_set_behavior(eiu, buf + offset, len);
                break;
            case TLV_TAG_DATA:
                eiu_set_data(eiu, buf + offset, len);
                break;
            case TLV_TAG_STATE:
                eiu_set_state(eiu, buf + offset, len);
                break;
            case TLV_TAG_CAPABILITY:
                if (len == sizeof(Capability)) {
                    Capability cap;
                    memcpy(&cap, buf + offset, len);
                    eiu_set_capability(eiu, &cap);
                }
                break;
            default:
                break; /* Unknown tag, skip */
        }
        offset += len;
    }
    
    return 0;
}

int eiu_validate(const EIU *eiu) {
    if (!eiu) return 0;
    if (eiu->magic != EIU_MAGIC) return 0;
    if (eiu->version != EIU_VERSION) return 0;
    if (eiu->behavior_len > EIU_MAX_BEHAVIOR) return 0;
    if (eiu->data_len > EIU_MAX_DATA) return 0;
    if (eiu->state_len > EIU_MAX_STATE) return 0;
    return 1;
}
