<div align="center">

# Airbot

### Executable Information System

<br>

[![C99](https://img.shields.io/badge/C99-00599C?style=for-the-badge&logo=c&logoColor=white)](https://en.wikipedia.org/wiki/C99)
[![License](https://img.shields.io/badge/License-Academic_Research-blue?style=for-the-badge)](LICENSE)
[![Zero Dependencies](https://img.shields.io/badge/Dependencies-Zero-brightgreen?style=for-the-badge)](#zero-dependencies)
[![Build](https://img.shields.io/badge/Build-Passing-success?style=for-the-badge&logo=gnu-bash&logoColor=white)](#building)

<br>

> **A dependency-free C99 research prototype for executable information units and autonomous information addressing.**

<br>

[Getting Started](#getting-started) •
[Architecture](#architecture) •
[Usage](#usage) •
[Examples](#example-program) •
[Documentation](#documentation)

</div>

---

## Tech Stack

<div align="center">

<table>
<tr>
<td align="center" width="110">
<img src="https://cdn.jsdelivr.net/gh/devicons/devicon/icons/c/c-original.svg" width="48" height="48" alt="C" />
<br><b>C99</b>
<br><sub>Core Language</sub>
</td>
<td align="center" width="110">
<img src="https://cdn.jsdelivr.net/gh/devicons/devicon/icons/gcc/gcc-original.svg" width="48" height="48" alt="GCC" />
<br><b>GCC</b>
<br><sub>Compiler</sub>
</td>
<td align="center" width="110">
<img src="https://cdn.jsdelivr.net/gh/devicons/devicon/icons/cmake/cmake-original.svg" width="48" height="48" alt="Make" />
<br><b>Make</b>
<br><sub>Build System</sub>
</td>
<td align="center" width="110">
<img src="https://cdn.jsdelivr.net/gh/devicons/devicon/icons/linux/linux-original.svg" width="48" height="48" alt="Linux" />
<br><b>Linux</b>
<br><sub>Platform</sub>
</td>
<td align="center" width="110">
<img src="https://cdn.jsdelivr.net/gh/devicons/devicon/icons/windows8/windows8-original.svg" width="48" height="48" alt="Windows" />
<br><b>Windows</b>
<br><sub>Platform</sub>
</td>
<td align="center" width="110">
<img src="https://cdn.jsdelivr.net/gh/devicons/devicon/icons/git/git-original.svg" width="48" height="48" alt="Git" />
<br><b>Git</b>
<br><sub>Version Control</sub>
</td>
</tr>
</table>

<br>

| Category | Technology |
|----------|------------|
| **Language** | ![C](https://img.shields.io/badge/C99-00599C?style=flat-square&logo=c&logoColor=white) Pure ISO C99 — no extensions |
| **Compiler** | ![GCC](https://img.shields.io/badge/GCC-A42E2B?style=flat-square&logo=gnu&logoColor=white) with `-Wall -Wextra -Wpedantic` |
| **Build** | ![Make](https://img.shields.io/badge/GNU%20Make-A42E2B?style=flat-square&logo=gnu&logoColor=white) single Makefile |
| **Crypto** | ![X25519](https://img.shields.io/badge/X25519-333333?style=flat-square) ![ChaCha20](https://img.shields.io/badge/ChaCha20--Poly1305-333333?style=flat-square) ![HKDF](https://img.shields.io/badge/HKDF--SHA256-333333?style=flat-square) ![BLAKE3](https://img.shields.io/badge/BLAKE3--256-333333?style=flat-square) |
| **Privacy** | ![Tor](https://img.shields.io/badge/Tor%20SOCKS5-7D4698?style=flat-square) ![Onion](https://img.shields.io/badge/Per--hop%20onion-333333?style=flat-square) ![Failclosed](https://img.shields.io/badge/Fail--closed-critical?style=flat-square) |
| **Architecture** | ![Custom VM](https://img.shields.io/badge/16--bit%20VM-4B32C3?style=flat-square) ![32 Opcodes](https://img.shields.io/badge/32%20Opcodes-4B32C3?style=flat-square) |
| **Platform** | ![Linux](https://img.shields.io/badge/Linux-FCC624?style=flat-square&logo=linux&logoColor=black) ![Windows](https://img.shields.io/badge/Windows-0078D6?style=flat-square&logo=windows&logoColor=white) |
| **Dependencies** | ![None](https://img.shields.io/badge/Zero-brightgreen?style=flat-square) Built entirely from scratch |

</div>

---

## What is Airbot?

Airbot is a research prototype of an **application-layer object model** in
which **mobile, stateful computational objects** carry their own behaviour,
state and capabilities instead of being passive packets.

> **It does not replace IP.** Airbot rides on ordinary TCP/IP, and in privacy
> mode on top of Tor. The EIA is an application-layer identifier, not a
> routing address — nothing routes on it. The contribution is the object model
> and the privacy layer above the network, not a new network layer.

### Three Distinct Contributions

| # | Contribution | What It Demonstrates |
|---|---|---|
| **1** | **Network Object Model** | Data, behavior, state, and capabilities encoded in a single mobile bitstring |
| **2** | **Programmable Stateful Routing** | The object carries authorized computational logic that participates in its own routing and evolution |
| **3** | **Privacy / Observability Model** | Formal model for estimating non-detection probability under defined observer types |

### Core Primitives

| Primitive | Name | Description |
|-----------|------|-------------|
| **EIU** | Executable Information Unit | A finite bitstring simultaneously encoding data, behavior, state, and capabilities |
| **EIA** | Executable Information Address | An address carrying routing logic, authentication, and capability constraints |

### Central Equation

```
A_{t+1} = F(A_t, E_t, Auth(A_t, E_t))
```

The unit observes permitted information from the environment, executes its encoded behavior under authorization constraints, and produces a successor state. Routing isn't merely selecting the next hop — the object carries authorized computational logic that participates in its own evolution.

---

## Architecture

```mermaid
flowchart TD
    CLI["CLI<br/>main.c"]

    subgraph toolchain["Toolchain"]
        ASM["Assembler<br/>.airasm to bytecode"]
        DIS["Disassembler<br/>bytecode to .airasm"]
    end

    subgraph runtime["Runtime"]
        VER["Static Verifier<br/>eBPF-inspired, pre-execution"]
        VM["Airbot Bit Machine<br/>16-bit, 32 opcodes, gas-metered"]
        ENV["Environment<br/>host for one EIU"]
        ST["State Evolution<br/>A t+1 = F A t, E t, Auth"]
        REP["Replicator<br/>constrained spawn"]
    end

    subgraph model["Object Model"]
        EIU["EIU<br/>Executable Information Unit"]
        EIA["EIA<br/>Executable Information Address"]
        CAP["Capability System<br/>64-bit mask, HMAC tokens"]
    end

    subgraph wire["Encoding"]
        BS["Bitstream Engine<br/>bit-level I/O"]
        TLV["TLV Wire Format<br/>prefix varints"]
    end

    subgraph crypto["Cryptography, from scratch"]
        B3["BLAKE3-256<br/>content addressing"]
        CC["ChaCha20-Poly1305<br/>RFC 8439 AEAD"]
        ONI["Onion Layer"]
    end

    subgraph analysis["Analysis"]
        PRIV["Privacy Model<br/>5-component vector"]
        VIS["Visibility Engine<br/>7 channels"]
        EXP["Metrics, Experiments, Matrix"]
    end

    CLI --> ASM
    CLI --> DIS
    CLI --> ENV
    CLI --> ST
    CLI --> REP
    CLI --> ONI
    CLI --> PRIV
    CLI --> VIS
    CLI --> EXP

    ASM --> VM
    DIS --> VM
    VER --> VM

    ENV --> VER
    ENV --> VM
    ENV --> EIU
    ENV --> CAP

    ST --> ENV
    ST --> EIU
    ST --> B3
    REP --> ENV
    REP --> EIU
    REP --> CAP

    EIU --> CAP
    EIU --> B3
    EIA --> CAP
    EIA --> B3
    EIA --> VM

    ONI --> CC
    CC --> B3

    TLV --> BS
    PRIV --> BS
    VIS --> BS
    EXP --> ST
    EXP --> BS
    EXP --> B3
```

| Component | Description |
|-----------|-------------|
| **Airbot Bit Machine (ABM)** | Custom 16-bit VM with 32 opcodes, 8 registers, gas-metered execution |
| **Self-Describing Wire Format** | TLV-based binary encoding with prefix varints |
| **Capability System** | 64-bit capability bitmask with HMAC-based authorization tokens |
| **Static Verifier** | eBPF-inspired pre-execution safety analysis |
| **Content Addressing** | BLAKE3-256 hash for self-certifying addresses |
| **Crypto Layer** | ChaCha20-Poly1305 AEAD (RFC 8439) for onion encryption |
| **Privacy Model** | 5-component privacy vector with multi-protocol comparison |
| **Visibility Engine** | 7-channel observability analysis with defined observer presets |

---

## Getting Started

### Prerequisites

- **GCC** with C99 support (or any C99-compatible compiler)
- **GNU Make**
- No external libraries required!

### Building

```bash
# Build optimized binary
make

# Build with debug symbols
make debug

# Run tests
make test

# Clean build artifacts
make clean
```

### Quick Start

```bash
# Assemble an example program
./build/airbot assemble examples/counter.airasm -o counter.eiu

# Verify bytecode safety
./build/airbot verify counter.eiu

# Execute the EIU
./build/airbot run counter.eiu --fuel 1000 --trace
```

---

## Usage

```bash
# Assemble an Airbot program
./build/airbot assemble examples/counter.airasm -o counter.eiu

# Verify bytecode safety
./build/airbot verify counter.eiu

# Execute an EIU
./build/airbot run counter.eiu --fuel 1000 --trace

# Run the full security gate (crypto vectors, privacy invariants,
# adversarial suite, live socket integration, static audit)
make privacy-check

# Evolve an EIU over multiple steps
./build/airbot evolve counter.eiu --steps 50 --trace

# Test constrained replication
./build/airbot replicate counter.eiu --count 5

# Run hypothesis experiments
./build/airbot experiment --hypothesis H1
./build/airbot experiment --hypothesis H2
./build/airbot experiment --hypothesis H3

# Compute metrics
./build/airbot metrics counter.eiu

# Population simulation
./build/airbot matrix --population 10 --steps 100

# Create an EIA
./build/airbot eia-create --target <hash> --cap 0xFF

# Disassemble binary
./build/airbot disassemble counter.eiu
```

---

## Example Program

```asm
; counter.airasm — A state-evolving counter EIU
.fuel 100

        LDI R0, 0          ; counter = 0
        LDI R1, 10         ; limit = 10
loop:
        ADDI R0, 1          ; counter++
        CMP R0, R1          ; compare counter vs limit
        JZ done             ; if equal, halt
        JMP loop            ; otherwise continue
done:
        EMIT R0             ; output final value
        HALT
```

---

## Instruction Set

The Airbot Bit Machine uses **32 opcodes** encoded in fixed **16-bit instructions**:

| Opcode | Mnemonic | Description |
|--------|----------|-------------|
| `0x00` | `NOP` | No operation |
| `0x01` | `HALT` | Stop execution |
| `0x06` | `LDI` | Load immediate value into register |
| `0x07` | `ADD` | Add two registers |
| `0x0B` | `XOR` | Bitwise XOR |
| `0x10` | `JMP` | Unconditional jump |
| `0x11` | `JZ` | Jump if zero |
| `0x16` | `EMIT` | Output register value |
| `0x1A` | `SELF` | Read own bytecode (self-reference) |
| `0x1B` | `SPAWN` | Constrained replication |
| ... | ... | See [`docs/isa.md`](docs/isa.md) for complete ISA |

---

## Cryptography

All primitives are implemented in-tree with no external libraries. Each is
verified against published vectors **and**, where an independent
implementation exists, differentially tested against it.

| Primitive | Standard | Purpose | Verification |
|---|---|---|---|
| **X25519** | RFC 7748 | Per-hop key agreement | RFC vector; 8/8 random ECDH and 10/10 Hamming-weight classes vs OpenSSL |
| **HKDF-SHA256** | RFC 5869 | All key derivation | RFC 5869 A.1–A.3 published values; pyca/cryptography; Python `hmac` |
| **SHA-256 / HMAC** | FIPS 180-4, RFC 2104 | Underlies HKDF | 19 lengths vs `hashlib` incl. block boundaries |
| **ChaCha20-Poly1305** | RFC 8439 | AEAD for onion layers and link envelopes | RFC 7539/8439 vectors + 11 negative tests |
| **BLAKE3-256** | BLAKE3 spec | EIU content addressing, relay fingerprints | 30/30 lengths vs the official library |

> **Note on scope.** Passing test vectors establishes *functional
> conformance*, not security. X25519 and BLAKE3 are hand-written and have
> **not** been independently reviewed, and constant-time behaviour is **not
> proven**. See [`CRYPTO-REVIEW.md`](CRYPTO-REVIEW.md).

---

## Privacy Mode

Airbot has three mutually exclusive network roles, enforced inside the
transport rather than by convention:

| Role | Inbound | Direct dial | Local DNS | Intent |
|---|---|---|---|---|
| `DIRECT` | allowed | allowed | allowed | ordinary networking, **no privacy** |
| `PRIVACY` | forbidden | forbidden | forbidden | client address protection |
| `RELAY` | allowed | allowed | allowed | infrastructure, **public by design** |

In privacy mode the data path is:

```
EIU -> per-hop onion (X25519 + HKDF-SHA256 + ChaCha20-Poly1305)
    -> constant-size 1084-byte link envelope
    -> Tor SOCKS5 (hostname resolved remotely)
    -> relay -> relay -> exit
```

Every link frame is **exactly 1084 bytes** regardless of payload size or hop
count, so the wire reveals neither payload length nor position in the chain.

```bash
# validate the privacy path before sending anything
AIRBOT_PRIVACY=1 ./build/airbot privacy-preflight

# fetch through Tor; fails closed if Tor is unavailable
./build/airbot privacy-fetch --host example.com --path /

# run a relay (public infrastructure, not a privacy client)
./build/airbot relay --port 9401 --next <next>.onion:9402
```

If Tor is unreachable, misconfigured, or killed mid-session, every operation
**fails closed** — there is no direct fallback, no plaintext fallback and no
alternate resolver.

---

## Security Status

**This is a research prototype and is NOT release-ready.** It has not had an
independent security audit.

Demonstrated with captured evidence in [`evidence/`](evidence/):

- destination sees a Tor exit address, never the client IP
- zero local DNS lookups; a unique hostname never entered the OS resolver cache
- IPv6 explicitly denied in privacy mode
- an intermediate relay cannot read the payload or peel another relay's layer
- replay, cross-session and wrong-key frames rejected
- constant 1084-byte links, verified by socket capture across 20 hop/payload combinations
- 68,827 fuzz cases, 0 accepted, 0 crashes

**Not protected:** a global passive observer, a compromised endpoint or OS, a
malicious exit relay (it sees the payload by construction), and traffic
confirmation by a well-resourced adversary. Batching reduces measured local
timing correlation from 100% to 13.0% — it does not prevent global
correlation.

No claim is made that Airbot is anonymous, untraceable or undetectable.

Open blockers, tracked in
[`evidence/FINAL-RELEASE-AUDIT.txt`](evidence/FINAL-RELEASE-AUDIT.txt):
independent-host multi-hop testing, a signed relay directory, independent
cryptographic review, and constant-time proof.

For reviewers: [`SECURITY-AUDIT-GUIDE.md`](SECURITY-AUDIT-GUIDE.md) and
[`CRYPTO-REVIEW.md`](CRYPTO-REVIEW.md).

---

## Project Structure

```
airbot/
├── Makefile                 # Pure C99 build — single command
├── README.md                # You are here!
├── .gitignore               # Git ignore rules
│
├── src/                     # All source code (46 files)
│   ├── bitstream.*          # Bit-level I/O engine
│   ├── blake3.*             # BLAKE3-256 hash (from scratch)
│   ├── chacha20.*           # ChaCha20-Poly1305 AEAD (RFC 8439)
│   ├── tlv.*                # Wire format (TLV codec)
│   ├── eiu.*                # EIU core (Executable Information Unit)
│   ├── eia.*                # EIA addressing (Executable Information Address)
│   ├── capability.*         # Authorization & capability system
│   ├── vm.*                 # Airbot Bit Machine (16-bit VM)
│   ├── verifier.*           # Static verifier (eBPF-inspired)
│   ├── environment.*        # Execution environment
│   ├── state.*              # State evolution
│   ├── replicator.*         # Constrained replication
│   ├── assembler.*          # Assembler (.airasm → bytecode)
│   ├── disassembler.*       # Disassembler (bytecode → assembly)
│   ├── metrics.*            # Metrics & measurements
│   ├── experiments.*        # Hypothesis testing engine
│   ├── matrix.*             # Population simulation
│   ├── visibility.*         # 7-channel observability engine
│   ├── privacy.*            # Formal privacy model (5-component vector)
│   ├── onion.*              # Onion encryption layer
│   ├── benchmark.*          # Performance benchmarking
│   ├── crypto_test.*        # Cryptographic test vectors
│   └── main.c               # CLI entry point
│
├── examples/                # Example programs
│   ├── counter.airasm       # State-evolving counter
│   ├── hello.airasm         # Hello world
│   └── quine.airasm         # Self-referencing quine
│
├── docs/                    # Documentation
│   ├── experiment_plan.md   # Experiment methodology
│   └── visibility_methodology.md  # Visibility analysis docs
│
└── tools/                   # Development tools
    └── tcc/                 # Tiny C Compiler (optional)
```

---

## Prototype Test Suite

The system tests three hypotheses. All implemented hypotheses passed the prototype's test suite:

| Hypothesis | Description | Result |
|------------|-------------|--------|
| **H1** (Encoded Behavior) | Different bit patterns produce different computational behaviors | 126 unique behaviors from 256 EIUs (49.2%) |
| **H2** (State Evolution) | EIUs evolve through distinct state transitions | 100 steps, 100 distinct states (100%) |
| **H3** (Constrained Replication) | EIUs produce valid successors only under authorization constraints | 4/4 tests passed |

> **Note**: A prototype passing its own test suite is not the same as independent empirical validation. These results demonstrate that the implementation behaves as designed.

---

## Visibility Analysis

Under the defined ISP observer model, Airbot produced an estimated **83.67% non-detection probability** versus **15.77%** for the modeled IPv4 baseline. Under the DPI model: **78.10%** vs **2.25%**.

These are modeled estimates computed from defined signal functions and observer parameters. See [`docs/visibility_methodology.md`](docs/visibility_methodology.md) for complete parameter documentation and reproducibility instructions.

### Structural Properties *(not empirical — follow from the design)*

| Property | Value | Meaning |
|----------|-------|---------|
| **κ** | 0.0 | Identity is logically decoupled from location in the EIA addressing model |
| **η** | 0.87 | 87% of EIA address bits encode identity/capability (vs 0% for IPv4) |

---

## Prior Art

Airbot explicitly addresses three limitations identified in **Active Network** architectures (1996–2002): resource metering, code verification, and capability-based authorization.

| System | Comparison |
|--------|-----------|
| **Active Networks** | Airbot adds gas metering, static verification, and capability authorization |
| **NDN** | Airbot routes by executable logic, not content name |
| **JADE / Mobile Agents** | Airbot operates at network-layer primitive, not application-layer JVM construct |
| **Tor** | Airbot embeds routing logic in the object itself, not in relay selection |

---

## Zero Dependencies

<div align="center">

```
┌───────────────────────────────────────────────────┐
│                                                   │
│    ✦ No external libraries                        │
│    ✦ No package managers                          │
│    ✦ No runtime dependencies                      │
│    ✦ No frameworks                                │
│    ✦ Pure C99 from first principles               │
│                                                   │
│    Just: gcc + make = airbot                      │
│                                                   │
└───────────────────────────────────────────────────┘
```

</div>

Every algorithm — from BLAKE3 hashing to ChaCha20-Poly1305 encryption to the VM interpreter — is implemented from scratch in pure C99.

---

> **Zero dependencies is a portability and reproducibility property, not a
> security one.** It means every primitive is hand-written in this tree, which
> is precisely why the cryptography needs independent review — see
> [`CRYPTO-REVIEW.md`](CRYPTO-REVIEW.md).

## Code Statistics

| Metric | Value |
|--------|-------|
| **Total Source Files** | 46 |
| **Language** | 100% C99 |
| **External Dependencies** | 0 |
| **VM Opcodes** | 32 |
| **VM Registers** | 8 |
| **Crypto Primitives** | 5 (BLAKE3, ChaCha20, Poly1305, AEAD, HMAC) |
| **Privacy Vector Dimensions** | 5 (H, U, L, T, C) |
| **Visibility Channels** | 7 |

---

## Documentation

| Document | Description |
|----------|-------------|
| [`docs/experiment_plan.md`](docs/experiment_plan.md) | Experiment methodology and plan |
| [`docs/visibility_methodology.md`](docs/visibility_methodology.md) | Visibility analysis methodology |

---

## License

Research prototype — academic use.

---

<div align="center">

**Built from scratch in pure C99**

<br>

![C](https://img.shields.io/badge/C-00599C?style=for-the-badge&logo=c&logoColor=white)
![Made from Scratch](https://img.shields.io/badge/Made%20From-Scratch-orange?style=for-the-badge)
![Zero Dependencies](https://img.shields.io/badge/Zero-Dependencies-brightgreen?style=for-the-badge)

</div>
