# Airbot — Security Audit Guide

Orientation for an external reviewer. Airbot is a **research prototype**, not
production software. It is **NOT release-ready**; the open blockers are in
§14 and `evidence/FINAL-RELEASE-AUDIT.txt`.

No claim is made that Airbot provides anonymity against a global observer, a
compromised endpoint, or a resourced traffic-analysis adversary. Those are
listed as NOT PROTECTED and are not defended against.

---

## 1. Threat model

| Adversary | Status |
|---|---|
| Local network observer / hostile Wi-Fi | PARTIALLY PROTECTED |
| ISP | PARTIALLY PROTECTED (sees Tor use, not destination) |
| Local DNS resolver / router | PROTECTED |
| Destination server | PROTECTED from client IP |
| Malicious entry or middle relay | PROTECTED from payload; sees neighbours |
| Malicious exit relay | NOT PROTECTED — the exit sees the payload |
| Colluding relays | PARTIALLY — no shared identifier; timing still links |
| Traffic-analysis adversary | PARTIALLY — size nil, timing reduced not solved |
| Global passive observer | **NOT PROTECTED** |
| Compromised endpoint / malicious OS / EDR | **NOT PROTECTED** |
| Supply-chain compromise | **NOT PROTECTED** |

## 2. Trust assumptions

1. Tor is installed, genuine and functioning. Airbot verifies a SOCKS5
   handshake on loopback but does not attest the Tor binary.
2. Relay public keys are distributed out-of-band and pinned by fingerprint.
   **If that channel is compromised, the layering is defeated.** There is no
   signed directory (see §14, C3).
3. The endpoint is not compromised. Plaintext and keys exist there.
4. The hand-written X25519 and BLAKE3 are correct. They match reference
   implementations but have not been reviewed (see `CRYPTO-REVIEW.md`).

## 3. Protocol

```
application payload
  |  true_len(2) || payload || random padding  -> constant size per hop count
  v
ox_wrap: one ChaCha20-Poly1305 layer per hop, per-hop X25519+HKDF keys
  v
link envelope: AEAD(k_link, len(2) || onion || padding)  -> CONSTANT 1084 B
  v
SOCKS5 -> Tor -> relay
```

## 4. Wire format

```
off  size  field            visible to a passive observer
0    32    link_eph_pk      yes, uniformly random
32   12    nonce            yes, uniformly random
44   1024  ciphertext       yes as bytes, opaque
1068 16    Poly1305 tag     yes, changes every frame
                            TOTAL 1084 B, CONSTANT at every hop
```

Nothing else is visible: no magic, no version, no message type, no hop
counter, no digest, no length of the real payload.

## 5–7. Keys, nonces, onion construction

See `CRYPTO-REVIEW.md` §2–5. Summary: fresh ephemeral X25519 per hop per
message; HKDF-SHA256 (RFC 5869) with distinct `info` labels for onion vs
link keys; random
nonces, never counters; ephemeral secrets erased after derivation.

## 8. Relay processing

```
recv 1084 B -> envelope_open (k_link) -> ox_peel (k_i)
  -> replay check on the per-hop id
  -> if exit: recover payload; else: re-seal inner onion for the next hop
```

A relay learns: its predecessor (from the transport), its successor (from its
own layer header), and nothing else. It cannot peel another relay's layer.

## 9. Replay protection

Bounded 512-entry per-hop id window, session-scoped. Rotates when full — a
rotation can re-admit a very old frame; this is the documented trade-off
against unbounded memory. Cross-epoch and wrong-relay frames are rejected by
authentication, not by the window.

## 10. Batching

`batch.c`: bounded 32-frame queue, release on full batch or deadline, CSPRNG
Fisher-Yates shuffle, explicit backpressure. No batch id, timestamp or
sequence number is transmitted.

Measured on the **production relay**: timing correlation 100% → 13.0%
(baseline 1.0%), mean anonymity set 1.0 → 7.8, at a latency cost of mean
119 ms / p95 222 ms. **This reduces local correlation opportunities. It does
not mathematically prevent global traffic correlation.**

## 11. Tor integration

`netpolicy.c` enforces three roles: DIRECT, PRIVACY (client), RELAY. In
privacy mode: direct dials refused, inbound listeners refused, local DNS
refused, IPv6 explicitly denied, proxy must be loopback and must complete a
live SOCKS5 handshake. Names travel as `ATYP=0x03` and resolve at the exit.

## 12. Failure behaviour

Every privacy-mode failure terminates. There is **no** direct fallback, no
plaintext fallback, no alternate resolver. Verified with Tor killed
mid-session: refused, zero sockets to the destination
(`evidence/16-failclosed.txt`).

## 13. Metadata exposure

**Visible:** that a Tor connection exists; frame sizes (constant 1084 B);
timing; connection count and duration.
**Not visible on the wire:** payload, payload size, hop index, hop count,
message type, any stable cross-hop identifier.
**Local:** the OS, `netstat`, and EDR see the process and its sockets
regardless of application-level redaction. Airbot cannot change this.

## 14. Known limitations / open blockers

- **C2 PARTIAL.** Multi-hop verified over real Tor onion services (11/11), but
  all relays ran on one host. A true independent-host test is NOT DONE.
- **C3 OPEN.** Fingerprint pinning, not a signed directory.
- **C4 OPEN — four sub-blockers:**
  - C4.1 independent review — OPEN (hand-written X25519/BLAKE3)
  - C4.2 compiler differential — OPEN (only tcc 0.9.27, no optimizer)
  - C4.3 constant-time — NOT PROVEN (no leak detected; detection != proof)
  - C4.4 **KDF validation — CLOSED.** Migrated to standard HKDF-SHA256
    (RFC 5869); agrees with the published vectors, with pyca/cryptography,
    and with Python hashlib/hmac. Functional conformance only.
- No formal analysis against a model such as Sphinx.

## 15. Test methodology

```sh
make privacy-check          # full gate
./build/airbot-net.exe privacy-test adversarial-test live-test ...
sh tools/static-audit.sh    # 7-point network/identity/logging audit
```

Test-design pitfalls already hit and fixed, worth knowing when reading the
suites: correlation metrics must group **egress** events (not compare ingress
to egress) and must break ties randomly; pattern-matching fewer than 4 bytes
against ciphertext produces false failures (~4.65% per run at 2 bytes).

## 16. Reproducible build

```sh
tools/tcc/tcc.exe -o build/airbot-net.exe $(ls src/*.c | grep -v bench_debug) \
    -Isrc -lws2_32
```

Verified byte-reproducible: two consecutive builds from identical source are
byte-identical. Revision and hashes in `evidence/14-artifact-integrity.txt`.

## 17. Evidence index

`evidence/` — Tor validation, DNS-leak, fail-closed, correlation, hop
concealment, live batching, performance, fuzzing, X25519 side-channel and
machine-code audit, `internet-multihop/`, and `FINAL-RELEASE-AUDIT.txt`.
