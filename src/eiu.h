#ifndef AIRBOT_EIU_H
#define AIRBOT_EIU_H

#include <stdint.h>
#include <stddef.h>
#include "capability.h"

/* It is assumed bitstream.h, tlv.h, blake3.h are available as requested. 
 * Defining necessary values inline to ensure functionality. */

#define EIU_MAGIC       0xAB01
#define EIU_VERSION     0x01
#define EIU_MAX_SIZE    4096  /* Maximum EIU size in bytes */
#define EIU_MAX_BEHAVIOR 1024 /* Max behavior section bytes */
#define EIU_MAX_DATA     2048 /* Max data section bytes */
#define EIU_MAX_STATE    512  /* Max state section bytes */

/* Flags byte bits */
#define EIU_FLAG_HAS_DATA       (1 << 0)
#define EIU_FLAG_HAS_BEHAVIOR   (1 << 1)
#define EIU_FLAG_HAS_STATE      (1 << 2)
#define EIU_FLAG_HAS_CAPABILITY (1 << 3)
#define EIU_FLAG_HAS_ADDRESS    (1 << 4)

/* Section TLV tags */
#define TLV_TAG_BEHAVIOR   0x01
#define TLV_TAG_DATA       0x02
#define TLV_TAG_STATE      0x03
#define TLV_TAG_CAPABILITY 0x04
#define TLV_TAG_ADDRESS    0x05

typedef struct {
    /* Header fields */
    uint16_t magic;          /* 0xAB01 */
    uint8_t  version;        /* Protocol version */
    uint8_t  flags;          /* Section presence flags */
    uint16_t fuel;           /* Gas budget for execution */
    uint8_t  total_len;      /* Total length in bytes (for small EIUs, or 0xFF for extended) */
    uint8_t  header_crc;     /* Simple CRC-8 of header bytes */
    
    /* Sections (stored in raw buffer) */
    uint8_t  behavior[EIU_MAX_BEHAVIOR];
    uint16_t behavior_len;   /* Length in bytes */
    
    uint8_t  data[EIU_MAX_DATA];
    uint16_t data_len;
    
    uint8_t  state[EIU_MAX_STATE];
    uint16_t state_len;
    
    Capability capability;   /* Capability section */
    
    /* Content hash (BLAKE3-256) */
    uint8_t  content_hash[32];
    int      hash_valid;     /* 1 if content_hash has been computed */
} EIU;

/* Zero-initialize all fields, set magic and version */
void eiu_init(EIU *eiu);

/* Set behavior section */
int eiu_set_behavior(EIU *eiu, const uint8_t *code, uint16_t len);

/* Set data section */
int eiu_set_data(EIU *eiu, const uint8_t *data, uint16_t len);

/* Set state section */
int eiu_set_state(EIU *eiu, const uint8_t *state, uint16_t len);

/* Set capability */
int eiu_set_capability(EIU *eiu, const Capability *cap);

/* Set gas budget */
int eiu_set_fuel(EIU *eiu, uint16_t fuel);

/* Serialize to wire format using TLV encoding. Returns 0 on success. */
int eiu_serialize(const EIU *eiu, uint8_t *buf, size_t buf_cap, size_t *out_len);

/* Deserialize from wire format. Returns 0 on success. */
int eiu_deserialize(EIU *eiu, const uint8_t *buf, size_t buf_len);

/* Validate structural integrity. Returns 1 if valid, 0 otherwise. */
int eiu_validate(const EIU *eiu);

/* CRC-8 for header */
uint8_t eiu_compute_crc8(const uint8_t *data, size_t len);

/* Compute BLAKE3-256 content hash */
void eiu_compute_hash(EIU *eiu);

/* Returns 0 if equal */
int eiu_compare(const EIU *a, const EIU *b);

/* Total size in bits */
size_t eiu_total_bits(const EIU *eiu);

#endif /* AIRBOT_EIU_H */
