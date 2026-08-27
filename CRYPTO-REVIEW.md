# Airbot — Cryptographic Review Scope

This document exists for an external cryptographer. It lists **only** the
cryptographic surface, what has been verified, and what has not. Everything
else about the project is deliberately out of scope here.

**Please read the limitations section first.** X25519 and BLAKE3 are
hand-written and unreviewed; that is the single largest risk in this codebase.

> **KDF: FUNCTIONALLY VERIFIED (was NOT PROVEN).**
> Key derivation is now standard **HKDF-SHA256 (RFC 5869)**, replacing HKDF's
> shape over HMAC-BLAKE3 — a bespoke instantiation that no published vector
> covered and no second implementation could check. It is verified against
> three independent sources that all agree: the published RFC 5869 A.1–A.3
> vectors, pyca/cryptography's OpenSSL-backed HKDF, and Python hashlib/hmac
> for SHA-256 and HMAC-SHA256 across block boundaries and oversized keys.
> **This establishes functional conformance only** — not security, and not
> constant-time behaviour (still C4.3).

---

## 1. Primitives and their provenance

| Primitive | Source | File | Verification performed |
|---|---|---|---|
| X25519 | **hand-written**, from RFC 7748 + ref10 field arithmetic | `src/x25519.c` | RFC 7748 §6.1 vector; 8/8 random ECDH vs OpenSSL; 10/10 Hamming-weight classes vs OpenSSL |
| BLAKE3-256 | **hand-written**, from the BLAKE3 spec + reference impl | `src/blake3.c` | 30/30 lengths vs the official `blake3` library, incl. chunk (1023/1024/1025) and Merkle boundaries to 65536 B |
| ChaCha20-Poly1305 | pre-existing in project | `src/chacha20.c` | RFC 7539 §2.4.2 and RFC 8439 §2.5.2 vectors; 11 negative tests (forgery, truncation, AAD tampering, wrong key/nonce) |
| SHA-256 / HMAC-SHA256 | **hand-written**, FIPS 180-4 / RFC 2104 | `src/sha256.c` | 19/19 lengths vs Python hashlib (incl. 55/56/57/63/64/65 block boundaries); 5/5 HMAC cases vs Python hmac incl. key > blocksize |
| **HKDF-SHA256** | **hand-written**, RFC 5869 | `src/sha256.c` | RFC 5869 A.1–A.3 published PRK+OKM; pyca/cryptography (OpenSSL) agreement; 8 boundary cases; domain- and salt-separation checks |

A prior `blake3.c` reused the BLAKE3 IV with an ad-hoc compression function
and **failed the official vectors**. It was removed, not repaired.

---

## 2. Key hierarchy

```
relay long-term identity        X25519 keypair, generated per relay
        |
        |  client generates a FRESH ephemeral keypair per hop, per message
        v
shared_i = X25519(eph_sk_i, relay_pk_i)
k_i      = HKDF(ikm = shared_i,
                salt = eph_pk_i || relay_pk_i,
                info = "airbot-onion-v1-hopkey")        <- onion layer key

link:
shared_L = X25519(link_eph_sk, relay_pk)
k_link   = HKDF(same salt form, info = "airbot-link-v1") <- transport envelope

per-hop message id:
hop_id_i = HKDF(ikm = random 16-byte seed,
                salt = k_i,
                info = "airbot-onion-v1-msgid")
```

**Domain separation** is by the `info` label only. The envelope key and the
onion hop key are derived from the *same* X25519 agreement inputs when a
client talks directly to a relay it also onion-addresses. **An auditor should
check whether label-only separation is sufficient here**, or whether the salt
should also differ.

Erasure: `eph_sk` and `shared` are zeroed immediately after derivation;
`k_i` after use. Erasure uses a `volatile` byte loop (`ox_secure_zero`), which
is best-effort and does not defeat register spills or swap.

---

## 3. Nonces

| Where | Nonce | Source |
|---|---|---|
| onion layer | 12 B | `csprng_bytes` (RtlGenRandom) per layer, per message |
| link envelope | 12 B | `csprng_bytes` per frame |
| message id seed | 16 B | `csprng_bytes` per message |

All nonces are **random, never counters**. With a fresh key per layer per
message, nonce reuse requires both a repeated key and a repeated nonce.
**An auditor should confirm there is no path where a key is reused across
two frames**, since that would make the 96-bit random nonce the sole
protection.

CSPRNG: `csprng_bytes` in `src/chacha20.c` uses `RtlGenRandom` with a
`QueryPerformanceCounter`-seeded fallback. **The fallback path is weak and
should be reviewed** — it is reached only if `RtlGenRandom` fails.

---

## 4. AEAD usage

Both layers use ChaCha20-Poly1305:

```
onion layer i:  AEAD(k_i,   nonce_i, AAD = eph_pk_i (32 B),
                    plaintext = flags|hop|msg_id|next_addr|inner_len|inner)

link envelope:  AEAD(k_link, nonce,  AAD = link_eph_pk (32 B),
                    plaintext = onion_len(2) || onion || random padding)
```

The AAD binds the ephemeral public key so it cannot be swapped. Outer
transport padding lives **inside** the AEAD, so stripping or extending it
fails the tag (tested).

---

## 5. Constructions to scrutinise

These are compositions, not primitives, and are where design errors are most
likely:

1. **Onion layering** (`ox_wrap` / `ox_peel`) — is one AEAD layer per hop with
   independent ephemeral keys sound against a tagging attack by an
   intermediate relay?
2. **The link envelope** — does wrapping an already-authenticated onion in a
   second AEAD under a related key introduce any interaction?
3. **Per-hop message ids** — `hop_id_i = HKDF(ikm = seed, salt = k_i)`. Only
   `hop_id_i` is placed in layer *i*'s plaintext; the 16-byte seed is used
   solely as HKDF input and zeroed before return (`onionx.c:227`, `:293`), so
   it never reaches the wire and no relay observes it. Verified by inspection.
   What an auditor should still check: the hop key is used as the HKDF **salt**
   rather than the IKM, and the truncation of a 32-byte HKDF output to 16
   bytes. Both look sound but neither is covered by a published test vector.
4. **Replay window** — bounded 512-entry table, rotates when full. A rotation
   re-admits very old frames. Is that acceptable?
5. **No key agreement for relay identities** — relay public keys are pinned by
   fingerprint out-of-band; there is no signed directory.

---

## 6. What has NOT been done

- **No independent review.** This document is a request for one.
- **HKDF/HMAC-BLAKE3 has no test vectors.**
- **Constant-time is NOT PROVEN.** Source and machine-code inspection found no
  secret-dependent branches or memory accesses, and 34,560 timing samples
  showed 0/55 significant pairwise differences — but only under tcc 0.9.27
  with no optimizer, and with no cache/microarchitectural analysis.
- **No formal analysis** of the onion construction against a model such as
  Sphinx.
- **No fault-injection or power analysis.**

---

## 7. Reproducing the crypto tests

```sh
./build/airbot-net.exe crypto-test      # RFC 7539/8439 + 11 negative tests
./build/airbot-net.exe onion-test       # layering, key separation, replay
python build/b3oracle.py                # BLAKE3 vs official library, 30 lengths
python build/x_diag2.py                 # X25519 vs OpenSSL
python build/ct_openssl.py              # X25519 across Hamming-weight classes
python build/ctcheck.py                 # machine-code constant-time check
./build/fuzz.exe                        # 68,827 malformed-input cases
```
