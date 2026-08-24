# Airbot Visibility Model — Methodology & Parameter Documentation

## Purpose

This document describes **exactly** how the visibility analysis numbers are produced,
so any reader can evaluate whether the results are meaningful and reproduce them.

## What Is Measured

The visibility analysis computes the **estimated non-detection probability** of an
Airbot unit versus a conventional IPv4 host under three defined observer models.

These are **modeled estimates**, not empirical measurements from real networks. The
model defines detection probability per observable channel, then combines them
assuming channel independence.

## The Model

### Observable Channels (7 total)

| Channel | Index | What It Represents |
|---------|-------|-------------------|
| Address (IP/MAC) | 0 | Can the observer see a native network address? |
| Traffic Pattern | 1 | Can the observer detect anomalous traffic volume/rate? |
| Timing Analysis | 2 | Can the observer fingerprint inter-packet timing? |
| Packet Sizes | 3 | Can the observer fingerprint packet size distributions? |
| Behavior/DPI | 4 | Can the observer do deep packet inspection of protocol behavior? |
| Route Analysis | 5 | Can the observer trace routing paths via IP? |
| Content Inspection | 6 | Can the observer read/classify payload content? |

### Per-Channel Detection Formula

For each channel i:

```
P_i = enabled_i × signal_i(profile) × accuracy_i(observer) × (1 - noise)
```

Where:
- `enabled_i` = 1 if the observer monitors this channel, 0 otherwise
- `signal_i(profile)` = how detectable the unit is on this channel (0.0 to 1.0)
- `accuracy_i(observer)` = observer's classification accuracy (0.0 to 1.0)
- `noise` = background traffic noise level (0.0 to 1.0)

### Signal Functions (How Profile Maps to Detectability)

These are the model-defined signal functions. Each maps a communication
profile parameter to a detection signal strength:

| Channel | Signal Formula | Rationale |
|---------|---------------|-----------|
| Address | `signal = has_ip_address` (0 or 1) | Binary: either has an IP or doesn't |
| Traffic | `signal = rate × (1 - variance)` | Low rate + high variance = harder to detect |
| Timing | `signal = timing_regularity` | Random timing = no fingerprint |
| Size | `signal = 1 - size_variance` | Variable sizes = harder to fingerprint |
| Behavior | `signal = (1 - behavioral_entropy) × proto_factor` | High entropy + standard protocol = camouflaged |
| Route | `signal = has_ip_address` (0 or 1) | No IP → no traceable route |
| Content | `signal = (1 - encrypted) × (1 - content_entropy)` | Encrypted + high entropy = opaque |

`proto_factor` = 0.2 if mimicking a standard protocol, 1.0 if using a novel protocol.

### Combined Observability

Assuming channel independence (this is a modeling assumption):

```
O(A,N) = 1 - Product_{i=0}^{6} (1 - P_i)
```

Non-detection probability:

```
P(non-detect) = Product_{i=0}^{6} (1 - P_i)
```

### Why Channel Independence Is Assumed

This is a simplification. In real networks, channels may be correlated (e.g.,
traffic patterns correlate with timing). The independence assumption provides
a **lower bound on observability** — real observability would likely be higher
because correlated channels provide more information than independent ones.

## Observer Presets

### ISP Observer
Monitors addresses, traffic, timing, packet sizes, and routes. Does NOT do DPI.

| Channel | Enabled | Accuracy | Noise |
|---------|---------|----------|-------|
| Address | Yes | 0.95 | 0.6 |
| Traffic | Yes | 0.80 | 0.6 |
| Timing | Yes | 0.60 | 0.6 |
| Size | Yes | 0.70 | 0.6 |
| Behavior | No | — | — |
| Route | Yes | 0.85 | 0.6 |
| Content | No | — | — |

### DPI Observer (Deep Packet Inspection)
Monitors all channels.

| Channel | Enabled | Accuracy | Noise |
|---------|---------|----------|-------|
| Address | Yes | 0.99 | 0.4 |
| Traffic | Yes | 0.90 | 0.4 |
| Timing | Yes | 0.85 | 0.4 |
| Size | Yes | 0.80 | 0.4 |
| Behavior | Yes | 0.75 | 0.4 |
| Route | Yes | 0.90 | 0.4 |
| Content | Yes | 0.70 | 0.4 |

### Passive Observer
Monitors only traffic patterns — no address visibility, no DPI.

| Channel | Enabled | Accuracy | Noise |
|---------|---------|----------|-------|
| Address | No | — | — |
| Traffic | Yes | 0.60 | 0.7 |
| Timing | Yes | 0.50 | 0.7 |
| Size | Yes | 0.40 | 0.7 |
| Behavior | No | — | — |
| Route | No | — | — |
| Content | No | — | — |

## Communication Profiles

### Airbot (EIA-only)

| Parameter | Value | Justification |
|-----------|-------|---------------|
| has_ip_address | 0 | EIA addressing has no IP |
| has_mac_address | 0 | Tunneled or encapsulated |
| traffic_rate | 0.1 msg/s | Low-rate bursty communication |
| traffic_variance | 0.9 | Intentionally variable |
| timing_regularity | 0.1 | Randomized timing |
| min/max_packet_size | 16 / 512 | Variable sizes |
| size_variance | 0.8 | Intentionally variable |
| uses_standard_protocol | 1 | Mimics HTTP/DNS |
| behavioral_entropy | 0.9 | High unpredictability |
| encrypted | 1 | Payload encrypted |
| content_entropy | 0.99 | Near-maximum (encrypted) |
| identity_bits | 320 | 256-bit hash + 64-bit capability |
| address_bits | 368 | 320 identity + 48 header/bytecode |

### Conventional IPv4

| Parameter | Value | Justification |
|-----------|-------|---------------|
| has_ip_address | 1 | Standard IPv4 |
| has_mac_address | 1 | Standard Ethernet |
| traffic_rate | 1.0 msg/s | Typical web traffic |
| traffic_variance | 0.3 | Somewhat regular |
| timing_regularity | 0.6 | Semi-periodic |
| min/max_packet_size | 64 / 1500 | Standard MTU range |
| size_variance | 0.4 | Moderate variation |
| uses_standard_protocol | 1 | HTTP/HTTPS |
| behavioral_entropy | 0.5 | Predictable patterns |
| encrypted | 0 | Mixed (modeled as unencrypted) |
| content_entropy | 0.4 | Partially readable |
| identity_bits | 0 | IPv4 carries 0 identity info |
| address_bits | 32 | 32-bit IPv4 address |

## Important Caveats

1. **These are modeled estimates, not empirical measurements.** The accuracy
   values, noise levels, and signal parameters are researcher-defined, not
   measured from real network data.

2. **The channel independence assumption** is a simplification. Real-world
   observability may be higher due to cross-channel correlations.

3. **The conventional baseline** models unencrypted IPv4 traffic. Modern
   HTTPS traffic would have higher content_entropy and encrypted=1, which
   would improve the conventional baseline's non-detection probability.

4. **The Airbot profile assumes optimal countermeasures**: encryption,
   randomized timing, variable sizes, and protocol mimicry. An Airbot that
   does NOT implement these countermeasures would have lower non-detection.

5. **κ (identity-location coupling) = 0** is a structural property of the
   EIA design, not an empirical measurement. It means the address format
   carries no inherent location information.

6. **η (address efficiency) = 0.87** is a structural property: the ratio
   of identity+capability bits to total address bits in the EIA format.

## How to Reproduce

```bash
# Run all three observer analyses
build\airbot.exe visibility --observer ISP
build\airbot.exe visibility --observer DPI
build\airbot.exe visibility --observer PASSIVE
```

The source code for all computations is in:
- `src/visibility.h` — Model definitions and signal function documentation
- `src/visibility.c` — All computations, observer presets, and profile parameters

Every number in the output is deterministically computed from the parameters
documented above. No randomness is involved.
