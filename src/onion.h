#ifndef AIRBOT_ONION_H
#define AIRBOT_ONION_H

#include <stdint.h>
#include "bitstream.h"
#include "blake3.h"
#include "chacha20.h"

#ifndef CHACHA20_POLY1305_TAG_SIZE
#define CHACHA20_POLY1305_TAG_SIZE 16
#endif

// A relay node in the circuit
typedef struct {
    uint8_t  node_id[4];        // Relay identifier
    uint8_t  secret_key[32];    // Relay's symmetric key (BLAKE3-derived)
    uint8_t  capability[32];    // Capability token for this hop
} OnionRelay;

// Replay detection filter (small hash set of seen nonces)
#define REPLAY_FILTER_SIZE 256
typedef struct {
    uint8_t  entries[REPLAY_FILTER_SIZE][12];  // Stored 12-byte nonces
    uint8_t  occupied[REPLAY_FILTER_SIZE];      // 1 if slot occupied
    uint32_t count;
} ReplayFilter;

// A circuit (ordered list of relays)
typedef struct {
    OnionRelay relays[8];       // Up to 8 hops
    uint8_t    num_relays;      // Actual relay count (typically 3)
    uint8_t    nonce[12];       // Circuit nonce for replay protection
    uint32_t   circuit_id;      // Circuit identifier
    ReplayFilter replay_filter; // Replay detection filter
} OnionCircuit;

// An onion-wrapped packet
typedef struct {
    uint32_t circuit_id;
    uint8_t  nonce[12];
    uint8_t  num_layers;        // Total layers in circuit
    uint8_t  current_hop;       // Next hop to unwrap (0 = outermost)
    uint8_t  payload[4096];     // Encrypted onion payload
    uint16_t payload_len;
    uint8_t  tags[8][16];       // Per-layer authentication tags
} OnionPacket;

// Result of unwrapping one layer at a relay
typedef struct {
    uint8_t  next_hop[4];       // Where to forward
    uint8_t  capability[32];    // Capability for this hop
    uint8_t  inner_payload[4096]; // Remaining encrypted layers
    uint16_t inner_len;
    uint8_t  is_final;          // 1 if this relay is the destination
    uint8_t  valid;             // 1 if layer was valid
} OnionUnwrapResult;

// Privacy metrics after onion routing
typedef struct {
    uint8_t  num_hops;          // Circuit length
    double   entry_exit_separation;  // P(entry knows exit) — should be near 0
    double   relay_knowledge;        // What each relay knows (should be minimal)
    double   replay_protected;       // 1.0 if nonce-based replay protection works
    double   estimated_H;            // Estimated anonymity set from circuit topology
    double   estimated_U;            // Estimated unlinkability from layered encryption
} OnionPrivacyMetrics;

// Initialize a circuit with N relays
void onion_circuit_init(OnionCircuit *circuit, uint8_t num_relays);

// Generate relay keys (deterministic from seed for reproducibility)
void onion_generate_relays(OnionCircuit *circuit, const uint8_t *seed, uint8_t seed_len);

// Wrap an EIU in onion layers (encrypt from innermost to outermost)
int onion_wrap(const OnionCircuit *circuit, const uint8_t *plaintext, uint16_t plain_len, OnionPacket *packet);

// Unwrap one layer at a relay (decrypt outermost layer, extract next-hop info)
int onion_unwrap(const OnionRelay *relay, const OnionPacket *packet, OnionUnwrapResult *result);

// Verify capability at a relay
int onion_verify_capability(const OnionRelay *relay, const uint8_t *capability);

void replay_filter_init(ReplayFilter *rf);
int replay_filter_check(ReplayFilter *rf, const uint8_t nonce[12]);

// Check replay protection (nonce freshness)
int onion_check_replay(ReplayFilter *rf, const uint8_t *nonce);

// Compute privacy metrics for a circuit
void onion_compute_metrics(const OnionCircuit *circuit, OnionPrivacyMetrics *metrics);

// Print circuit info
void onion_print_circuit(const OnionCircuit *circuit);

// Print privacy metrics
void onion_print_metrics(const OnionPrivacyMetrics *metrics);

// Run a full onion routing simulation:
// Wrap an EIU, pass through all relays, unwrap at destination, verify integrity
int onion_simulate(const OnionCircuit *circuit, const uint8_t *eiu_data, uint16_t eiu_len);

#endif // AIRBOT_ONION_H
