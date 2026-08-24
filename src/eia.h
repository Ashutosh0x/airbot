#ifndef AIRBOT_EIA_H
#define AIRBOT_EIA_H

#include <stdint.h>
#include <stddef.h>

#define EIA_MAGIC         0xEA
#define EIA_MAX_BYTECODE  256

#define EIA_TYPE_HAS_DIGEST     (1 << 0)
#define EIA_TYPE_HAS_CAPABILITY (1 << 1)
#define EIA_TYPE_HAS_BYTECODE   (1 << 2)
#define EIA_TYPE_SELF_CERTIFY   (1 << 3)

typedef struct {
    uint8_t  magic;              /* 0xEA */
    uint8_t  type_flags;         /* EIA_TYPE_* flags */
    uint8_t  gas_budget;         /* Gas for routing bytecode */
    uint8_t  total_len;          /* Total length */
    uint8_t  content_digest[32]; /* BLAKE3-256 of target content */
    uint64_t cap_badge;          /* Capability rights for this address */
    uint8_t  bytecode[EIA_MAX_BYTECODE]; /* Routing/constraint bytecode */
    uint16_t bytecode_len;       /* Bytecode length */
} EIA;

typedef enum {
    EIA_ACTION_FORWARD  = 0,  /* Forward to next hop */
    EIA_ACTION_DELIVER  = 1,  /* Deliver to local node */
    EIA_ACTION_DROP     = 2,  /* Drop the unit */
    EIA_ACTION_TRANSFORM = 3  /* Transform payload */
} EIAAction;

typedef struct {
    EIAAction action;        /* Routing decision */
    uint32_t  next_hop;      /* Next hop node ID (for FORWARD) */
    int       authorized;    /* 1 if capability check passed */
    uint8_t   fuel_used;     /* Gas consumed by routing code */
} EIAResolution;

/* Initialize EIA */
void eia_init(EIA *eia);

/* Set content digest */
int eia_set_digest(EIA *eia, const uint8_t digest[32]);

/* Set capability badge */
int eia_set_capability(EIA *eia, uint64_t cap_badge);

/* Set routing bytecode */
int eia_set_bytecode(EIA *eia, const uint8_t *code, uint16_t len);

/* Verify self-certifying digest */
int eia_verify(const EIA *eia);

/* Resolve EIA routing */
int eia_resolve(const EIA *eia, uint32_t node_id, uint64_t timestamp, EIAResolution *result);

/* Serialize EIA to bytes */
int eia_serialize(const EIA *eia, uint8_t *buf, size_t cap, size_t *out_len);

/* Deserialize EIA from bytes */
int eia_deserialize(EIA *eia, const uint8_t *buf, size_t len);

#endif /* AIRBOT_EIA_H */
