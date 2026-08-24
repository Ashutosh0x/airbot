/*
 * Airbot — Executable Information System
 * privacy.h — Formal Privacy Model
 *
 * Replaces the single observability number with a 5-component
 * privacy vector and enables multi-protocol comparison.
 *
 * Privacy Vector:
 *   P_A = (H, U, L, T, C)
 *
 * Where:
 *   H = anonymity-set uncertainty (entropy of observer's belief)
 *   U = unlinkability (entry/exit observation decorrelation)
 *   L = identity/location separation
 *   T = traffic-analysis resistance
 *   C = content/behavior confidentiality
 *
 * Target: H → H_max, U → 1, L → 1, T → 1, C → 1
 *
 * NOT: O = 0 (which is impossible for any real system)
 *
 * Multi-protocol comparison:
 *   IPv4 vs NDN vs Tor vs Airbot
 *   under the same defined observer model.
 */

#ifndef AIRBOT_PRIVACY_H
#define AIRBOT_PRIVACY_H

#include <stdint.h>
#include "visibility.h"

/* ─── Privacy Vector ────────────────────────────────────────── */

/*
 * The privacy vector P_A = (H, U, L, T, C) ∈ [0,1]^5
 *
 * Each component measures a distinct privacy property:
 *
 *   H: Anonymity-set uncertainty
 *      H = log2(|anonymity_set|) / log2(N_total)
 *      0 = uniquely identified, 1 = indistinguishable from all others
 *
 *   U: Unlinkability
 *      U = 1 - P(link entry to exit observation)
 *      0 = trivially linkable, 1 = completely unlinkable
 *
 *   L: Identity/location separation
 *      L = 1 - κ (complement of identity-location coupling)
 *      0 = identity = location, 1 = fully decoupled
 *
 *   T: Traffic-analysis resistance
 *      T = 1 - P(distinguish from background | traffic features)
 *      0 = trivially fingerprinted, 1 = indistinguishable from background
 *
 *   C: Content/behavior confidentiality
 *      C = 1 - P(infer content or behavior | observation)
 *      0 = fully transparent, 1 = completely opaque
 */
typedef struct {
    double H;   /* Anonymity-set uncertainty ∈ [0, 1] */
    double U;   /* Unlinkability ∈ [0, 1] */
    double L;   /* Identity/location separation ∈ [0, 1] */
    double T;   /* Traffic-analysis resistance ∈ [0, 1] */
    double C;   /* Content/behavior confidentiality ∈ [0, 1] */

    /* Aggregate privacy score (geometric mean of components) */
    double aggregate;
} PrivacyVector;

/* ─── Protocol Profiles ────────────────────────────────────── */

/*
 * Models different network architectures for comparison.
 */
typedef enum {
    PROTO_IPV4,     /* Standard IPv4 — baseline */
    PROTO_IPV6,     /* IPv6 with EUI-64 interface IDs */
    PROTO_NDN,      /* Named Data Networking (content-name routing) */
    PROTO_TOR,      /* Tor onion routing (3-hop circuits) */
    PROTO_AIRBOT,   /* Airbot EIA-based (no IP, programmable routing) */
    PROTO_COUNT
} ProtocolType;

typedef struct {
    ProtocolType type;
    const char  *name;
    const char  *description;

    /* Architectural properties */
    uint8_t  has_native_address;     /* Has IP/MAC in protocol */
    uint8_t  has_content_addressing; /* Routes by content hash */
    uint8_t  has_onion_encryption;   /* Layered multi-hop encryption */
    uint8_t  has_relay_separation;   /* No single relay sees both ends */
    uint8_t  has_executable_routing; /* Routing logic in the object */
    uint8_t  has_capability_auth;    /* Cryptographic capability system */
    uint8_t  has_state_evolution;    /* Stateful objects that evolve */
    uint8_t  has_traffic_shaping;    /* Traffic normalization */

    /* Anonymity properties */
    uint32_t anonymity_set_size;     /* Typical anonymity set */
    double   link_correlation;       /* P(link entry to exit) */
    double   timing_leakage;         /* P(timing correlation succeeds) */

    /* Address properties */
    uint32_t address_bits;
    uint32_t identity_bits;
    uint8_t  address_contains_location;

    /* Communication properties */
    uint8_t  encrypted;
    double   behavioral_entropy;
    double   traffic_regularity;
    double   size_variance;
} ProtocolProfile;

/* ─── Privacy Hierarchy ─────────────────────────────────────── */

/*
 * The privacy hierarchy (from weakest to strongest):
 *
 *   1. Identity-location separation
 *   2. Unlinkability
 *   3. Traffic-analysis resistance
 *   4. Metadata indistinguishability
 *   5. Formal privacy bound
 *
 * Each level subsumes the previous:
 *   Level k achieved ⟹ all levels < k also achieved.
 */
typedef struct {
    uint8_t  level_achieved;         /* Highest level achieved (1-5) */
    double   level_scores[5];        /* Score per level ∈ [0, 1] */
    const char *level_names[5];      /* Human-readable level names */
} PrivacyHierarchy;

/* ─── Distinguishability Test ───────────────────────────────── */

/*
 * Formal distinguishability:
 *
 *   Given observable metadata M = (Volume, Size, Timing, Route, Metadata):
 *   Test whether P(M|A) ≈ P(M|¬A)
 *
 *   If the distributions are close, the protocol is indistinguishable
 *   from background traffic under observation.
 *
 *   Measured via statistical distance:
 *     Δ = |P(M|A) - P(M|¬A)|
 *     Δ = 0: perfectly indistinguishable
 *     Δ = 1: trivially distinguishable
 */
typedef struct {
    double volume_distance;   /* |P(volume|A) - P(volume|¬A)| */
    double size_distance;     /* |P(size|A) - P(size|¬A)| */
    double timing_distance;   /* |P(timing|A) - P(timing|¬A)| */
    double route_distance;    /* |P(route|A) - P(route|¬A)| */
    double metadata_distance; /* |P(metadata|A) - P(metadata|¬A)| */

    /* Overall statistical distance */
    double total_distance;    /* max or mean of component distances */

    /* Privacy loss bound (differential privacy ε) */
    double epsilon;           /* ln(max P(M|A) / P(M|¬A)) */
} DistinguishabilityTest;

/* ─── Multi-Protocol Comparison ─────────────────────────────── */

typedef struct {
    ProtocolProfile     profiles[PROTO_COUNT];
    PrivacyVector       vectors[PROTO_COUNT];
    PrivacyHierarchy    hierarchies[PROTO_COUNT];
    DistinguishabilityTest distinguishability[PROTO_COUNT];
    VisibilityAnalysis  visibility[PROTO_COUNT];
} ProtocolComparison;

/* ─── Functions ─────────────────────────────────────────────── */

/* Initialize protocol profiles with defined parameters */
void priv_init_profiles(ProtocolComparison *cmp);

/* Compute privacy vector for a given protocol under an observer */
void priv_compute_vector(const ProtocolProfile *prof,
                         const NetworkObserver *obs,
                         PrivacyVector *vec);

/* Compute privacy hierarchy level for a protocol */
void priv_compute_hierarchy(const PrivacyVector *vec,
                            PrivacyHierarchy *hier);

/* Compute distinguishability test for a protocol */
void priv_compute_distinguishability(const ProtocolProfile *prof,
                                     const NetworkObserver *obs,
                                     DistinguishabilityTest *dist);

/* Run full multi-protocol comparison under a given observer */
void priv_compare_all(ProtocolComparison *cmp,
                      const NetworkObserver *obs);

/* Print privacy vector */
void priv_print_vector(const PrivacyVector *vec, const char *name);

/* Print full multi-protocol comparison table */
void priv_print_comparison(const ProtocolComparison *cmp);

/* Print privacy hierarchy */
void priv_print_hierarchy(const PrivacyHierarchy *hier, const char *name);

/* Print distinguishability test results */
void priv_print_distinguishability(const DistinguishabilityTest *dist,
                                    const char *name);

#endif /* AIRBOT_PRIVACY_H */
