# Airbot Security & Privacy Model

**Status:** research prototype. The properties below are the ones that are
*implemented and tested*. Properties that are not implemented are listed as
such. Airbot is **not** anonymous software in the sense Tor Browser is, and
nothing here should be read as a claim of untraceability.

Airbot **cannot** make you "100% anonymous" or its traffic "100% undetectable."
No system can. What follows is a set of measurable, individually testable
properties, plus an explicit list of what remains observable.

---

## 1. Roles

Airbot has three mutually exclusive network roles. They are enforced in
`netpolicy.c`, inside the transport chokepoint, not by CLI convention.

| Role | Value | Inbound listener | Direct dial | Local DNS | Anonymity intent |
|---|---|---|---|---|---|
| Direct | `AIRB_MODE_DIRECT` | allowed | allowed | allowed | **none** |
| Privacy client | `AIRB_MODE_PRIVACY` | **forbidden** | **forbidden** | **forbidden** | client address protection |
| Relay | `AIRB_MODE_RELAY` | allowed | allowed | allowed | **none — relays are public** |

A relay is infrastructure. Its address is public by definition. Running a
privacy client in relay mode does not give the client anonymity, and the two
roles are deliberately not combinable.

---

## 2. Layer boundaries

```
EIU  (application object)
  |
  |  <- BLAKE3 content hash lives HERE, inside the object. Never on the wire.
  v
ChaCha20-Poly1305 AEAD  (privframe.c)
  |     confidentiality + authenticity of type, hop counter, length, payload
  |     fresh 12-byte CSPRNG nonce per frame
  v
Padded transport frame  (2-byte bucketed length prefix only)
  |
  v
SOCKS5 / Tor  (socks5.c)  <- hostname resolved remotely, per-connection circuit
  |
  v
TCP / IP  (transport.c)   <- the ONLY module with socket syscalls
```

### What each layer protects

| Layer | Confidentiality | Integrity | Authentication | Anonymity | Unlinkability |
|---|---|---|---|---|---|
| BLAKE3 content hash | no | yes (local) | no | **no** | **no** |
| ChaCha20-Poly1305 | yes | yes | yes (key holders) | no | yes (fresh nonce) |
| Size bucketing | no | no | no | no | partial (hides exact length) |
| Tor | no (Tor's own layer does) | no | no | yes (source address) | partial |

**BLAKE3 integrity is not anonymity.** A content hash is a *stable identifier*:
the same content always produces the same digest. Putting one on the wire — as
the original AIRB frame did — hands any observer a correlation token that
matches the same message at every hop. BLAKE3 is now confined to EIU content
addressing inside the encrypted boundary.

### Key ownership (per-hop, no shared PSK)

The single shared `AIRBOT_PSK_HEX` key has been **replaced**. Each relay owns
a long-term X25519 identity; the client derives a *fresh* per-hop key for every
message:

```
eph_sk_i, eph_pk_i  <- fresh CSPRNG keypair, per hop, per message
shared_i            = X25519(eph_sk_i, relay_pk_i)
k_i                 = HKDF(shared_i, salt = eph_pk_i || relay_pk_i,
                           info = "airbot-onion-v1-hopkey")
```

- **Who can decrypt layer i:** only the holder of `relay_sk_i`. Relay A cannot
  derive `k_B` or `k_C`; measured in the compromise matrix.
- **Forward secrecy:** `eph_sk_i` and `shared_i` are erased immediately after
  derivation, `k_i` after use. Seizing a relay's long-term key later does not
  recover past hop keys.
- **Replay protection:** each layer carries a per-hop message id derived as
  `HKDF(seed, k_i, "airbot-onion-v1-msgid")`, checked against a bounded,
  session-scoped window. The id is inside the ciphertext and differs at every
  hop, so it is neither externally visible nor a cross-hop correlation token.
- **Failure behavior:** a degenerate X25519 result (`OX_ERR_BADKEY`), a failed
  tag (`OX_ERR_AUTH`) or a duplicate id (`OX_ERR_REPLAY`) all reject the frame.
  No path falls back to plaintext.

Primitives: X25519 (RFC 7748), HKDF (RFC 5869), ChaCha20-Poly1305 (RFC 8439),
BLAKE3. All verified against published vectors and, for BLAKE3 and X25519,
against independent implementations. No custom cryptography is on the path.

---

## 3. Threat model

| Adversary | Status | Reasoning |
|---|---|---|
| **Local network observer** (café Wi-Fi, LAN sniffer) | **PARTIALLY PROTECTED** | Sees a TCP connection to the local Tor SOCKS port and to a Tor guard. No destination hostname (resolved remotely), no payload, no protocol magic. Sees packet sizes, timing, and that *some* encrypted protocol is in use. |
| **ISP** | **PARTIALLY PROTECTED** | Sees you connecting to a Tor guard relay, and volume/timing. Does **not** see destination or DNS. Knows you use Tor — that alone is identifying in some jurisdictions. |
| **Local DNS resolver / router** | **PROTECTED** | Privacy mode never calls `getaddrinfo` on a hostname. Names travel as SOCKS5 `ATYP=0x03` and resolve at the Tor exit. Verified by test and by proxy-side capture. |
| **Destination server** | **PROTECTED** (from client IP) | Sees the Tor exit relay's address. Airbot sends no local IP, public IP, MAC, hostname, username, machine ID, or OS identifier. |
| **Malicious / compromised relay** | **PARTIALLY PROTECTED** | Sees its previous and next hop only. **Cannot read the payload** (measured: A=no, B=no) and cannot peel another relay's layer. The **exit** always sees the payload — inherent to onion routing. |
| **Colluding relays** | **PARTIALLY PROTECTED** | Identifier linkage is closed: A+B, A+C, B+C hold unrelated per-hop ids (measured). Timing/volume linkage remains — batching reduces it to 18.8%, it is not eliminated. |
| **Global passive observer** | **NOT PROTECTED** | An adversary seeing both ends correlates by timing and volume. This defeats Tor itself; Airbot does not improve on it. |
| **Traffic-analysis adversary** | **PARTIALLY PROTECTED** | Measured: 100% correlation with immediate forwarding, 18.8% with 250 ms batching + shuffle (baseline 2.5%). Reduced, not solved. No cover traffic. |
| **Protocol-fingerprinting observer** | **PARTIALLY PROTECTED** | No fixed magic, no version byte, no cleartext type or hop counter. But the 2-byte bucketed length prefix and the six-value size distribution are a *behavioral* signature. **It has not been demonstrated that Airbot traffic is indistinguishable from arbitrary internet traffic, and no such claim is made.** |
| **Compromised endpoint** | **NOT PROTECTED** | Malware, a kernel driver, or anyone with read access to process memory recovers keys and plaintext before encryption. Nothing in a userspace application defends against this. |
| **Local EDR / security software** | **NOT PROTECTED** | Endpoint agents see the process, its sockets, and the connection tuple regardless of application-level log redaction. Redaction controls what *Airbot* writes, nothing more. |
| **Malicious destination** | **PARTIALLY PROTECTED** | Cannot see the client IP. Can still fingerprint application behavior, and a malicious payload runs in the EIU VM under whatever capabilities were granted. |
| **Malicious application payload** | **NOT PROTECTED** | The EIU VM's sandbox is out of scope for this document. A payload with the `NETWORK` capability is constrained by the policy gate, but capability assignment itself is not audited here. |

---

## 4. Observability matrix

What each vantage point can see for a single EIU transmission in privacy mode.

| | Client host | Local network | Tor guard | Relay A/B/C | Destination |
|---|---|---|---|---|---|
| Client public IP | yes | yes | **yes** | no | **no** |
| Destination identity | yes | no | no | next-hop only | itself |
| DNS query | none emitted | **none** | no | no | no |
| Payload plaintext | yes (pre-encryption) | no | no | **exit only** | yes (it is the peer) |
| EIU structure | yes | no | no | **exit only** | yes |
| Stable cross-hop identifier | n/a | **no** | **no** | **no** | n/a |
| Hop count | yes | no | no | own position only | no |
| Frame type | yes | no | no | own layer only | yes |
| Packet size | yes | **yes (bucketed)** | **yes** | **yes** | yes |
| Timing | yes | **yes** | **yes** | **yes** | yes |

Rows in bold are the residual exposure. Size and timing are visible at every
network vantage point and are the basis of traffic-analysis attacks Airbot does
not defend against.

---

## 5. Fail-closed invariants

Enforced in `netpolicy.c`, asserted by `airbot privacy-test` (42 tests).

```
privacy mode                -> direct TCP dial            REFUSED
privacy mode                -> inbound listener           REFUSED
privacy mode                -> local DNS on a hostname    REFUSED
proxy not on loopback       -> connection                 REFUSED
Tor unreachable             -> connection                 REFUSED, no fallback
endpoint not SOCKS5         -> connection                 REFUSED
preflight not passed        -> connection                 REFUSED
no session key              -> frame emission             REFUSED, no plaintext
```

There is no code path that converts a failed privacy check into a direct
connection. This is verified by static search over every socket syscall
(`transport.c` is the sole chokepoint) and by the harness.

---

## 6. Cryptography

| Primitive | Status | Verification |
|---|---|---|
| BLAKE3-256 | genuine, portable reference implementation | 30/30 lengths match the official `blake3` library oracle, incl. chunk and Merkle boundaries; `blake3_selftest()` gates the build |
| ChaCha20 | RFC 7539 | official test vector, `crypto-test` |
| Poly1305 | RFC 8439 | official test vector, `crypto-test` |
| ChaCha20-Poly1305 AEAD | RFC 8439 | 11 tests incl. forgery, truncation, AAD tampering |

**A prior `blake3.c` was not BLAKE3.** It reused the BLAKE3 IV with an ad-hoc
ChaCha-style compression function and failed the official vectors. It has been
removed with no fallback. No custom cryptography remains on the security path.

---

## 6b. CRITICAL FINDING AND FIX — the live path was unprotected

A socket capture of the **production** `net-send` showed this:

```
0000  41 49 52 42 01 01 03 00 ...    |AIRB.......|   magic at offset 0
0020  41 49 52 42 4F 54 2D 4E 45 54  |AIRBOT-NET|   plaintext payload
0030  44 12 82 79 ...                                stable content digest
```

`onionx` and `privframe` were both implemented, tested and **never called by
any live command**. Every property previously credited to them was absent from
the wire. Passing unit tests protected nothing.

**Fixed.** `airbchan.c` is now the only sanctioned application data path, and
`transport_send_frame`/`transport_recv_frame` (the legacy cleartext framer)
return `AIRB_ERR_POLICY` in privacy mode — enforced in the transport, not by
caller discipline. `live-test` drives the real production functions over real
sockets and inspects the bytes on every link:

```
link 1 (client->A): 514 bytes  0200830A6F22FA20...  34% printable
link 2 (A->B):      416 bytes  019E7A3D57E5071B...  39% printable
link 3 (B->C):      318 bytes  013CF46C33EA310F...  35% printable
```

No plaintext payload, no AIRB magic, no EIU magic on any link (27/27 tests).

**Also fixed: `ox_peel` was not alias-safe.** It cleared the output struct
before reading the wire, so the natural relay loop
`ox_peel(me, w, p.inner, p.inner_len, &p)` zeroed its own input. `ox_selftest`
missed it by using three separate structs. The wire is now copied first.

### What the link framing leaks

Links carry a 2-byte length then the onion. Bucket padding on the link is not
possible: a nested onion shrinks by exactly 98 bytes per layer and `ox_peel`
authenticates the declared length, so padding would break every tag after the
first. Therefore:

- **Payload size is hidden** — the client pads the innermost payload so the
  first link is always exactly a size bucket (514 = 2 + 512 above).
- **Hop index is revealed** by size: link N is `bucket - N*98`. An observer of
  one link learns roughly where it sits in the chain, not what it carries.

## 6c. Relay key authentication (`relaydir.c`)

Previously nothing authenticated relay public keys, so substituting one gave
an attacker the derived hop key. Now: **explicit BLAKE3 fingerprint pinning,
no trust-on-first-use.** 9/9 cases pass — genuine key accepted; substituted
key, single-bit-flipped key, unknown relay, expired, not-yet-valid, wrong
role, un-repinned rotation and generation rollback all refused.

**Limits, stated plainly:** this is not a PKI. Security rests entirely on
out-of-band fingerprint distribution; there is no signed consensus, no
revocation feed, and no directory authority. Rotation is manual.

## 6d. Hop-index concealment — fixed-size link envelope

Links previously shrank 514 / 416 / 318, so an observer read the remaining
layer count off the wire. Each link is now wrapped in its own constant-size
AEAD envelope:

```
link_eph_pk(32) || nonce(12) || AEAD_k_link( len(2) || onion || pad ) || tag(16)
--------------------------- 1084 bytes, constant ---------------------------
k_link = HKDF(X25519(link_eph_sk, relay_pk), "airbot-link-v1", ...)
```

The label differs from the onion hop key, so the transport envelope and the
onion layer are cryptographically independent: opening the envelope does not
help peel the onion. Outer padding is **inside** the AEAD, so stripping or
extending it fails the tag (tested).

**Measured:** 20 cases — 1 to 4 hops × payloads {0, 1, 64, 300, 600} — all
produce exactly one wire size, 1084 bytes. Live 3-relay capture confirms all
three links at 1084.

**What a relay still learns:** after opening its own envelope it sees the
remaining onion length, and therefore roughly how many layers are left. That
is inherent to this construction and is not concealed.

**Cost:** frame expansion is 41.7x for a 26-byte payload, 1.8x at 600 bytes.

## 6e. Live relay batching

`batch.c` implements the mechanism in the production relay: bounded queue
(32 frames), release on full batch or deadline, CSPRNG Fisher-Yates shuffle,
explicit backpressure, clean flush. No batch id, timestamp or sequence number
is ever transmitted; each frame keeps its own independent envelope.

**Honest status:** the queue is implemented and unit-tested (bounds,
backpressure, deadline, shuffle integrity, flush). The end-to-end correlation
re-measurement against a live 3-relay chain with batching enabled has **NOT**
been run — the 18.8% figure remains a harness result, not a production one.

## 7. Final classification (measured)

Every row below was produced by a test in this repository. Commands are in
section 9. Nothing here is asserted without a test that produces it.

### PROTECTED — demonstrated

| Property | Evidence |
|---|---|
| Destination does not learn the client's public IP | `privacy-fetch` reported exit `109.70.100.15` / `185.243.218.232`; real IP `103.206.9.142`; `check.torproject.org` returned `IsTor:true` |
| ISP/router/local resolver does not learn the destination hostname | unique hostname `airbot-dnsleak-probe-<ts>.example.com` never entered the Windows DNS cache; counter shows 0 local lookups |
| No direct-network fallback when Tor is down | Tor process killed, port 9050 closed → preflight REFUSED, fetch FAILED CLOSED, no socket to the destination |
| IPv6 cannot bypass the privacy path | family gate denies `::1` and `[2001:db8::1]`; IPv4 still allowed; 5 tests |
| Intermediate relay cannot read the payload | compromise matrix: A=no, B=no; only the exit sees plaintext |
| Relay A cannot peel B's or C's layer | `ox_peel` with A's key on B's layer returns `OX_ERR_AUTH` |
| No relay learns the full path | each layer carries only the immediate successor |
| Colluding relays cannot link by Airbot identifier | A+B, A+C, B+C all measured non-linking after per-hop HKDF id derivation |
| Replay rejected | 20 replays rejected; cross-epoch and wrong-relay frames rejected |
| Reordering tolerated without weakening replay | out-of-order 3,1,2 accepted; each still single-use |
| No stable cross-hop token | no 8-byte sequence recurs between hop images; content digest never on the wire |
| No protocol magic / no plaintext payload | `AIRB` absent; payload absent; 33% printable bytes (uniform ≈37%) |
| Exact payload length hidden | 1-byte and 200-byte payloads both emit 286 bytes |
| No persistent client/session/device identifier | static audit §6 |
| Cryptographic correctness | BLAKE3 30/30 vs official oracle; X25519 RFC 7748 + 8/8 ECDH vs OpenSSL; ChaCha20-Poly1305 11/11 |

### PARTIALLY PROTECTED

| Property | What holds / what does not |
|---|---|
| Forward secrecy | Ephemeral per-hop secrets are erased; captured traffic is undecryptable after long-term key erasure. Does **not** help if the key is seized while live, or if the relay was already compromised. |
| Relay compromise | A and B learn nothing but their neighbour. The **exit always sees the payload** — inherent to onion routing. Payload-to-destination confidentiality needs its own end-to-end layer. |
| Protocol fingerprinting | No magic, no version, no type, no hop count. A 2-byte length prefix taking one of six values remains, and the six-value size distribution is itself a signature. **Statistical indistinguishability from ordinary traffic has NOT been demonstrated and is not claimed.** |
| Timing correlation | Measured: immediate forwarding gives an attacker **100%** matching (baseline 2.5%). Batching into 250 ms rounds with shuffling reduces this to **18.8%** (mean anonymity set 5.3) at a cost of up to 250 ms per hop. Reduced, not solved. |

### Measured metadata leakage (Phase 9)

A classifier given only frame sizes, counts and timing from the production
path separates protocol activity at **74.2%** against a 25% baseline.

- **Payload size: PROTECTED.** 16 B vs 600 B is indistinguishable (53.3%,
  chance). Both emit exactly 1084 bytes.
- **Message count: NOT PROTECTED.** A 3-frame burst is perfectly separable
  from a single message.
- **Idle vs active: NOT PROTECTED.** Trivially separable by duration.

The constant-size envelope does the job it was designed for. There is no
corresponding defence for how many messages are sent or when.

### NOT PROTECTED — stated plainly

| Threat | Why |
|---|---|
| **Global passive adversary** | An observer of both ends correlates by timing and volume. This defeats Tor itself; Airbot does not improve on it. |
| **Sufficiently powerful timing correlation** | Batching raises ambiguity within a round; an adversary aggregating across many rounds defeats it. |
| **Endpoint compromise** | Malware, a debugger, a privileged process or a malicious OS reads plaintext and keys before encryption. |
| **Local EDR / security software** | Sees the process and its sockets regardless of application-level redaction. |
| **Malicious exit / destination** | The exit sees the payload by construction. |
| **Colluding entry+exit by traffic pattern** | Identifier linkage is closed; timing/volume linkage is not. |
| **Independent audit** | None performed. |

### Answers to the 10 required questions

1. **Destination learns client public IP?** No — measured, exit address only.
2. **ISP learns destination hostname?** No — remote DNS via SOCKS5 `ATYP=0x03`; ISP does see you are using Tor.
3. **Relay decrypts the full payload?** Only the exit. A and B cannot.
4. **Two relays correlate the same message?** Not via Airbot identifiers (measured). Yes via timing.
5. **Captured traffic decryptable after later key compromise?** No, once ephemeral secrets are erased.
6. **Frame replayable?** No — duplicate, cross-epoch and wrong-relay frames all rejected.
7. **Global passive observer correlates ingress/egress?** **Yes. NOT PROTECTED.**
8. **Endpoint malware defeats the system?** **Yes. NOT PROTECTED.**
9. **IPv6 bypass?** No — explicitly denied in privacy mode.
10. **Airbot traffic still fingerprintable?** Likely yes, by size distribution and behaviour. Reduced, not eliminated.

### Terminology, kept distinct

- **Network address anonymity** — PROTECTED (Tor).
- **Message unlinkability** — PROTECTED against identifier linkage; NOT against timing.
- **Confidentiality** — PROTECTED to the exit; the exit sees plaintext.
- **Forward secrecy** — PARTIAL, as scoped above.
- **Traffic-analysis resistance** — PARTIAL (100% → 18.8%), not solved.
- **Protocol fingerprint resistance** — PARTIAL, unmeasured against real-world classifiers.
- **Endpoint security** — NOT PROVIDED.

Airbot is **not** untraceable, undetectable, 100% anonymous, or proof against
a state-level adversary. Those claims cannot be substantiated for this or any
comparable system, and are not made here.

---

## 8. Wire format (final)

```
off  size  field       visible?  why it must exist
0    2     length      YES       framing; quantized to 6 buckets
2    12    nonce       YES       AEAD requirement; uniformly random
14   B     ciphertext  opaque    the message
14+B 16    tag         YES       Poly1305; changes every frame
```

Per onion layer, inside the ciphertext:

```
32  ephemeral X25519 public key   (AAD-bound, fresh per hop per message)
12  nonce
N   encrypted { flags | hop_index | per-hop msg_id | next_addr | inner }
16  tag
```

Everything else — type, hop counter, message id, next hop, payload — is
encrypted. The length prefix is the only externally visible application byte
pair, and it carries a bucket, not a size.

---

## 9. Reproducing the evidence

```sh
make privacy-check            # full gate: audit + all four suites
sh tools/static-audit.sh      # 7-point static audit
./build/airbot-net.exe crypto-test        # 11 vectors
./build/airbot-net.exe onion-test         # per-hop separation, FS, replay
./build/airbot-net.exe privacy-test       # 42 invariants
./build/airbot-net.exe adversarial-test   # 28 red-team checks
python build/b3oracle.py                  # BLAKE3 vs official library
python build/x_diag2.py                   # X25519 vs OpenSSL
python build/corr_final.py                # correlation before/after
```

Captures are in `evidence/`:
`01-tor-validation.txt`, `02-dns-leak.txt`, `04-tor-failclosed.txt`,
`05-correlation-before-after.txt`.

---

## 7. Limitations — read this before trusting anything above

1. **Relay identity distribution is out of scope.** Relay public keys are
   assumed already known and authentic. There is no directory, no consensus,
   and no signature over relay descriptors — a man-in-the-middle who substitutes
   relay public keys defeats the layering. This is the largest remaining gap.
2. **The exit always sees the payload.** Inherent to onion routing. If the
   destination is not the exit, the payload needs its own end-to-end encryption.
3. **`onionx` is not yet wired into the live socket path.** Per-hop onion
   encryption is implemented, verified and gated by tests, but `net-send`/`relay`
   still use the single-layer `privframe`. Wiring is the next integration step.
4. **Batching is implemented in the experiment, not in the relay.** The 18.8%
   figure comes from a measured harness; the production relay still forwards
   immediately.
5. **Multi-hop over the real internet is untested.** Tor egress is proven
   end-to-end, but the A/B/C relay chain has only run over loopback — the test
   host is behind NAT (public 103.206.9.142 / local 192.168.0.100).
6. **IPv6 is unhandled, not disabled.** `transport.c` requests `AF_INET` only.
   That avoids an IPv6 path today but is not a considered IPv6 policy.
7. **Windows-only.** Winsock, and a TinyCC build using a hand-written
   `ws2_32.def`.
8. **Unaudited.** No external review. The threat model above is the author's
   analysis, not a verdict from a Tor or traffic-analysis specialist, and it
   should be reviewed by one before any real use.
