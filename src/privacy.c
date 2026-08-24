/*
 * Airbot — Executable Information System
 * privacy.c — Formal Privacy Model Implementation
 *
 * Implements the privacy vector P_A = (H, U, L, T, C),
 * multi-protocol comparison (IPv4/IPv6/NDN/Tor/Airbot),
 * formal distinguishability tests, and privacy hierarchy.
 *
 * All parameters are defined and documented below.
 * Results are modeled estimates, not empirical measurements.
 *
 * Pure C99. No external dependencies.
 */

#include "privacy.h"
#include "bitstream.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

/* ═══════════════════════════════════════════════════════════════
 * Helpers
 * ═══════════════════════════════════════════════════════════════ */

static double clamp01(double x) {
    if (x < 0.0) return 0.0;
    if (x > 1.0) return 1.0;
    return x;
}

/* log2 implementation for TCC compatibility (Windows TCC lacks log2) */
static double priv_log2(double x) {
    if (x <= 0.0) return 0.0;
    return log(x) / log(2.0);
}

static double geometric_mean_5(double a, double b, double c, double d, double e) {
    /* Geometric mean of 5 values — gives 0 if any component is 0 */
    double product = a * b * c * d * e;
    if (product <= 0.0) return 0.0;
    return pow(product, 0.2);
}

/* ═══════════════════════════════════════════════════════════════
 * Protocol Profile Initialization
 *
 * Each protocol's parameters are explicitly defined here.
 * These are the values that produce the comparison results.
 * ═══════════════════════════════════════════════════════════════ */

void priv_init_profiles(ProtocolComparison *cmp) {
    memset(cmp, 0, sizeof(*cmp));

    /* ── IPv4 ────────────────────────────────────────────────── */
    ProtocolProfile *ipv4 = &cmp->profiles[PROTO_IPV4];
    ipv4->type = PROTO_IPV4;
    ipv4->name = "IPv4";
    ipv4->description = "Standard IPv4 — baseline";
    ipv4->has_native_address      = 1;
    ipv4->has_content_addressing  = 0;
    ipv4->has_onion_encryption    = 0;
    ipv4->has_relay_separation    = 0;
    ipv4->has_executable_routing  = 0;
    ipv4->has_capability_auth     = 0;
    ipv4->has_state_evolution     = 0;
    ipv4->has_traffic_shaping     = 0;
    ipv4->anonymity_set_size      = 1;   /* Unique IP = set of 1 */
    ipv4->link_correlation        = 1.0; /* Trivially linkable via IP */
    ipv4->timing_leakage          = 0.8; /* High timing correlation */
    ipv4->address_bits            = 32;
    ipv4->identity_bits           = 0;   /* IP carries 0 identity info */
    ipv4->address_contains_location = 1;
    ipv4->encrypted               = 0;   /* Modeled as unencrypted baseline */
    ipv4->behavioral_entropy      = 0.5;
    ipv4->traffic_regularity      = 0.6;
    ipv4->size_variance           = 0.4;

    /* ── IPv6 ────────────────────────────────────────────────── */
    ProtocolProfile *ipv6 = &cmp->profiles[PROTO_IPV6];
    ipv6->type = PROTO_IPV6;
    ipv6->name = "IPv6";
    ipv6->description = "IPv6 with EUI-64 interface IDs";
    ipv6->has_native_address      = 1;
    ipv6->has_content_addressing  = 0;
    ipv6->has_onion_encryption    = 0;
    ipv6->has_relay_separation    = 0;
    ipv6->has_executable_routing  = 0;
    ipv6->has_capability_auth     = 0;
    ipv6->has_state_evolution     = 0;
    ipv6->has_traffic_shaping     = 0;
    ipv6->anonymity_set_size      = 1;   /* Unique address */
    ipv6->link_correlation        = 0.9; /* Slightly harder due to privacy extensions */
    ipv6->timing_leakage          = 0.8;
    ipv6->address_bits            = 128;
    ipv6->identity_bits           = 0;   /* EUI-64 leaks device, not identity */
    ipv6->address_contains_location = 1;
    ipv6->encrypted               = 0;
    ipv6->behavioral_entropy      = 0.5;
    ipv6->traffic_regularity      = 0.6;
    ipv6->size_variance           = 0.4;

    /* ── NDN ─────────────────────────────────────────────────── */
    ProtocolProfile *ndn = &cmp->profiles[PROTO_NDN];
    ndn->type = PROTO_NDN;
    ndn->name = "NDN";
    ndn->description = "Named Data Networking (content-name routing)";
    ndn->has_native_address      = 0;   /* No IP in native NDN */
    ndn->has_content_addressing  = 1;   /* Routes by content name */
    ndn->has_onion_encryption    = 0;
    ndn->has_relay_separation    = 0;
    ndn->has_executable_routing  = 0;   /* Names are passive, not executable */
    ndn->has_capability_auth     = 0;   /* Content verification, not capability */
    ndn->has_state_evolution     = 0;   /* Passive content, no state */
    ndn->has_traffic_shaping     = 0;
    ndn->anonymity_set_size      = 100; /* In-network caching obscures requester */
    ndn->link_correlation        = 0.5; /* Caching helps but not fully unlinkable */
    ndn->timing_leakage          = 0.6; /* Interest-Data timing correlation */
    ndn->address_bits            = 256; /* Content name hash */
    ndn->identity_bits           = 256; /* Content name IS identity */
    ndn->address_contains_location = 0; /* Content name ≠ location */
    ndn->encrypted               = 0;  /* Content is typically public */
    ndn->behavioral_entropy      = 0.6;
    ndn->traffic_regularity      = 0.5;
    ndn->size_variance           = 0.5;

    /* ── Tor ─────────────────────────────────────────────────── */
    ProtocolProfile *tor = &cmp->profiles[PROTO_TOR];
    tor->type = PROTO_TOR;
    tor->name = "Tor";
    tor->description = "Tor onion routing (3-hop circuits)";
    tor->has_native_address      = 1;   /* Uses IP at transport level */
    tor->has_content_addressing  = 0;
    tor->has_onion_encryption    = 1;   /* Layered AES encryption */
    tor->has_relay_separation    = 1;   /* No relay sees both ends */
    tor->has_executable_routing  = 0;   /* Fixed circuit-based routing */
    tor->has_capability_auth     = 0;
    tor->has_state_evolution     = 0;
    tor->has_traffic_shaping     = 0;   /* Tor does NOT shape traffic */
    tor->anonymity_set_size      = 6000; /* ~6000 relays as of 2024 */
    tor->link_correlation        = 0.1;  /* Low but nonzero (timing attacks) */
    tor->timing_leakage          = 0.4;  /* Vulnerable to end-to-end correlation */
    tor->address_bits            = 32;   /* Underlying IP */
    tor->identity_bits           = 0;
    tor->address_contains_location = 0;  /* Onion address hides location */
    tor->encrypted               = 1;   /* Triple-encrypted */
    tor->behavioral_entropy      = 0.7;
    tor->traffic_regularity      = 0.3; /* Semi-random due to cell padding */
    tor->size_variance           = 0.2; /* Fixed 512-byte cells */

    /* ── Airbot ──────────────────────────────────────────────── */
    ProtocolProfile *airbot = &cmp->profiles[PROTO_AIRBOT];
    airbot->type = PROTO_AIRBOT;
    airbot->name = "Airbot";
    airbot->description = "EIA-based (no IP, programmable routing)";
    airbot->has_native_address      = 0;  /* No IP — EIA only */
    airbot->has_content_addressing  = 1;  /* BLAKE3 content hash */
    airbot->has_onion_encryption    = 0;  /* Not yet implemented */
    airbot->has_relay_separation    = 0;  /* Not yet implemented */
    airbot->has_executable_routing  = 1;  /* EIA contains routing bytecode */
    airbot->has_capability_auth     = 1;  /* Cryptographic capabilities */
    airbot->has_state_evolution     = 1;  /* Stateful objects that evolve */
    airbot->has_traffic_shaping     = 0;  /* Not yet implemented */
    airbot->anonymity_set_size      = 50; /* Depends on deployment */
    airbot->link_correlation        = 0.3; /* Content-addressed, harder to link */
    airbot->timing_leakage          = 0.5; /* No traffic shaping yet */
    airbot->address_bits            = 368; /* 32-byte hash + 8-byte cap + header */
    airbot->identity_bits           = 320; /* 256 hash + 64 capability */
    airbot->address_contains_location = 0; /* EIA = content hash, not location */
    airbot->encrypted               = 1;
    airbot->behavioral_entropy      = 0.9;
    airbot->traffic_regularity      = 0.1; /* Randomized */
    airbot->size_variance           = 0.8;
}

/* ═══════════════════════════════════════════════════════════════
 * Privacy Vector Computation
 *
 * P_A = (H, U, L, T, C)
 *
 * Each component is derived from the protocol profile and
 * observer model. All derivations are documented below.
 * ═══════════════════════════════════════════════════════════════ */

void priv_compute_vector(const ProtocolProfile *prof,
                         const NetworkObserver *obs,
                         PrivacyVector *vec) {
    if (!prof || !obs || !vec) return;
    memset(vec, 0, sizeof(*vec));

    /*
     * H: Anonymity-set uncertainty
     *
     * H = log2(|anonymity_set|) / log2(N_total)
     *
     * We normalize against a reference population of 10,000.
     * H = 0: set size 1 (uniquely identified)
     * H = 1: set size = total population
     *
     * Affected by: observer accuracy on address channel
     */
    {
        double N_total = 10000.0;
        double effective_set = (double)prof->anonymity_set_size;
        /* Observer's address accuracy reduces effective anonymity set */
        if (obs->monitors_address && prof->has_native_address) {
            effective_set *= (1.0 - obs->accuracy[VIS_CHANNEL_ADDRESS] *
                             (1.0 - obs->noise_level));
            if (effective_set < 1.0) effective_set = 1.0;
        }
        vec->H = clamp01(priv_log2(effective_set) / priv_log2(N_total));
    }

    /*
     * U: Unlinkability
     *
     * U = 1 - P(link entry observation to exit observation)
     *
     * Depends on:
     * - Relay separation (onion routing)
     * - Link correlation of the protocol
     * - Observer's route-analysis capability
     */
    {
        double base_linkability = prof->link_correlation;
        if (prof->has_relay_separation) {
            base_linkability *= 0.3; /* Relay separation reduces linkability */
        }
        if (obs->monitors_route) {
            base_linkability *= (1.0 + obs->accuracy[VIS_CHANNEL_ROUTE] * 0.5);
            if (base_linkability > 1.0) base_linkability = 1.0;
        }
        vec->U = clamp01(1.0 - base_linkability);
    }

    /*
     * L: Identity/location separation
     *
     * L = 1 - κ
     *
     * κ = address_contains_location × (1 - identity_bits/address_bits)
     *
     * For protocols where the address IS the location (IPv4): L ≈ 0
     * For protocols where the address is content-based (EIA): L ≈ 1
     */
    {
        double kappa;
        if (!prof->address_contains_location) {
            kappa = 0.0; /* Address has no location component */
        } else if (prof->address_bits == 0) {
            kappa = 0.0;
        } else {
            double id_frac = (double)prof->identity_bits /
                             (double)prof->address_bits;
            kappa = (1.0 - id_frac);
        }
        vec->L = clamp01(1.0 - kappa);
    }

    /*
     * T: Traffic-analysis resistance
     *
     * T = 1 - P(distinguish from background traffic)
     *
     * Components:
     * - Timing leakage (lower = more resistant)
     * - Traffic regularity (lower = harder to fingerprint)
     * - Size variance (higher = harder to fingerprint)
     * - Traffic shaping (if present, reduces all leakage)
     */
    {
        double timing_resist = 1.0 - prof->timing_leakage;
        double regularity_resist = 1.0 - prof->traffic_regularity;
        double size_resist = prof->size_variance;

        double T_raw = (timing_resist + regularity_resist + size_resist) / 3.0;

        if (prof->has_traffic_shaping) {
            T_raw = T_raw * 0.5 + 0.5; /* Shaping boosts resistance */
        }

        /* Observer's timing accuracy degrades resistance */
        if (obs->monitors_timing) {
            T_raw *= (1.0 - obs->accuracy[VIS_CHANNEL_TIMING] * 0.3);
        }

        vec->T = clamp01(T_raw);
    }

    /*
     * C: Content/behavior confidentiality
     *
     * C = 1 - P(infer content or behavior)
     *
     * Components:
     * - Encryption (primary defense)
     * - Behavioral entropy (higher = harder to classify)
     * - Onion encryption (additional layer)
     * - Observer's DPI capability
     */
    {
        double base_conf;
        if (prof->encrypted) {
            base_conf = 0.85 + 0.1 * prof->behavioral_entropy;
        } else {
            base_conf = 0.1 + 0.3 * prof->behavioral_entropy;
        }

        if (prof->has_onion_encryption) {
            base_conf = base_conf * 0.5 + 0.5; /* Onion layers help */
        }

        /* DPI observer can partially penetrate */
        if (obs->monitors_content) {
            double dpi_penetration = obs->accuracy[VIS_CHANNEL_CONTENT] *
                                     (1.0 - (double)prof->encrypted * 0.8);
            base_conf *= (1.0 - dpi_penetration);
        }

        vec->C = clamp01(base_conf);
    }

    /* Aggregate: geometric mean of all 5 components */
    vec->aggregate = geometric_mean_5(vec->H, vec->U, vec->L, vec->T, vec->C);
}

/* ═══════════════════════════════════════════════════════════════
 * Privacy Hierarchy
 * ═══════════════════════════════════════════════════════════════ */

void priv_compute_hierarchy(const PrivacyVector *vec,
                            PrivacyHierarchy *hier) {
    if (!vec || !hier) return;
    memset(hier, 0, sizeof(*hier));

    hier->level_names[0] = "Identity-location separation";
    hier->level_names[1] = "Unlinkability";
    hier->level_names[2] = "Traffic-analysis resistance";
    hier->level_names[3] = "Metadata indistinguishability";
    hier->level_names[4] = "Formal privacy bound";

    /*
     * REVISED SCORING: Each level requires a SPECIFIC metric to exceed
     * its threshold. Levels are cumulative — you cannot achieve Level N
     * without achieving all levels < N.
     *
     * This prevents a protocol from scoring "5/5" when its actual
     * anonymity metrics (H, U) are weaker than another protocol.
     *
     * Level 1: L >= 0.5  (identity-location separation)
     * Level 2: U >= 0.7  (strong unlinkability — raised threshold)
     * Level 3: T >= 0.5  (traffic-analysis resistance)
     * Level 4: C >= 0.7  AND H >= 0.5  (content confidentiality + anonymity set)
     * Level 5: ALL of H >= 0.7, U >= 0.8, L >= 0.8, T >= 0.6, C >= 0.8
     *          (formal privacy bound — requires strong scores on ALL dimensions)
     */

    /* Level 1: Identity-location separation */
    hier->level_scores[0] = vec->L;

    /* Level 2: Unlinkability (threshold 0.7, not 0.5) */
    hier->level_scores[1] = vec->U;

    /* Level 3: Traffic-analysis resistance */
    hier->level_scores[2] = vec->T;

    /* Level 4: Content confidentiality AND anonymity set */
    hier->level_scores[3] = (vec->C < vec->H) ? vec->C : vec->H;

    /* Level 5: ALL dimensions strong (geometric mean, but with high thresholds) */
    hier->level_scores[4] = geometric_mean_5(
        vec->H, vec->U, vec->L, vec->T, vec->C);

    /* Determine highest achieved level (cumulative, with per-level thresholds) */
    hier->level_achieved = 0;

    /* Level 1: L >= 0.5 */
    if (vec->L >= 0.5) {
        hier->level_achieved = 1;
    } else {
        return;
    }

    /* Level 2: U >= 0.7 (requires Level 1) */
    if (vec->U >= 0.7) {
        hier->level_achieved = 2;
    } else {
        return;
    }

    /* Level 3: T >= 0.5 (requires Level 2) */
    if (vec->T >= 0.5) {
        hier->level_achieved = 3;
    } else {
        return;
    }

    /* Level 4: C >= 0.7 AND H >= 0.5 (requires Level 3) */
    if (vec->C >= 0.7 && vec->H >= 0.5) {
        hier->level_achieved = 4;
    } else {
        return;
    }

    /* Level 5: ALL dimensions strong (requires Level 4) */
    if (vec->H >= 0.7 && vec->U >= 0.8 && vec->L >= 0.8 &&
        vec->T >= 0.6 && vec->C >= 0.8) {
        hier->level_achieved = 5;
    }
}

/* ═══════════════════════════════════════════════════════════════
 * Distinguishability Test
 *
 * Tests: P(M|A) ≈ P(M|¬A) ?
 *
 * For each metadata component M_i, compute the statistical
 * distance between the protocol's traffic signature and
 * typical background traffic.
 * ═══════════════════════════════════════════════════════════════ */

void priv_compute_distinguishability(const ProtocolProfile *prof,
                                     const NetworkObserver *obs,
                                     DistinguishabilityTest *dist) {
    if (!prof || !obs || !dist) return;
    memset(dist, 0, sizeof(*dist));

    /*
     * Volume distance: how different is the traffic volume from background?
     * Regular, predictable traffic is more distinguishable.
     */
    dist->volume_distance = clamp01(prof->traffic_regularity *
        (obs->monitors_traffic ? obs->accuracy[VIS_CHANNEL_TRAFFIC] : 0.0));

    /*
     * Size distance: how different are packet sizes from background?
     * Low variance = distinctive size fingerprint.
     */
    dist->size_distance = clamp01((1.0 - prof->size_variance) *
        (obs->monitors_size ? obs->accuracy[VIS_CHANNEL_SIZE] : 0.0));

    /*
     * Timing distance: how different is the timing from background?
     */
    dist->timing_distance = clamp01(prof->timing_leakage *
        (obs->monitors_timing ? obs->accuracy[VIS_CHANNEL_TIMING] : 0.0));

    /*
     * Route distance: can the observer distinguish routing patterns?
     */
    dist->route_distance = clamp01(
        (double)prof->has_native_address *
        (obs->monitors_route ? obs->accuracy[VIS_CHANNEL_ROUTE] : 0.0) *
        (1.0 - obs->noise_level));

    /*
     * Metadata distance: overall protocol fingerprint
     */
    dist->metadata_distance = clamp01(
        (1.0 - prof->behavioral_entropy) *
        (obs->monitors_behavior ? obs->accuracy[VIS_CHANNEL_BEHAVIOR] : 0.0));

    /* Total: maximum of component distances (worst case) */
    dist->total_distance = dist->volume_distance;
    if (dist->size_distance > dist->total_distance)
        dist->total_distance = dist->size_distance;
    if (dist->timing_distance > dist->total_distance)
        dist->total_distance = dist->timing_distance;
    if (dist->route_distance > dist->total_distance)
        dist->total_distance = dist->route_distance;
    if (dist->metadata_distance > dist->total_distance)
        dist->total_distance = dist->metadata_distance;

    /*
     * Privacy loss bound (differential privacy ε):
     *   ε = ln(max P(M|A) / P(M|¬A))
     *
     * Approximated from total statistical distance:
     *   If Δ ≈ 0, ε ≈ 0 (perfect privacy)
     *   If Δ ≈ 1, ε → ∞ (no privacy)
     *
     *   ε ≈ -ln(1 - Δ) for Δ ∈ [0, 1)
     */
    if (dist->total_distance >= 0.999) {
        dist->epsilon = 10.0; /* Cap at large value */
    } else {
        dist->epsilon = -log(1.0 - dist->total_distance);
    }
}

/* ═══════════════════════════════════════════════════════════════
 * Full Multi-Protocol Comparison
 * ═══════════════════════════════════════════════════════════════ */

void priv_compare_all(ProtocolComparison *cmp,
                      const NetworkObserver *obs) {
    if (!cmp || !obs) return;

    for (int i = 0; i < PROTO_COUNT; i++) {
        /* Privacy vector */
        priv_compute_vector(&cmp->profiles[i], obs, &cmp->vectors[i]);

        /* Privacy hierarchy */
        priv_compute_hierarchy(&cmp->vectors[i], &cmp->hierarchies[i]);

        /* Distinguishability */
        priv_compute_distinguishability(&cmp->profiles[i], obs,
                                        &cmp->distinguishability[i]);

        /* Visibility (reuse existing model) */
        CommProfile comm;
        memset(&comm, 0, sizeof(comm));
        comm.has_ip_address = cmp->profiles[i].has_native_address;
        comm.address_bits   = cmp->profiles[i].address_bits;
        comm.identity_bits  = cmp->profiles[i].identity_bits;
        comm.traffic_rate   = 1.0 - (double)cmp->profiles[i].traffic_regularity * 0.5;
        comm.traffic_variance = cmp->profiles[i].size_variance;
        comm.timing_regularity = cmp->profiles[i].traffic_regularity;
        comm.min_packet_size = 64;
        comm.max_packet_size = 1500;
        comm.size_variance   = cmp->profiles[i].size_variance;
        comm.uses_standard_protocol = 1;
        comm.behavioral_entropy = cmp->profiles[i].behavioral_entropy;
        comm.encrypted       = cmp->profiles[i].encrypted;
        comm.content_entropy = cmp->profiles[i].encrypted ? 0.95 : 0.4;

        vis_analyze(&comm, obs, &cmp->visibility[i]);
    }
}

/* ═══════════════════════════════════════════════════════════════
 * Printing
 * ═══════════════════════════════════════════════════════════════ */

void priv_print_vector(const PrivacyVector *vec, const char *name) {
    printf("  │  %-8s  H=%.3f  U=%.3f  L=%.3f  T=%.3f  C=%.3f  Agg=%.3f\n",
           name, vec->H, vec->U, vec->L, vec->T, vec->C, vec->aggregate);
}

void priv_print_hierarchy(const PrivacyHierarchy *hier, const char *name) {
    static const double thresholds[5] = {0.5, 0.7, 0.5, 0.5, 0.7};
    static const char *threshold_desc[5] = {
        "L>=0.5", "U>=0.7", "T>=0.5", "C>=0.7 & H>=0.5", "ALL strong"
    };
    printf("  │  %-8s  Level %d/5 achieved\n", name, hier->level_achieved);
    for (int i = 0; i < 5; i++) {
        const char *status = ((int)(i + 1) <= hier->level_achieved) ? "PASS" : "----";
        printf("  │    L%d: %-35s  %.3f [%s] (req: %s)\n",
               i + 1, hier->level_names[i], hier->level_scores[i],
               status, threshold_desc[i]);
    }
}

void priv_print_distinguishability(const DistinguishabilityTest *dist,
                                    const char *name) {
    printf("  │  %-8s  Delta=%.4f  epsilon=%.4f", name,
           dist->total_distance, dist->epsilon);
    if (dist->epsilon < 0.5) printf("  (strong privacy)");
    else if (dist->epsilon < 1.0) printf("  (moderate privacy)");
    else if (dist->epsilon < 3.0) printf("  (weak privacy)");
    else printf("  (poor privacy)");
    printf("\n");
}

void priv_print_comparison(const ProtocolComparison *cmp) {
    printf("\n");
    printf("  ╔════════════════════════════════════════════════════════════════╗\n");
    printf("  ║  MULTI-PROTOCOL PRIVACY COMPARISON (modeled estimates)       ║\n");
    printf("  ╚════════════════════════════════════════════════════════════════╝\n");
    printf("\n");

    /* Architecture comparison */
    printf("  ┌─── Architectural Properties ─────────────────────────────────\n");
    printf("  │\n");
    printf("  │  Protocol  │ Addr │ Content │ Onion │ Relay │ Exec │ Cap │ State │ Shape\n");
    printf("  │  ──────────┼──────┼─────────┼───────┼───────┼──────┼─────┼───────┼──────\n");
    for (int i = 0; i < PROTO_COUNT; i++) {
        const ProtocolProfile *p = &cmp->profiles[i];
        printf("  │  %-9s │  %c   │    %c    │   %c   │   %c   │  %c   │  %c  │   %c   │  %c\n",
               p->name,
               p->has_native_address      ? 'Y' : '-',
               p->has_content_addressing  ? 'Y' : '-',
               p->has_onion_encryption    ? 'Y' : '-',
               p->has_relay_separation    ? 'Y' : '-',
               p->has_executable_routing  ? 'Y' : '-',
               p->has_capability_auth     ? 'Y' : '-',
               p->has_state_evolution     ? 'Y' : '-',
               p->has_traffic_shaping     ? 'Y' : '-');
    }
    printf("  │\n");
    printf("  └──────────────────────────────────────────────────────────────\n\n");

    /* Privacy vectors */
    printf("  ┌─── Privacy Vectors P_A = (H, U, L, T, C) ──────────────────\n");
    printf("  │\n");
    printf("  │  H = anonymity-set uncertainty (higher = more anonymous)\n");
    printf("  │  U = unlinkability            (higher = harder to link)\n");
    printf("  │  L = identity/location sep.   (higher = more decoupled)\n");
    printf("  │  T = traffic-analysis resist. (higher = harder to fingerprint)\n");
    printf("  │  C = content confidentiality  (higher = more opaque)\n");
    printf("  │\n");
    for (int i = 0; i < PROTO_COUNT; i++) {
        priv_print_vector(&cmp->vectors[i], cmp->profiles[i].name);
    }
    printf("  │\n");
    printf("  └──────────────────────────────────────────────────────────────\n\n");

    /* Privacy hierarchy */
    printf("  ┌─── Privacy Hierarchy ────────────────────────────────────────\n");
    printf("  │\n");
    for (int i = 0; i < PROTO_COUNT; i++) {
        priv_print_hierarchy(&cmp->hierarchies[i], cmp->profiles[i].name);
        printf("  │\n");
    }
    printf("  └──────────────────────────────────────────────────────────────\n\n");

    /* Distinguishability */
    printf("  ┌─── Distinguishability: P(M|A) vs P(M|not A) ───────────────\n");
    printf("  │\n");
    printf("  │  Delta = max statistical distance (0=indistinguishable, 1=trivial)\n");
    printf("  │  epsilon = privacy loss bound (lower = stronger guarantee)\n");
    printf("  │\n");
    for (int i = 0; i < PROTO_COUNT; i++) {
        priv_print_distinguishability(&cmp->distinguishability[i],
                                      cmp->profiles[i].name);
    }
    printf("  │\n");
    printf("  └──────────────────────────────────────────────────────────────\n\n");

    /* Summary comparison table */
    printf("  ┌─── Summary (under the defined observer model) ──────────────\n");
    printf("  │\n");
    printf("  │  Protocol  │ Non-detect %% │ Priv. Level │    ε    │ η addr\n");
    printf("  │  ──────────┼──────────────┼─────────────┼─────────┼────────\n");
    for (int i = 0; i < PROTO_COUNT; i++) {
        printf("  │  %-9s │   %7.2f%%   │    %d / 5    │ %7.4f │ %.4f\n",
               cmp->profiles[i].name,
               cmp->visibility[i].invisibility * 100.0,
               cmp->hierarchies[i].level_achieved,
               cmp->distinguishability[i].epsilon,
               cmp->visibility[i].address_efficiency);
    }
    printf("  │\n");
    printf("  │  NOTE: All results are modeled estimates, not empirical\n");
    printf("  │  measurements. Protocol parameters are defined in privacy.c.\n");
    printf("  │  See docs/visibility_methodology.md for methodology.\n");
    printf("  └──────────────────────────────────────────────────────────────\n\n");
}
