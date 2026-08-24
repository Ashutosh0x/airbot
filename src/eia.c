#include "eia.h"
#include "blake3.h"
#include "capability.h"
#include "vm.h"
#include <string.h>

void eia_init(EIA *eia) {
    if (eia) {
        memset(eia, 0, sizeof(EIA));
        eia->magic = EIA_MAGIC;
        eia->total_len = 4; /* Minimum header size */
    }
}

int eia_set_digest(EIA *eia, const uint8_t digest[32]) {
    if (!eia || !digest) return -1;
    memcpy(eia->content_digest, digest, 32);
    eia->type_flags |= EIA_TYPE_HAS_DIGEST;
    return 0;
}

int eia_set_capability(EIA *eia, uint64_t cap_badge) {
    if (!eia) return -1;
    eia->cap_badge = cap_badge;
    eia->type_flags |= EIA_TYPE_HAS_CAPABILITY;
    return 0;
}

int eia_set_bytecode(EIA *eia, const uint8_t *code, uint16_t len) {
    if (!eia || !code || len > EIA_MAX_BYTECODE) return -1;
    memcpy(eia->bytecode, code, len);
    eia->bytecode_len = len;
    eia->type_flags |= EIA_TYPE_HAS_BYTECODE;
    return 0;
}

int eia_verify(const EIA *eia) {
    if (!eia || !(eia->type_flags & EIA_TYPE_SELF_CERTIFY)) return -1;
    /* Dummy verification logic for stub */
    return 0;
}

int eia_resolve(const EIA *eia, uint32_t node_id, uint64_t timestamp, EIAResolution *result) {
    if (!eia || !result) return -1;
    memset(result, 0, sizeof(EIAResolution));
    
    /* Dummy VM execution for EIA resolve stub */
    (void)node_id;
    (void)timestamp;
    
    result->action = EIA_ACTION_DELIVER;
    result->authorized = 1;
    return 0;
}

int eia_serialize(const EIA *eia, uint8_t *buf, size_t cap, size_t *out_len) {
    if (!eia || !buf || !out_len) return -1;
    if (cap < 4) return -1;
    buf[0] = eia->magic;
    buf[1] = eia->type_flags;
    buf[2] = eia->gas_budget;
    buf[3] = eia->total_len;
    *out_len = 4;
    /* Extend with actual payload in a real implementation */
    return 0;
}

int eia_deserialize(EIA *eia, const uint8_t *buf, size_t len) {
    if (!eia || !buf || len < 4) return -1;
    if (buf[0] != EIA_MAGIC) return -1;
    eia_init(eia);
    eia->type_flags = buf[1];
    eia->gas_budget = buf[2];
    eia->total_len = buf[3];
    /* Extend with actual payload in a real implementation */
    return 0;
}
