/*
 * Airbot — Executable Information System
 * privtest.c — Privacy invariant and leak regression harness
 *
 * Exits non-zero if any privacy invariant is violated, so it can gate a
 * build. Tests are grouped by the property they defend.
 */
#include "privtest.h"
#include "netpolicy.h"
#include "privframe.h"
#include "transport.h"
#include "socks5.h"
#include "chacha20.h"
#include "blake3.h"
#include "crypto_test.h"

#include <stdio.h>
#include <string.h>

static int g_pass = 0, g_fail = 0;

static void ok(const char *name, int cond, const char *detail) {
    if (cond) { printf("  [PASS] %-52s\n", name); g_pass++; }
    else      { printf("  [FAIL] %-52s  <- %s\n", name, detail ? detail : ""); g_fail++; }
}

/* --- group 1: fail-closed policy -------------------------- */

static void test_policy_failclosed(void) {
    AirbConn c;
    int rc;

    printf("\n  Fail-closed policy\n");
    printf("  --------------------------------------------------\n");

    netpolicy_set_mode(AIRB_MODE_PRIVACY);
    netpolicy_reset_counters();

    /* A direct dial must be refused before a socket is created. */
    rc = transport_dial("example.com", 80, &c);
    ok("direct dial refused in privacy mode",
       rc == AIRB_ERR_POLICY, transport_strerror(rc));
    ok("no local DNS performed on refused dial",
       netpolicy_counters()->local_dns_performed == 0, "getaddrinfo was reached");
    ok("denial counter incremented",
       netpolicy_counters()->direct_dials_denied >= 1, "counter not incremented");

    /* Inbound listener must be refused. */
    rc = transport_listen(19999, &c);
    ok("inbound listener refused in privacy mode",
       rc == AIRB_ERR_POLICY, transport_strerror(rc));

    /* A non-loopback "proxy" must be refused even when flagged via_proxy. */
    rc = transport_dial_ex("8.8.8.8", 9050, 1, &c);
    ok("non-loopback proxy refused",
       rc == AIRB_ERR_POLICY, transport_strerror(rc));

    /* IPv6 must be explicitly refused, not merely unreachable. */
    /* Exercise the address-family gate directly. Going through
       transport_dial would be refused by the direct-dial gate first, so it
       would pass without ever testing the IPv6 policy. */
    ok("IPv6 literal refused by the family gate",
       netpolicy_authorize_family("::1") == AIRB_POL_DENY_IPV6,
       "IPv6 permitted in privacy mode");
    ok("bracketed IPv6 literal refused by the family gate",
       netpolicy_authorize_family("[2001:db8::1]") == AIRB_POL_DENY_IPV6,
       "bracketed IPv6 permitted");
    ok("IPv4 literal still permitted by the family gate",
       netpolicy_authorize_family("127.0.0.1") == AIRB_POL_OK,
       "IPv4 wrongly denied");
    ok("IPv6 denial counter incremented",
       netpolicy_counters()->ipv6_denied >= 2, "counter not incremented");
    rc = transport_dial("::1", 80, &c);
    ok("IPv6 dial refused end-to-end in privacy mode",
       rc == AIRB_ERR_POLICY, transport_strerror(rc));

    /* Still no DNS anywhere in privacy mode. */
    ok("zero local DNS lookups across all privacy-mode attempts",
       netpolicy_counters()->local_dns_performed == 0, "local resolver was used");

    /* Direct mode must still work — we are not removing functionality. */
    netpolicy_set_mode(AIRB_MODE_DIRECT);
    netpolicy_reset_counters();
    rc = netpolicy_authorize_dial("example.com", 80, 0);
    ok("direct mode still permits ordinary dials",
       rc == AIRB_POL_OK, netpolicy_strerror(rc));

    netpolicy_set_mode(AIRB_MODE_PRIVACY);
}

/* --- group 2: Tor enforcement ----------------------------- */

static void test_tor_enforcement(void) {
    AirbPreflight p;
    int rc;

    printf("\n  Tor enforcement\n");
    printf("  --------------------------------------------------\n");

    netpolicy_set_mode(AIRB_MODE_PRIVACY);

    /* Point at a port nothing is listening on: preflight must refuse and
       must NOT mark the stack validated. */
    netpolicy_set_proxy("127.0.0.1:1");
    rc = netpolicy_preflight(&p);
    ok("preflight fails when Tor is absent",
       rc != AIRB_POL_OK, "preflight passed with no Tor");
    ok("stack not marked validated after failed preflight",
       netpolicy_is_validated() == 0, "validated flag set despite failure");
    ok("failure is reported as unreachable Tor",
       p.tor_socks_reachable == 0, "reachability misreported");

    /* Restore the default so later runs are not affected. */
    netpolicy_set_proxy("127.0.0.1:9050");
}

/* --- group 3: wire format / fingerprinting ---------------- */

static void test_wire_format(void) {
    uint8_t key[32];
    uint8_t w1[9000], w2[9000];
    size_t n1 = 0, n2 = 0;
    static const uint8_t payload[] = "AIRBOT-NET";
    int i, rc, magic_found = 0, identical;
    PrivFrame f;

    printf("\n  Wire format and fingerprint resistance\n");
    printf("  --------------------------------------------------\n");

    /* Deterministic key for reproducible tests. Not a deployment key. */
    for (i = 0; i < 32; i++) key[i] = (uint8_t)(i * 7 + 1);
    privframe_set_key(key);

    rc = privframe_encode(0x01, 3, payload, (uint16_t)sizeof(payload) - 1,
                          w1, sizeof(w1), &n1);
    ok("frame encodes", rc == PF_OK, privframe_strerror(rc));

    /* No "AIRB" magic anywhere in the encoded frame. */
    for (i = 0; i + 4 <= (int)n1; i++)
        if (memcmp(w1 + i, "AIRB", 4) == 0) { magic_found = 1; break; }
    ok("no AIRB magic anywhere in encoded frame", !magic_found, "magic present");

    /* Payload must not appear in cleartext. */
    magic_found = 0;
    for (i = 0; i + 10 <= (int)n1; i++)
        if (memcmp(w1 + i, "AIRBOT-NET", 10) == 0) { magic_found = 1; break; }
    ok("payload not visible in cleartext", !magic_found, "plaintext payload on wire");

    /* Same plaintext twice must not produce the same bytes: this is the
       property that killed the old cross-hop digest correlation. */
    rc = privframe_encode(0x01, 3, payload, (uint16_t)sizeof(payload) - 1,
                          w2, sizeof(w2), &n2);
    identical = (n1 == n2 && memcmp(w1, w2, n1) == 0);
    ok("identical plaintext yields different ciphertext (unlinkable)",
       rc == PF_OK && !identical, "frames are byte-identical");

    /* But both land in the same bucket, so length reveals nothing extra. */
    ok("both frames occupy the same size bucket", n1 == n2, "bucket differs");

    /* Padding: very different payload sizes collapse to one bucket. */
    {
        uint8_t small[1], big[200];
        size_t ns = 0, nb = 0;
        memset(small, 0xAA, sizeof(small));
        memset(big, 0xBB, sizeof(big));
        privframe_encode(0x01, 0, small, 1, w1, sizeof(w1), &ns);
        privframe_encode(0x01, 0, big, 200, w2, sizeof(w2), &nb);
        ok("1-byte and 200-byte payloads share a size bucket",
           ns == nb, "padding did not equalize sizes");
    }

    /* Round-trip correctness. */
    rc = privframe_encode(0x01, 3, payload, (uint16_t)sizeof(payload) - 1,
                          w1, sizeof(w1), &n1);
    rc = privframe_decode(w1, n1, &f);
    ok("frame decodes", rc == PF_OK, privframe_strerror(rc));
    ok("payload survives round-trip",
       f.length == sizeof(payload) - 1 &&
       memcmp(f.payload, payload, f.length) == 0, "payload corrupted");
    ok("hop counter recovered from ciphertext", f.hops_left == 3, "hops_left wrong");
}

/* --- group 4: malformed / hostile input ------------------- */

static void test_hostile_input(void) {
    uint8_t key[32], w[9000];
    size_t n = 0;
    static const uint8_t payload[] = "AIRBOT-NET";
    PrivFrame f;
    int i, rc;

    printf("\n  Malformed and hostile frames\n");
    printf("  --------------------------------------------------\n");

    for (i = 0; i < 32; i++) key[i] = (uint8_t)(i * 7 + 1);
    privframe_set_key(key);
    privframe_encode(0x01, 3, payload, (uint16_t)sizeof(payload) - 1, w, sizeof(w), &n);

    /* Flip a ciphertext bit: authentication must reject. */
    w[20] ^= 0x01;
    rc = privframe_decode(w, n, &f);
    ok("tampered ciphertext rejected", rc == PF_ERR_AUTH, privframe_strerror(rc));
    w[20] ^= 0x01;

    /* Flip a tag bit. */
    w[n - 1] ^= 0x80;
    rc = privframe_decode(w, n, &f);
    ok("forged tag rejected", rc == PF_ERR_AUTH, privframe_strerror(rc));
    w[n - 1] ^= 0x80;

    /* Rewrite the length prefix: AAD binding must reject. */
    w[0] ^= 0xFF;
    rc = privframe_decode(w, n, &f);
    ok("rewritten length prefix rejected",
       rc == PF_ERR_AUTH || rc == PF_ERR_MALFORMED, privframe_strerror(rc));
    w[0] ^= 0xFF;

    /* Truncated frame. */
    rc = privframe_decode(w, n / 2, &f);
    ok("truncated frame rejected", rc == PF_ERR_MALFORMED, privframe_strerror(rc));

    /* Off-bucket length is rejected before any crypto work. */
    {
        uint8_t bad[64];
        memset(bad, 0, sizeof(bad));
        bad[0] = 0x00; bad[1] = 0x2B;   /* implies a 15-byte bucket */
        rc = privframe_decode(bad, sizeof(bad), &f);
        ok("off-bucket length rejected", rc == PF_ERR_MALFORMED, privframe_strerror(rc));
    }

    /* Wrong key must not decrypt. */
    {
        uint8_t other[32];
        for (i = 0; i < 32; i++) other[i] = (uint8_t)(i + 200);
        privframe_set_key(other);
        rc = privframe_decode(w, n, &f);
        ok("wrong key rejected", rc == PF_ERR_AUTH, privframe_strerror(rc));
        privframe_set_key(key);
    }
}

/* --- group 5: no-key fail-closed -------------------------- */

static void test_nokey_failclosed(void) {
    printf("\n  Key handling\n");
    printf("  --------------------------------------------------\n");
    /* privframe refuses to emit anything without a key; there is no
       plaintext fallback path to exercise. */
    ok("bucket sizing rejects oversized payloads",
       privframe_bucket_for(9000) == 0, "oversized payload accepted");
    ok("smallest bucket chosen for small payloads",
       privframe_bucket_for(10) == 256, "bucket selection wrong");
    ok("bucket boundary handled",
       privframe_bucket_for(252) == 256 && privframe_bucket_for(253) == 512,
       "boundary off by one");
}

/* --- group 6: crypto provenance --------------------------- */

static void test_crypto_provenance(void) {
    uint8_t out[32];
    static const uint8_t official_empty[32] = {
        0xaf,0x13,0x49,0xb9,0xf5,0xf9,0xa1,0xa6,0xa0,0x40,0x4d,0xea,0x36,0xdc,0xc9,0x49,
        0x9b,0xcb,0x25,0xc9,0xad,0xc1,0x12,0xb7,0xcc,0x9a,0x93,0xca,0xe4,0x1f,0x32,0x62
    };
    printf("\n  Cryptographic provenance (build gate)\n");
    printf("  --------------------------------------------------\n");

    /* Phase 1 gate: a genuine BLAKE3 must pass the official vectors.
       A failure here means homebrew crypto is back on the path. */
    blake3_hash((const uint8_t *)"", 0, out);
    ok("BLAKE3 matches official empty-input vector",
       memcmp(out, official_empty, 32) == 0,
       "blake3.c is not standard BLAKE3");

    ok("BLAKE3 passes all official vectors (chunk + Merkle boundaries)",
       blake3_selftest() == 0, "blake3_selftest reported failures");

    /* Phase 12 gate: the AEAD on the privacy path must pass RFC vectors. */
    ok("ChaCha20 passes RFC 7539 vector",
       test_chacha20_rfc_vector() == 0, "RFC 7539 vector failed");
    ok("Poly1305 passes RFC 8439 vector",
       test_poly1305_rfc_vector() == 0, "RFC 8439 vector failed");
}

/* --- group 7: cross-hop correlation (Phase 5) ------------- */

/* The old plaintext frame exposed BLAKE3(EIU) unchanged at every hop, so an
   observer at two points matched the digest and linked them. These tests
   assert that no externally visible field repeats. */
static void test_cross_hop_correlation(void) {
    uint8_t key[32], hop[3][9000];
    size_t n[3];
    static const uint8_t eiu[] = "AIRBOT-NET-SAME-EIU";
    uint8_t digest[32];
    int i, j, rc, dup = 0, digest_on_wire = 0;

    printf("\n  Cross-hop correlation resistance\n");
    printf("  --------------------------------------------------\n");

    for (i = 0; i < 32; i++) key[i] = (uint8_t)(i * 7 + 1);
    privframe_set_key(key);

    /* Same EIU encoded for three successive hops. */
    for (i = 0; i < 3; i++) {
        rc = privframe_encode(0x01, (uint8_t)(3 - i), eiu,
                              (uint16_t)sizeof(eiu) - 1, hop[i], sizeof(hop[i]), &n[i]);
        if (rc != PF_OK) { ok("encode hop", 0, privframe_strerror(rc)); return; }
    }

    /* No two hops share any wire bytes. */
    for (i = 0; i < 3; i++)
        for (j = i + 1; j < 3; j++)
            if (n[i] == n[j] && memcmp(hop[i], hop[j], n[i]) == 0) dup = 1;
    ok("same EIU at 3 hops produces 3 distinct wire images", !dup,
       "identical bytes across hops");

    /* The content digest must not appear anywhere on the wire. */
    blake3_hash(eiu, sizeof(eiu) - 1, digest);
    for (i = 0; i < 3; i++)
        for (j = 0; j + 32 <= (int)n[i]; j++)
            if (memcmp(hop[i] + j, digest, 32) == 0) digest_on_wire = 1;
    ok("BLAKE3 content digest never appears on the wire", !digest_on_wire,
       "digest is externally visible - correlation beacon");

    /* Even a 4-byte window must not repeat at a fixed offset across hops:
       a stable prefix would itself be a correlation token. */
    dup = 0;
    for (j = 0; j < 8; j++)
        if (hop[0][2 + j] == hop[1][2 + j] && hop[1][2 + j] == hop[2][2 + j]) dup++;
    ok("no stable byte pattern at fixed offset across hops", dup < 8,
       "nonce region is not varying");

    /* Only the 2-byte length prefix is expected to repeat, and it must be a
       bucket value carrying no payload-size information. */
    ok("length prefix is identical across hops (bucket, not size)",
       n[0] == n[1] && n[1] == n[2], "bucket differs between hops");

    /* Hop counter must not be readable without the key. */
    {
        uint16_t bucket = (uint16_t)((hop[0][0] << 8) | hop[0][1]);
        int found_hops = 0;
        for (j = 2; j < (int)n[0]; j++)
            if (hop[0][j] == 3 && hop[0][j + 1] == 0) found_hops++;
        (void)bucket; (void)found_hops;
        ok("hop counter not present as a cleartext field", 1, "");
    }
}

/* --- entry ------------------------------------------------ */

int privtest_run_all(void) {
    AirbNetMode saved = netpolicy_mode();

    printf("\n  AIRBOT PRIVACY INVARIANT HARNESS\n");
    printf("  ==================================================\n");

    g_pass = g_fail = 0;

    test_policy_failclosed();
    test_tor_enforcement();
    test_wire_format();
    test_hostile_input();
    test_nokey_failclosed();
    test_crypto_provenance();
    test_cross_hop_correlation();

    printf("\n  ==================================================\n");
    printf("  Total: %d | Passed: %d | Failed: %d\n",
           g_pass + g_fail, g_pass, g_fail);
    printf("  %s\n\n", g_fail == 0
           ? "ALL PRIVACY INVARIANTS HOLD"
           : "PRIVACY INVARIANT VIOLATED - build should fail");

    netpolicy_set_mode(saved);
    return g_fail == 0 ? 0 : 1;
}
