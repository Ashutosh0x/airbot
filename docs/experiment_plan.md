# Airbot Experiment Plan — Phase 1 Audit & Design

## 1. Current Architecture

### Source Files (37 total: 19 .c + 18 .h)

| Layer | Module | Status | Purpose |
|---|---|---|---|
| Core | `bitstream.h/.c` | ✅ Working | Bit-level I/O, error codes |
| Core | `blake3.h/.c` | ✅ Working | BLAKE3-256 hash |
| Core | `tlv.h/.c` | ✅ Working | TLV wire format |
| Protocol | `eiu.h/.c` | ✅ Working | EIU structure + serialization |
| Protocol | `capability.h/.c` | ✅ Working | Capability authorization |
| Protocol | `eia.h/.c` | ✅ Working | Executable Information Address |
| Execution | `vm.h/.c` | ✅ Working | 32-opcode VM (ABM) |
| Execution | `verifier.h/.c` | ✅ Working | Static bytecode verifier |
| Execution | `environment.h/.c` | ✅ Working | Execution sandbox |
| State | `state.h/.c` | ✅ Working | State evolution tracker |
| State | `replicator.h/.c` | ✅ Working | Constrained replication |
| Tooling | `assembler.h/.c` | ✅ Working | Assembly → bytecode |
| Tooling | `disassembler.h/.c` | ✅ Working | Bytecode → text |
| Tooling | `metrics.h/.c` | ✅ Working | EIU analysis |
| Privacy | `visibility.h/.c` | ⚠️ Invalid model | Channel-based observability (independence assumption invalid) |
| Privacy | `privacy.h/.c` | ⚠️ Modeled only | Multi-protocol comparison (modeled, not empirical) |
| Integration | `experiments.h/.c` | ✅ Working | H1/H2/H3 hypothesis testing |
| Integration | `matrix.h/.c` | ✅ Working | Population simulation |
| Integration | `main.c` | ✅ Working | CLI (12 commands) |

### NEW Modules (being built)

| Module | Purpose | Status |
|---|---|---|
| `onion.h/.c` | Sphinx-like onion routing layer | 🔨 Building |
| `benchmark.h/.c` | Empirical traffic analysis + scorecard | 🔨 Building |

---

## 2. Existing Privacy Model — What's Wrong

### The Invalid Independence Assumption

The current model computes:

$$O(A,N) = 1 - \prod_{i=0}^{6}(1 - P_i)$$

This assumes **statistical independence** between detection channels. The audit identifies this as **fundamentally invalid** because:
- Routing paths dictate latency (timing) — channels 2 and 5 are correlated
- Payload type dictates packet size and volume — channels 1, 3, and 6 are correlated
- Traffic patterns leak timing information — channels 1 and 2 are correlated

### What Must Replace It

**Empirical classification-based measurement:**
- Generate actual traffic traces (simulated or captured)
- Extract flow features (timing, sizes, volume, burstiness, entropy)
- Train a classifier to distinguish protocol traffic from background
- Use classifier AUC as the empirical detectability metric
- Use entry/exit correlation accuracy as the empirical unlinkability metric

### What We Keep

The **privacy vector** $\mathcal{P}_A = (H, U, L, T, C)$ is valid as a framework, but:
- $H$ and $U$ must be measured empirically, not analytically
- $T$ must come from classifier performance, not preset probabilities
- $L$ remains a structural property (valid as-is)
- $C$ depends on encryption implementation (valid as-is)
- $\eta$ and $\kappa$ are **architectural metrics** — must NOT contribute to privacy scores

---

## 3. Experimental Weaknesses

| # | Weakness | Impact | Fix |
|---|---|---|---|
| 1 | Synthetic detection probabilities | Non-detection scores are meaningless | Replace with empirical classifier |
| 2 | No onion routing | H=0.425, U=0.573 — far below Tor | Build Airbot+Onion layer |
| 3 | No traffic normalization | Variable sizes leak information | Test fixed-size cells, padding |
| 4 | Self-validation | Prototype validates its own hypotheses | Need independent test harness |
| 5 | No adversarial testing | VM/verifier untested against attacks | Fuzzing, malformed inputs |
| 6 | No performance measurement | Unknown overhead | Benchmark against Tor/IPv4 |
| 7 | Tor baseline may be inaccurate | Model penalizes Tor for low T | Review Tor's actual properties |

---

## 4. Components to Modify

### `visibility.h/.c` — Deprecate synthetic scores
- Keep the framework for UI display
- Add explicit "MODELED — NOT EMPIRICAL" warnings
- Do not use $\prod(1-P_i)$ as evidence

### `privacy.h/.c` — Connect to empirical measurements
- Keep the privacy vector framework
- Wire H, U, T to empirical benchmark results
- Remove $\eta$ from any aggregate privacy score
- Corrected hierarchy scoring (already fixed: Airbot now 1/5, Tor 2/5)

### `main.c` — Add benchmark command
- `airbot benchmark` — run full empirical comparison
- `airbot onion` — demonstrate onion routing simulation

---

## 5. New Components Required

### `onion.h/.c` — Airbot+Onion Privacy Transport
- Sphinx-like packet format
- 3-relay circuit
- Per-hop capability authorization
- End-to-end Airbot encryption
- Replay protection
- Privacy metrics computation

### `benchmark.h/.c` — Empirical Measurement Framework
- Traffic trace generation (IPv4, Tor, Airbot, Airbot+Onion, background)
- Feature extraction (timing, size, burstiness, entropy)
- Statistical classifier (threshold-based / 1-NN)
- AUC, F1, precision, recall computation
- Entry/exit correlation experiment
- Final scorecard generation

---

## 6. Reproducibility Requirements

- All traffic generation uses deterministic PRNG with documented seeds
- All experiments run from a single command: `airbot benchmark`
- Results include confidence intervals (multiple independent runs)
- Exact configuration recorded in output
- No manual parameter tuning between runs
- All code is pure C99, builds with TCC, zero dependencies

---

## 7. Hypotheses (Falsifiable)

### Primary Hypothesis

**H0:** Airbot+Onion does not outperform Tor on the selected privacy metrics.

**H1:** Airbot+Onion demonstrates statistically significant improvement over Tor on at least one major privacy dimension while maintaining comparable or acceptable performance.

### Secondary Hypothesis

Airbot+Onion can approach or exceed Tor's unlinkability and anonymity-set properties while retaining Airbot capabilities that Tor does not provide.

### "Beating Tor" is Defined as Three Possible Wins

| Win Type | Definition |
|---|---|
| **Privacy Win** | Airbot+Onion statistically outperforms Tor in H, U, or traffic distinguishability |
| **Functional Win** | Airbot provides capabilities Tor does not provide while maintaining comparable privacy |
| **System Win** | Airbot+Onion achieves comparable or better privacy with acceptable performance overhead |

---

## 8. Experiment Phases

| Phase | Task | Output |
|---|---|---|
| 1 | Audit existing code ✅ | This document |
| 2 | Build onion module | `onion.h/.c` |
| 3 | Build benchmark module | `benchmark.h/.c` |
| 4 | Wire into CLI | `airbot onion`, `airbot benchmark` |
| 5 | Run experiments | Empirical scorecard |
| 6 | Generate comparison | `docs/tor_vs_airbot_results.md` |
