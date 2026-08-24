/*
 * Airbot — Executable Information System
 * visibility.c — Visibility & Observability Model Implementation
 *
 * Implements the mathematical framework:
 *
 *   O(A,N) = 1 - Π_{i}(1 - P_detect(C_i))
 *
 *   Visibility(A) = f(I_A, T_A, B_A, M_A)
 *
 *   Identity(Airbot) ≠ Location(Airbot)
 *
 *   η_address = identity_capability_bits / total_address_bits
 *
 *   κ = identity_location_coupling ∈ [0, 1]
 *
 * Pure C99. No external dependencies.
 */

#include "visibility.h"
#include "bitstream.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

/* ═══════════════════════════════════════════════════════════════
 * Helper: Clamp a value to [0.0, 1.0]
 * ═══════════════════════════════════════════════════════════════ */

static double clamp01(double x) {
    if (x < 0.0) return 0.0;
    if (x > 1.0) return 1.0;
    return x;
}

/* ═══════════════════════════════════════════════════════════════
 * Observer Initialization
 * ═══════════════════════════════════════════════════════════════ */

void vis_observer_init(NetworkObserver *obs) {
    memset(obs, 0, sizeof(*obs));
    obs->noise_level = 0.5; /* Moderate background traffic */
    for (int i = 0; i < VIS_CHANNEL_COUNT; i++) {
        obs->accuracy[i] = 0.5; /* 50% base detection accuracy */
    }
}

void vis_observer_isp(NetworkObserver *obs) {
    vis_observer_init(obs);
    obs->monitors_address  = 1;
    obs->monitors_traffic  = 1;
    obs->monitors_timing   = 1;
    obs->monitors_size     = 1;
    obs->monitors_behavior = 0; /* ISP typically doesn't do DPI */
    obs->monitors_route    = 1;
    obs->monitors_content  = 0;

    obs->accuracy[VIS_CHANNEL_ADDRESS]  = 0.95;
    obs->accuracy[VIS_CHANNEL_TRAFFIC]  = 0.80;
    obs->accuracy[VIS_CHANNEL_TIMING]   = 0.60;
    obs->accuracy[VIS_CHANNEL_SIZE]     = 0.70;
    obs->accuracy[VIS_CHANNEL_ROUTE]    = 0.85;
    obs->noise_level = 0.6;
}

void vis_observer_dpi(NetworkObserver *obs) {
    vis_observer_init(obs);
    obs->monitors_address  = 1;
    obs->monitors_traffic  = 1;
    obs->monitors_timing   = 1;
    obs->monitors_size     = 1;
    obs->monitors_behavior = 1;
    obs->monitors_route    = 1;
    obs->monitors_content  = 1;

    obs->accuracy[VIS_CHANNEL_ADDRESS]  = 0.99;
    obs->accuracy[VIS_CHANNEL_TRAFFIC]  = 0.90;
    obs->accuracy[VIS_CHANNEL_TIMING]   = 0.85;
    obs->accuracy[VIS_CHANNEL_SIZE]     = 0.80;
    obs->accuracy[VIS_CHANNEL_BEHAVIOR] = 0.75;
    obs->accuracy[VIS_CHANNEL_ROUTE]    = 0.90;
    obs->accuracy[VIS_CHANNEL_CONTENT]  = 0.70;
    obs->noise_level = 0.4;
}

void vis_observer_passive(NetworkObserver *obs) {
    vis_observer_init(obs);
    obs->monitors_address  = 0;
    obs->monitors_traffic  = 1;
    obs->monitors_timing   = 1;
    obs->monitors_size     = 1;
    obs->monitors_behavior = 0;
    obs->monitors_route    = 0;
    obs->monitors_content  = 0;

    obs->accuracy[VIS_CHANNEL_TRAFFIC]  = 0.60;
    obs->accuracy[VIS_CHANNEL_TIMING]   = 0.50;
    obs->accuracy[VIS_CHANNEL_SIZE]     = 0.40;
    obs->noise_level = 0.7;
}

/* ═══════════════════════════════════════════════════════════════
 * Communication Profile Initialization
 * ═══════════════════════════════════════════════════════════════ */

void vis_profile_airbot(CommProfile *prof, const EIU *eiu, const EIA *eia) {
    memset(prof, 0, sizeof(*prof));

    /* EIA-only: NO IP address, NO MAC address */
    prof->has_ip_address  = 0;  /* I_A = 0 */
    prof->has_mac_address = 0;

    /* Address information: all identity/capability, no location */
    if (eia) {
        prof->address_bits  = 8 + 8 + 8 + 8;  /* Header: 4 bytes */
        if (eia->type_flags & EIA_TYPE_HAS_DIGEST) {
            prof->address_bits += 256;           /* 32-byte digest */
            prof->identity_bits += 256;
        }
        if (eia->type_flags & EIA_TYPE_HAS_CAPABILITY) {
            prof->address_bits += 64;            /* 8-byte capability */
            prof->identity_bits += 64;
        }
        if (eia->bytecode_len > 0) {
            prof->address_bits += (uint32_t)(eia->bytecode_len * 8);
        }
    }

    /* Traffic: low rate, high variance (bursty, hard to fingerprint) */
    prof->traffic_rate     = 0.1;   /* Low: 0.1 msg/sec */
    prof->traffic_variance = 0.9;   /* High variance = harder to detect */

    /* Timing: randomized (not periodic) */
    prof->timing_regularity = 0.1;  /* Low regularity = hard to fingerprint */

    /* Packet sizes: variable */
    prof->min_packet_size = 16;
    prof->max_packet_size = 512;
    prof->size_variance   = 0.8;    /* High variance */

    /* Behavior: mimics standard protocols */
    prof->uses_standard_protocol = 1;  /* Camouflaged */
    prof->behavioral_entropy     = 0.9; /* High entropy = unpredictable */

    /* Content: encrypted */
    prof->encrypted      = 1;
    prof->content_entropy = 0.99; /* Near-maximum entropy (encrypted) */

    (void)eiu;
}

void vis_profile_conventional(CommProfile *prof) {
    memset(prof, 0, sizeof(*prof));

    /* Conventional: has IP and MAC */
    prof->has_ip_address  = 1;
    prof->has_mac_address = 1;

    /* IPv4: 32-bit address carries 0 identity/capability bits */
    prof->address_bits  = 32;   /* IPv4 address */
    prof->identity_bits = 0;    /* IP reveals location, NOT identity */

    /* Traffic: regular patterns */
    prof->traffic_rate     = 1.0;
    prof->traffic_variance = 0.3;

    /* Timing: somewhat regular */
    prof->timing_regularity = 0.6;

    /* Packet sizes: standard MTU */
    prof->min_packet_size = 64;
    prof->max_packet_size = 1500;
    prof->size_variance   = 0.4;

    /* Behavior: standard protocol */
    prof->uses_standard_protocol = 1;
    prof->behavioral_entropy     = 0.5;

    /* Content: often unencrypted or partially encrypted */
    prof->encrypted      = 0;
    prof->content_entropy = 0.4;
}

/* ═══════════════════════════════════════════════════════════════
 * Core Visibility Analysis
 *
 * Mathematical model:
 *
 * For each observable channel C_i, the detection probability is:
 *
 *   P_i = enabled_i × signal_i × accuracy_i × (1 - noise)
 *
 * where signal_i depends on the Airbot's communication profile.
 *
 * Combined observability (independent channels):
 *
 *   O(A,N) = 1 - Π_{i=0}^{N-1} (1 - P_i)
 *
 * This is the probability that the Airbot is detected on
 * AT LEAST ONE channel.
 *
 * Invisibility (complementary):
 *
 *   P(invisible) = Π_{i=0}^{N-1} (1 - P_i)
 *
 * Key result for I_A = 0 (no IP):
 *   P_address = 0, P_route ≈ 0
 *   → Two channel factors become 1.0 (contribute nothing to detection)
 *   → Overall invisibility strictly increases
 *
 * ═══════════════════════════════════════════════════════════════ */

int vis_analyze(const CommProfile *prof, const NetworkObserver *obs,
                VisibilityAnalysis *result) {
    if (!prof || !obs || !result) return AIRBOT_ERR_NULLPTR;
    memset(result, 0, sizeof(*result));

    double noise_complement = 1.0 - obs->noise_level;

    /* ── Channel 0: Address (IP/MAC) ─────────────────────────
     *
     * P_address = has_ip × accuracy × (1 - noise)
     *
     * KEY: If I_A = 0 (no IP), then P_address = 0
     * This ELIMINATES the entire address detection channel.
     */
    if (obs->monitors_address) {
        double signal = (double)prof->has_ip_address;
        result->channel_prob[VIS_CHANNEL_ADDRESS] =
            clamp01(signal * obs->accuracy[VIS_CHANNEL_ADDRESS] * noise_complement);
    }

    /* ── Channel 1: Traffic patterns ─────────────────────────
     *
     * P_traffic = (rate / max_rate) × (1 - variance) × accuracy × (1 - noise)
     *
     * Low rate + high variance = hard to detect
     */
    if (obs->monitors_traffic) {
        double signal = prof->traffic_rate * (1.0 - prof->traffic_variance);
        result->channel_prob[VIS_CHANNEL_TRAFFIC] =
            clamp01(signal * obs->accuracy[VIS_CHANNEL_TRAFFIC] * noise_complement);
    }

    /* ── Channel 2: Timing analysis ──────────────────────────
     *
     * P_timing = regularity × accuracy × (1 - noise)
     *
     * Random timing (regularity→0) = undetectable timing signature
     */
    if (obs->monitors_timing) {
        double signal = prof->timing_regularity;
        result->channel_prob[VIS_CHANNEL_TIMING] =
            clamp01(signal * obs->accuracy[VIS_CHANNEL_TIMING] * noise_complement);
    }

    /* ── Channel 3: Packet size distribution ─────────────────
     *
     * P_size = (1 - size_variance) × accuracy
     *
     * High size variance = looks like normal traffic
     */
    if (obs->monitors_size) {
        double signal = 1.0 - prof->size_variance;
        result->channel_prob[VIS_CHANNEL_SIZE] =
            clamp01(signal * obs->accuracy[VIS_CHANNEL_SIZE]);
    }

    /* ── Channel 4: Behavioral fingerprint ───────────────────
     *
     * P_behavior = (1 - behavioral_entropy) × (!uses_std_proto) × accuracy
     *
     * High entropy + standard protocol mimicry = undetectable behavior
     */
    if (obs->monitors_behavior) {
        double proto_factor = prof->uses_standard_protocol ? 0.2 : 1.0;
        double signal = (1.0 - prof->behavioral_entropy) * proto_factor;
        result->channel_prob[VIS_CHANNEL_BEHAVIOR] =
            clamp01(signal * obs->accuracy[VIS_CHANNEL_BEHAVIOR]);
    }

    /* ── Channel 5: Route analysis ───────────────────────────
     *
     * P_route = has_ip × accuracy × (1 - noise)
     *
     * KEY: No IP → no routable address → no traceable route
     * This is the second channel eliminated by I_A = 0
     */
    if (obs->monitors_route) {
        double signal = (double)prof->has_ip_address;
        result->channel_prob[VIS_CHANNEL_ROUTE] =
            clamp01(signal * obs->accuracy[VIS_CHANNEL_ROUTE] * noise_complement);
    }

    /* ── Channel 6: Content inspection (DPI) ─────────────────
     *
     * P_content = (1 - encrypted) × (1 - content_entropy) × accuracy
     *
     * Encrypted + high entropy = completely opaque to DPI
     */
    if (obs->monitors_content) {
        double signal = (1.0 - (double)prof->encrypted) *
                        (1.0 - prof->content_entropy);
        result->channel_prob[VIS_CHANNEL_CONTENT] =
            clamp01(signal * obs->accuracy[VIS_CHANNEL_CONTENT]);
    }

    /* ═══════════════════════════════════════════════════════════
     * Combined Observability Equation
     *
     *   O(A,N) = 1 - Π_{i=0}^{6} (1 - P_i)
     *
     * Each factor (1 - P_i) is the probability of escaping
     * channel i. The product is escaping ALL channels.
     * ═══════════════════════════════════════════════════════════ */

    double product = 1.0;
    for (int i = 0; i < VIS_CHANNEL_COUNT; i++) {
        product *= (1.0 - result->channel_prob[i]);
    }

    result->observability = 1.0 - product;
    result->invisibility  = product;

    /* ═══════════════════════════════════════════════════════════
     * Address Information Content
     * ═══════════════════════════════════════════════════════════ */

    result->address_info_bits = prof->address_bits;

    /* ═══════════════════════════════════════════════════════════
     * Identity-Location Coupling: κ ∈ [0, 1]
     *
     * For conventional IP:
     *   IP address ≈ network location → κ = 1.0
     *
     * For EIA-only Airbot:
     *   EIA = H(content) + capabilities → κ = 0.0
     *   Identity is content-based, NOT location-based
     *
     * General formula:
     *   κ = P(locate | identify)
     *   κ = has_ip × (1 - identity_bits/address_bits)
     *
     * If identity_bits dominate address_bits, location is
     * a negligible fraction → κ → 0
     * ═══════════════════════════════════════════════════════════ */

    result->identity_location_coupling =
        vis_identity_location_coupling(prof);

    /* ═══════════════════════════════════════════════════════════
     * Address Efficiency: η = identity_cap_bits / address_bits
     *
     * IP:  η = 0/32 = 0.0  (IP carries ZERO identity info)
     * EIA: η = (256+64)/(256+64+bc) ≈ 0.8+  (mostly identity/cap)
     * ═══════════════════════════════════════════════════════════ */

    result->address_efficiency = vis_address_efficiency(prof);

    return AIRBOT_OK;
}

/* ═══════════════════════════════════════════════════════════════
 * Comparison: Airbot vs Conventional
 * ═══════════════════════════════════════════════════════════════ */

int vis_compare(const EIU *eiu, const EIA *eia,
                const NetworkObserver *obs,
                VisibilityAnalysis *airbot_result,
                VisibilityAnalysis *conventional_result) {

    CommProfile airbot_prof, conv_prof;

    vis_profile_airbot(&airbot_prof, eiu, eia);
    vis_profile_conventional(&conv_prof);

    vis_analyze(&airbot_prof, obs, airbot_result);
    vis_analyze(&conv_prof, obs, conventional_result);

    /* Compute stealth advantage */
    airbot_result->conventional_observability = conventional_result->observability;
    airbot_result->stealth_advantage =
        conventional_result->observability - airbot_result->observability;

    return AIRBOT_OK;
}

/* ═══════════════════════════════════════════════════════════════
 * Derived Quantities
 * ═══════════════════════════════════════════════════════════════ */

double vis_invisibility_bound(const CommProfile *prof,
                               const NetworkObserver *obs) {
    VisibilityAnalysis va;
    vis_analyze(prof, obs, &va);
    return va.invisibility;
}

double vis_address_efficiency(const CommProfile *prof) {
    if (prof->address_bits == 0) return 0.0;
    return (double)prof->identity_bits / (double)prof->address_bits;
}

double vis_identity_location_coupling(const CommProfile *prof) {
    /*
     * κ = P(locate | identify)
     *
     * If has_ip = 1 and identity_bits = 0:
     *   Address is PURE location → κ = 1.0
     *
     * If has_ip = 0 and identity_bits > 0:
     *   Address is PURE identity → κ = 0.0
     *
     * Mixed: κ = has_ip × (1 - identity_fraction)
     */
    if (!prof->has_ip_address) return 0.0;
    if (prof->address_bits == 0) return 0.0;

    double identity_fraction = (double)prof->identity_bits /
                               (double)prof->address_bits;
    return clamp01((double)prof->has_ip_address * (1.0 - identity_fraction));
}

/* ═══════════════════════════════════════════════════════════════
 * Printing
 * ═══════════════════════════════════════════════════════════════ */

static const char *channel_names[VIS_CHANNEL_COUNT] = {
    "Address (IP/MAC)",
    "Traffic Pattern ",
    "Timing Analysis ",
    "Packet Sizes    ",
    "Behavior/DPI    ",
    "Route Analysis  ",
    "Content Inspect "
};

void vis_print_analysis(const VisibilityAnalysis *va, const char *label) {
    printf("\n  ┌─── Visibility Analysis: %s ───\n", label);
    printf("  │\n");
    printf("  │  Per-channel detection probabilities (modeled):\n");

    for (int i = 0; i < VIS_CHANNEL_COUNT; i++) {
        /* Visual bar */
        int bar_len = (int)(va->channel_prob[i] * 20.0);
        printf("  │    %s  P = %.4f  [", channel_names[i], va->channel_prob[i]);
        for (int j = 0; j < 20; j++) {
            printf("%c", j < bar_len ? '#' : '.');
        }
        printf("]\n");
    }

    printf("  │\n");
    printf("  │  ╔══════════════════════════════════════════════════════════╗\n");
    printf("  │  ║  O(A,N) = 1 - Π(1 - P_i) = %.6f                    ║\n",
           va->observability);
    printf("  │  ║  Est. non-detection P     = %.6f                    ║\n",
           va->invisibility);
    printf("  │  ║  Est. non-detection       = %.2f%% (under this model) ║\n",
           va->invisibility * 100.0);
    printf("  │  ╚══════════════════════════════════════════════════════════╝\n");
    printf("  │\n");
    printf("  │  Address info:    %u bits\n", va->address_info_bits);
    printf("  │  Address η:       %.4f (identity+cap / total address bits)\n",
           va->address_efficiency);
    printf("  │  κ (id≠loc):      %.4f (0=decoupled, 1=coupled)\n",
           va->identity_location_coupling);
    printf("  │\n");
    printf("  │  NOTE: Probabilities are computed from the defined\n");
    printf("  │  signal model. See methodology for parameter derivation.\n");
    printf("  └────────────────────────────────────────────\n");
}

void vis_print_comparison(const VisibilityAnalysis *airbot,
                          const VisibilityAnalysis *conventional) {
    printf("\n");
    printf("  ╔══════════════════════════════════════════════════════════╗\n");
    printf("  ║        AIRBOT vs CONVENTIONAL VISIBILITY ANALYSIS       ║\n");
    printf("  ╚══════════════════════════════════════════════════════════╝\n");

    vis_print_analysis(airbot, "AIRBOT (EIA-only, I_A = 0)");
    vis_print_analysis(conventional, "CONVENTIONAL (IPv4)");

    printf("\n  ═══ COMPARISON ═══════════════════════════════════════════\n");
    printf("  │\n");
    printf("  │  Metric                 │ Airbot      │ Conventional\n");
    printf("  │  ───────────────────────┼─────────────┼─────────────\n");
    printf("  │  Observability O(A,N)   │ %10.6f  │ %10.6f\n",
           airbot->observability, conventional->observability);
    printf("  │  Invisibility %%         │ %9.2f%%  │ %9.2f%%\n",
           airbot->invisibility * 100.0, conventional->invisibility * 100.0);
    printf("  │  Address efficiency η   │ %10.4f  │ %10.4f\n",
           airbot->address_efficiency, conventional->address_efficiency);
    printf("  │  Identity-Location κ    │ %10.4f  │ %10.4f\n",
           airbot->identity_location_coupling,
           conventional->identity_location_coupling);
    printf("  │\n");

    double advantage = conventional->observability - airbot->observability;
    double factor = (conventional->observability > 0.001)
                    ? airbot->invisibility / conventional->invisibility
                    : 1.0;

    printf("  │  ┌──────────────────────────────────────────────────┐\n");
    printf("  │  │ Stealth Advantage:  %.4f (%.1f%% less visible) │\n",
           advantage, advantage * 100.0);
    printf("  │  │ Invisibility Ratio: %.2f× more invisible       │\n",
           factor);
    printf("  │  │                                                  │\n");
    if (airbot->identity_location_coupling < 0.01) {
        printf("  │  │ Identity ≠ Location: FULLY DECOUPLED (κ ≈ 0)   │\n");
    }
    printf("  │  │                                                  │\n");
    printf("  │  │ KEY: I_A = 0 eliminates address & route channels │\n");
    printf("  │  │ Remaining channels require active DPI to detect  │\n");
    printf("  │  └──────────────────────────────────────────────────┘\n");
    printf("  │\n");
    printf("  ═════════════════════════════════════════════════════════\n\n");
}
