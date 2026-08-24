/*
 * Airbot — Executable Information System
 * visibility.h — Visibility & Observability Model
 *
 * Mathematical formalization of the Airbot visibility problem:
 *
 * Core equation:
 *   Visibility(A) = f(I_A, T_A, B_A, M_A)
 *
 * Where:
 *   I_A = address information (0 for EIA-only units)
 *   T_A = traffic characteristics
 *   B_A = observable behavior
 *   M_A = communication mechanism fingerprint
 *
 * Key property: Identity(Airbot) ≠ Location(Airbot)
 *
 * An EIA-addressed unit has NO inherent IP → no network-layer
 * location binding. This produces address-layer untraceability.
 */

#ifndef AIRBOT_VISIBILITY_H
#define AIRBOT_VISIBILITY_H

#include <stdint.h>
#include "eiu.h"
#include "eia.h"

/* ─── Observable Channels ───────────────────────────────────── */

/*
 * Each channel contributes independently to observability.
 * An observer monitoring channel C detects an Airbot with
 * probability P_detect(C).
 *
 * Total observability:
 *   O(A,N) = 1 - Π(1 - P_detect(C_i))
 *
 * This is the complement of escaping ALL channels simultaneously.
 */

#define VIS_CHANNEL_ADDRESS     0   /* IP/MAC address information */
#define VIS_CHANNEL_TRAFFIC     1   /* Traffic volume & patterns */
#define VIS_CHANNEL_TIMING      2   /* Timing signatures */
#define VIS_CHANNEL_SIZE        3   /* Packet size distribution */
#define VIS_CHANNEL_BEHAVIOR    4   /* Protocol/behavioral fingerprint */
#define VIS_CHANNEL_ROUTE       5   /* Routing path analysis */
#define VIS_CHANNEL_CONTENT     6   /* Payload content inspection */
#define VIS_CHANNEL_COUNT       7

/* ─── Visibility Analysis ───────────────────────────────────── */

typedef struct {
    /* Per-channel detection probabilities: P_detect(C_i) ∈ [0.0, 1.0] */
    double channel_prob[VIS_CHANNEL_COUNT];

    /* Combined observability: O(A,N) = 1 - Π(1 - P_i) */
    double observability;

    /* Invisibility: P(undetected) = 1 - O(A,N) = Π(1 - P_i) */
    double invisibility;

    /* Address information content: |I_A| in bits */
    uint32_t address_info_bits;

    /* Identity-location coupling: κ ∈ [0.0, 1.0]
     * κ = 0: identity completely decoupled from location (Airbot/EIA)
     * κ = 1: identity = location (conventional IP) */
    double identity_location_coupling;

    /* Address efficiency: η = identity_capability_bits / total_address_bits */
    double address_efficiency;

    /* Conventional comparison: how visible would equivalent IP traffic be */
    double conventional_observability;

    /* Advantage: reduction in observability vs conventional */
    double stealth_advantage;
} VisibilityAnalysis;

/* ─── Network Model ─────────────────────────────────────────── */

/*
 * Models a network observer's capabilities.
 * Different observers can monitor different channels.
 */
typedef struct {
    uint8_t  monitors_address;   /* 1 if observer sees IP/MAC */
    uint8_t  monitors_traffic;   /* 1 if observer sees traffic volume */
    uint8_t  monitors_timing;    /* 1 if observer sees timing */
    uint8_t  monitors_size;      /* 1 if observer sees packet sizes */
    uint8_t  monitors_behavior;  /* 1 if observer does DPI/behavioral */
    uint8_t  monitors_route;     /* 1 if observer sees routing paths */
    uint8_t  monitors_content;   /* 1 if observer does deep inspection */

    /* Detection accuracy per channel (0.0 to 1.0) */
    double   accuracy[VIS_CHANNEL_COUNT];

    /* Background traffic noise level (higher = harder to detect) */
    double   noise_level;        /* 0.0 = silent network, 1.0 = saturated */
} NetworkObserver;

/* ─── Airbot Communication Profile ──────────────────────────── */

/*
 * Describes how an Airbot communicates, affecting its visibility.
 */
typedef struct {
    /* Address layer */
    uint8_t  has_ip_address;     /* 0 = no IP (EIA-only), 1 = has IP */
    uint8_t  has_mac_address;    /* 0 = no MAC (tunneled), 1 = has MAC */
    uint32_t address_bits;       /* Total address bits used */
    uint32_t identity_bits;      /* Identity/capability bits in address */

    /* Traffic characteristics */
    double   traffic_rate;       /* Messages per second */
    double   traffic_variance;   /* Variance in rate (higher = more detectable) */

    /* Timing */
    double   timing_regularity;  /* 0.0 = random, 1.0 = perfectly periodic */

    /* Packet sizes */
    uint16_t min_packet_size;
    uint16_t max_packet_size;
    double   size_variance;      /* Low variance = more fingerprint-able */

    /* Behavioral characteristics */
    uint8_t  uses_standard_protocol; /* 1 if mimics HTTP/DNS/etc */
    double   behavioral_entropy;     /* Higher = less predictable */

    /* Content protection */
    uint8_t  encrypted;          /* 1 if payload is encrypted */
    double   content_entropy;    /* Higher = harder to inspect */
} CommProfile;

/* ─── Functions ─────────────────────────────────────────────── */

/* Initialize a network observer with default capabilities */
void vis_observer_init(NetworkObserver *obs);

/* Create an ISP-level observer (sees addresses, traffic, routes) */
void vis_observer_isp(NetworkObserver *obs);

/* Create a DPI observer (deep packet inspection - sees everything) */
void vis_observer_dpi(NetworkObserver *obs);

/* Create a passive observer (only sees traffic patterns) */
void vis_observer_passive(NetworkObserver *obs);

/* Initialize communication profile for an EIA-only Airbot */
void vis_profile_airbot(CommProfile *prof, const EIU *eiu, const EIA *eia);

/* Initialize communication profile for conventional IP traffic */
void vis_profile_conventional(CommProfile *prof);

/*
 * Core analysis function.
 *
 * Computes:
 *   1. Per-channel detection probabilities
 *   2. Combined observability: O(A,N) = 1 - Π(1 - P_i)
 *   3. Identity-location coupling: κ
 *   4. Address efficiency: η
 *   5. Stealth advantage vs conventional
 *
 * Mathematical model:
 *
 *   P_address  = has_ip * accuracy_address * (1 - noise)
 *   P_traffic  = (traffic_rate / max_rate) * accuracy_traffic * (1 - noise)
 *   P_timing   = timing_regularity * accuracy_timing
 *   P_size     = (1 - size_variance) * accuracy_size
 *   P_behavior = (1 - behavioral_entropy) * accuracy_behavior * (!uses_std_proto)
 *   P_route    = accuracy_route * has_ip * (1 - noise)
 *   P_content  = (1 - encrypted) * (1 - content_entropy) * accuracy_content
 *
 *   O(A,N) = 1 - Π_{i}(1 - P_i)
 *
 *   κ = has_ip ? 1.0 : 0.0  (simplified; gradients possible)
 *
 *   η = identity_bits / address_bits  (0 for IP, high for EIA)
 */
int vis_analyze(const CommProfile *prof, const NetworkObserver *obs,
                VisibilityAnalysis *result);

/* Compare Airbot visibility vs conventional IP visibility */
int vis_compare(const EIU *eiu, const EIA *eia,
                const NetworkObserver *obs,
                VisibilityAnalysis *airbot_result,
                VisibilityAnalysis *conventional_result);

/* Print visibility analysis to stdout */
void vis_print_analysis(const VisibilityAnalysis *va, const char *label);

/* Print comparison report */
void vis_print_comparison(const VisibilityAnalysis *airbot,
                          const VisibilityAnalysis *conventional);

/*
 * Compute the fundamental invisibility equation:
 *
 *   P(invisible) = Π_{i=0}^{N-1} (1 - P_detect(channel_i))
 *
 * For I_A = 0 (no IP address):
 *   P_address = 0  →  factor (1 - 0) = 1.0  (no contribution)
 *   P_route   ≈ 0  →  factor ≈ 1.0
 *
 * Therefore removing the address channel strictly increases invisibility.
 *
 * The UPPER BOUND on invisibility is:
 *   P(invisible) ≤ 1.0  (only if ALL channels have P_detect = 0)
 *
 * The LOWER BOUND is:
 *   P(invisible) ≥ 0.0  (if ANY channel has P_detect = 1.0)
 *
 * Key theorem:
 *   I_A = 0 ⇏ O(A,N) = 0
 *   (No IP does NOT imply total invisibility)
 *
 *   BUT:
 *   I_A = 0 ∧ encrypted ∧ high_entropy ∧ mimics_standard
 *     ⇒ O(A,N) → ε  (approaches but never reaches 0)
 *
 * This is the rigorous form of the visibility equation.
 */
double vis_invisibility_bound(const CommProfile *prof,
                               const NetworkObserver *obs);

/* Address efficiency ratio: η = identity_cap_bits / total_address_bits
 * IP:  η = 0/32 = 0.0 (IP carries NO identity info)
 * EIA: η = (256+64) / (256+64+bytecode) ≈ 0.8+ (most bits are identity/cap) */
double vis_address_efficiency(const CommProfile *prof);

/* Identity-location coupling coefficient κ ∈ [0, 1]
 * κ = 1.0 for conventional IP (identity ≈ location)
 * κ = 0.0 for EIA (identity ≠ location) */
double vis_identity_location_coupling(const CommProfile *prof);

#endif /* AIRBOT_VISIBILITY_H */
