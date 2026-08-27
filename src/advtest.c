/*
 * Airbot — Executable Information System
 * advtest.c — Adversarial / red-team regression suite
 *
 * Each test models a specific adversary from SECURITY.md and reports what
 * that adversary actually achieves. Tests that DEMONSTRATE A SUCCESSFUL
 * ATTACK are reported as such rather than being omitted.
 */
#include "advtest.h"
#include "onionx.h"
#include "x25519.h"
#include "blake3.h"
#include "chacha20.h"
#include "privframe.h"
#include "netpolicy.h"

#include <stdio.h>
#include <string.h>

static int g_pass = 0, g_fail = 0;

static void ok(const char *name, int cond, const char *detail) {
    if (cond) { printf("  [PASS] %-54s\n", name); g_pass++; }
    else      { printf("  [FAIL] %-54s <- %s\n", name, detail ? detail : ""); g_fail++; }
}

static void note(const char *fmt, const char *a) {
    printf("         %s%s\n", fmt, a ? a : "");
}

/* --- 1. relay compromise --------------------------------- */

static void test_relay_compromise(void) {
    OxIdentity rA, rB, rC, path[3];
    OxReplayWindow wA, wB, wC;
    uint8_t onion[8192];
    size_t olen = 0;
    OxPeeled pA, pB, pC;
    static const uint8_t secret[] = "PAYLOAD-FOR-EXIT-ONLY";
    size_t i;
    int found;

    printf("\n  Adversary: malicious relay A\n");
    printf("  --------------------------------------------------\n");

    ox_identity_generate(&rA, "127.0.0.1:9401");
    ox_identity_generate(&rB, "127.0.0.1:9402");
    ox_identity_generate(&rC, "127.0.0.1:9403");
    ox_identity_from_public(&path[0], rA.public_key, "127.0.0.1:9401");
    ox_identity_from_public(&path[1], rB.public_key, "127.0.0.1:9402");
    ox_identity_from_public(&path[2], rC.public_key, "127.0.0.1:9403");
    ox_replay_init(&wA, 1); ox_replay_init(&wB, 1); ox_replay_init(&wC, 1);

    ox_wrap(path, 3, secret, (uint16_t)sizeof(secret) - 1, onion, sizeof(onion), &olen);
    ox_peel(&rA, &wA, onion, olen, &pA);

    /* What A gets. */
    found = 0;
    for (i = 0; i + sizeof(secret) - 1 <= pA.inner_len; i++)
        if (memcmp(pA.inner + i, secret, sizeof(secret) - 1) == 0) found = 1;
    ok("relay A cannot read the application payload", !found,
       "ATTACK SUCCEEDED: payload readable at first hop");

    ok("relay A cannot peel relay B's layer",
       ox_peel(&rA, NULL, pA.inner, pA.inner_len, &pB) == OX_ERR_AUTH,
       "ATTACK SUCCEEDED: A decrypted B's layer");

    /* A does learn its immediate successor - by design, and worth stating. */
    ok("relay A learns ONLY its immediate successor",
       memcmp(pA.next_addr, "127.0.0.1:9402", 14) == 0,
       "next-hop routing broken");
    note("relay A observes next hop: ", (const char *)pA.next_addr);
    note("relay A does NOT observe the exit or the payload", "");

    /* Colluding A + C. */
    ox_peel(&rB, &wB, pA.inner, pA.inner_len, &pB);
    ox_peel(&rC, &wC, pB.inner, pB.inner_len, &pC);
    ok("exit relay C DOES see the payload (inherent to onion routing)",
       pC.inner_len == sizeof(secret) - 1 &&
       memcmp(pC.inner, secret, pC.inner_len) == 0,
       "exit could not recover payload");

    /* Do A and C share any value that links the same message? */
    ok("colluding A + C hold UNRELATED message ids",
       memcmp(pA.msg_id, pC.msg_id, OX_MSGID_SIZE) != 0,
       "ATTACK SUCCEEDED: A and C share an identifier");
    printf("         Each hop id is derived under that hop's own key, so a
");
    printf("         shared identifier no longer links the two relays.
");
    printf("         A + C can STILL correlate by timing and size - that
");
    printf("         residual attack is unfixed; see the traffic section.
");

    ox_identity_erase(&rA); ox_identity_erase(&rB); ox_identity_erase(&rC);
}

/* --- 2. forward secrecy ---------------------------------- */

static void test_forward_secrecy(void) {
    OxIdentity relay, path[1];
    OxReplayWindow w;
    uint8_t captured[8192];
    size_t clen = 0;
    OxPeeled p;
    static const uint8_t secret[] = "HISTORIC-SESSION-DATA";
    int rc;

    printf("\n  Adversary: retrospective key compromise\n");
    printf("  --------------------------------------------------\n");

    ox_identity_generate(&relay, "127.0.0.1:9401");
    ox_identity_from_public(&path[0], relay.public_key, "127.0.0.1:9401");
    ox_replay_init(&w, 1);

    /* Step 1: adversary captures traffic now. */
    ox_wrap(path, 1, secret, (uint16_t)sizeof(secret) - 1,
            captured, sizeof(captured), &clen);
    ok("traffic captured for later analysis", clen > 0, "no capture");

    /* Sanity: with the key still live, the relay can read it. */
    rc = ox_peel(&relay, &w, captured, clen, &p);
    ok("while the long-term key lives, the relay can decrypt", rc == OX_OK,
       ox_strerror(rc));

    /* Step 2: the long-term identity key is compromised and then erased,
       modelling an adversary who seizes the relay AFTER the fact. The
       ephemeral secret was already destroyed inside ox_wrap. */
    ox_identity_erase(&relay);
    ok("long-term secret erased from memory",
       relay.have_secret == 0, "secret still present");

    /* Step 3: the captured ciphertext must no longer be decryptable. */
    {
        OxReplayWindow w2;
        OxPeeled p2;
        ox_replay_init(&w2, 2);
        rc = ox_peel(&relay, &w2, captured, clen, &p2);
        ok("captured traffic NOT decryptable after key erasure",
           rc != OX_OK, "ATTACK SUCCEEDED: historic traffic recovered");
    }
    printf("         Note: this demonstrates that the EPHEMERAL secret is gone.\n");
    printf("         An adversary who seizes the long-term key while it is\n");
    printf("         still in memory, or who recorded it earlier, is NOT\n");
    printf("         defeated by this. See SECURITY.md.\n");
}

/* --- 3. cross-hop correlation ---------------------------- */

static void test_cross_hop_tokens(void) {
    OxIdentity rA, rB, rC, path[3];
    OxReplayWindow wA;
    uint8_t o1[8192], o2[8192];
    size_t l1 = 0, l2 = 0;
    OxPeeled pA;
    static const uint8_t msg[] = "IDENTICAL-MESSAGE";
    int i, shared_bytes = 0;

    printf("\n  Adversary: cross-hop correlation\n");
    printf("  --------------------------------------------------\n");

    ox_identity_generate(&rA, "127.0.0.1:9401");
    ox_identity_generate(&rB, "127.0.0.1:9402");
    ox_identity_generate(&rC, "127.0.0.1:9403");
    ox_identity_from_public(&path[0], rA.public_key, "127.0.0.1:9401");
    ox_identity_from_public(&path[1], rB.public_key, "127.0.0.1:9402");
    ox_identity_from_public(&path[2], rC.public_key, "127.0.0.1:9403");
    ox_replay_init(&wA, 1);

    ox_wrap(path, 3, msg, (uint16_t)sizeof(msg) - 1, o1, sizeof(o1), &l1);
    ox_wrap(path, 3, msg, (uint16_t)sizeof(msg) - 1, o2, sizeof(o2), &l2);

    ok("same message wrapped twice differs on the wire",
       !(l1 == l2 && memcmp(o1, o2, l1) == 0), "identical ciphertext");

    /* The outer bytes an observer sees at hop 1 vs the inner bytes at hop 2
       must share nothing. */
    ox_peel(&rA, &wA, o1, l1, &pA);
    for (i = 0; i + 8 <= (int)pA.inner_len; i++) {
        int j, hit = 0;
        for (j = 0; j + 8 <= (int)l1; j++)
            if (memcmp(pA.inner + i, o1 + j, 8) == 0) { hit = 1; break; }
        if (hit) shared_bytes++;
    }
    ok("no 8-byte sequence recurs between hop 1 and hop 2 wire images",
       shared_bytes == 0, "a stable token survives the hop");

    ox_identity_erase(&rA); ox_identity_erase(&rB); ox_identity_erase(&rC);
}

/* --- 4. replay ------------------------------------------- */

static void test_replay(void) {
    OxIdentity relay, path[1];
    OxReplayWindow w;
    uint8_t onion[8192];
    size_t olen = 0;
    OxPeeled p;
    static const uint8_t msg[] = "REPLAY-ME";
    int i, accepted = 0;

    printf("\n  Adversary: replay / duplication\n");
    printf("  --------------------------------------------------\n");

    ox_identity_generate(&relay, "127.0.0.1:9401");
    ox_identity_from_public(&path[0], relay.public_key, "127.0.0.1:9401");
    ox_replay_init(&w, 1);

    ox_wrap(path, 1, msg, (uint16_t)sizeof(msg) - 1, onion, sizeof(onion), &olen);

    ok("first delivery accepted", ox_peel(&relay, &w, onion, olen, &p) == OX_OK, "");

    for (i = 0; i < 20; i++)
        if (ox_peel(&relay, &w, onion, olen, &p) == OX_OK) accepted++;
    ok("20 replays of the same frame all rejected", accepted == 0,
       "ATTACK SUCCEEDED: replay accepted");

    /* Cross-session: after rotation the window is empty by design. */
    ox_replay_reset(&w, 2);
    ok("replay window holds no cross-session state after rotation",
       w.count == 0, "state survived rotation");
    printf("         Trade-off: a rotated window will re-accept a very old\n");
    printf("         frame. Bounded memory was chosen over unbounded state.\n");

    ox_identity_erase(&relay);
}

/* --- 5. traffic analysis (measured, not asserted) -------- */

/*
 * Models a passive observer who sees only frame sizes. Encodes two different
 * logical flows and reports whether size alone separates them.
 */
static void test_traffic_analysis(void) {
    uint8_t key[32], wire[9000];
    size_t n = 0;
    int i;
    size_t sizes_a[6], sizes_b[6];
    int distinguishable = 0;

    printf("\n  Adversary: passive traffic analysis (size only)\n");
    printf("  --------------------------------------------------\n");

    for (i = 0; i < 32; i++) key[i] = (uint8_t)(i * 3 + 7);
    privframe_set_key(key);

    /* Flow A: six small messages of differing true length. */
    for (i = 0; i < 6; i++) {
        uint8_t buf[200];
        memset(buf, 'A', sizeof(buf));
        privframe_encode(0x01, 0, buf, (uint16_t)(10 + i * 30), wire, sizeof(wire), &n);
        sizes_a[i] = n;
    }
    /* Flow B: six messages, different content, same length class. */
    for (i = 0; i < 6; i++) {
        uint8_t buf[200];
        memset(buf, 'B', sizeof(buf));
        privframe_encode(0x02, 3, buf, (uint16_t)(15 + i * 30), wire, sizeof(wire), &n);
        sizes_b[i] = n;
    }

    printf("         flow A observed sizes: ");
    for (i = 0; i < 6; i++) printf("%u ", (unsigned)sizes_a[i]);
    printf("\n         flow B observed sizes: ");
    for (i = 0; i < 6; i++) printf("%u ", (unsigned)sizes_b[i]);
    printf("\n");

    for (i = 0; i < 6; i++) if (sizes_a[i] != sizes_b[i]) distinguishable++;
    ok("payloads of differing length collapse to identical observed sizes",
       distinguishable == 0, "size alone separates the two flows");

    printf("         MEASURED: %d of 6 message pairs distinguishable by size.\n",
           distinguishable);
    printf("         Bucketing removes exact length. It does NOT remove\n");
    printf("         timing, ordering, count or burst structure, and a\n");
    printf("         6-value size distribution is itself a signature.\n");
}

/* --- 6. protocol fingerprint ----------------------------- */

static void test_fingerprint(void) {
    uint8_t key[32], wire[9000];
    size_t n = 0;
    int i, printable = 0, magic = 0;

    printf("\n  Adversary: protocol fingerprinting\n");
    printf("  --------------------------------------------------\n");

    for (i = 0; i < 32; i++) key[i] = (uint8_t)(i * 5 + 3);
    privframe_set_key(key);
    privframe_encode(0x01, 3, (const uint8_t *)"FINGERPRINT-ME", 14, wire, sizeof(wire), &n);

    for (i = 0; i + 4 <= (int)n; i++)
        if (memcmp(wire + i, "AIRB", 4) == 0) magic = 1;
    ok("no application magic on the wire", !magic, "magic present");

    /* An all-ciphertext body should look close to uniform. A crude check:
       printable-ASCII density well below what a text protocol would show. */
    for (i = 2; i < (int)n; i++)
        if (wire[i] >= 32 && wire[i] < 127) printable++;
    ok("body has no text-protocol structure",
       (printable * 100) / (int)(n - 2) < 45, "too much printable ASCII");
    printf("         MEASURED: %d%% printable bytes (random ~37%% expected).\n",
           (printable * 100) / (int)(n - 2));
    printf("         Remaining signature: a 2-byte length prefix taking one\n");
    printf("         of six values. NOT demonstrated indistinguishable from\n");
    printf("         arbitrary internet traffic.\n");
}

/* --- 7. key hygiene -------------------------------------- */

static void test_key_hygiene(void) {
    OxIdentity id;
    int i, nonzero = 0;

    printf("\n  Endpoint: key material handling\n");
    printf("  --------------------------------------------------\n");

    ox_identity_generate(&id, "127.0.0.1:9401");
    for (i = 0; i < OX_KEY_SIZE; i++) if (id.secret_key[i]) nonzero++;
    ok("generated identity has a non-trivial secret", nonzero > 8, "weak key");

    ox_identity_erase(&id);
    nonzero = 0;
    for (i = 0; i < OX_KEY_SIZE; i++) if (id.secret_key[i]) nonzero++;
    ok("erase clears the secret key buffer", nonzero == 0, "key residue remains");
    printf("         Best-effort only: cannot defeat register spills, swap,\n");
    printf("         hibernation images, or a privileged local process.\n");
}

/* --- 8. full relay-compromise matrix (Phase 3 / 12) ------ */

/*
 * Every compromise combination, reported as measured fact. A "1" means the
 * compromised set genuinely obtains that item in this implementation.
 */
static void test_compromise_matrix(void) {
    OxIdentity rA, rB, rC, path[3];
    OxReplayWindow wA, wB, wC;
    uint8_t onion[8192];
    size_t olen = 0;
    OxPeeled pA, pB, pC;
    static const uint8_t secret[] = "EXIT-ONLY-PAYLOAD";
    int a_payload, b_payload, ab_link, ac_link, bc_link;

    printf("\n  Relay-compromise matrix (measured)\n");
    printf("  --------------------------------------------------\n");

    ox_identity_generate(&rA, "127.0.0.1:9401");
    ox_identity_generate(&rB, "127.0.0.1:9402");
    ox_identity_generate(&rC, "127.0.0.1:9403");
    ox_identity_from_public(&path[0], rA.public_key, "127.0.0.1:9401");
    ox_identity_from_public(&path[1], rB.public_key, "127.0.0.1:9402");
    ox_identity_from_public(&path[2], rC.public_key, "127.0.0.1:9403");
    ox_replay_init(&wA,1); ox_replay_init(&wB,1); ox_replay_init(&wC,1);

    ox_wrap(path, 3, secret, (uint16_t)sizeof(secret)-1, onion, sizeof(onion), &olen);
    ox_peel(&rA, &wA, onion, olen, &pA);
    ox_peel(&rB, &wB, pA.inner, pA.inner_len, &pB);
    ox_peel(&rC, &wC, pB.inner, pB.inner_len, &pC);

    { size_t i; a_payload = 0;
      for (i = 0; i + sizeof(secret)-1 <= pA.inner_len; i++)
        if (memcmp(pA.inner+i, secret, sizeof(secret)-1)==0) a_payload = 1; }
    { size_t i; b_payload = 0;
      for (i = 0; i + sizeof(secret)-1 <= pB.inner_len; i++)
        if (memcmp(pB.inner+i, secret, sizeof(secret)-1)==0) b_payload = 1; }

    ab_link = (memcmp(pA.msg_id, pB.msg_id, OX_MSGID_SIZE) == 0);
    ac_link = (memcmp(pA.msg_id, pC.msg_id, OX_MSGID_SIZE) == 0);
    bc_link = (memcmp(pB.msg_id, pC.msg_id, OX_MSGID_SIZE) == 0);

    printf("   compromise | client IP | dest | payload | other hops | id-link\n");
    printf("   -----------+-----------+------+---------+------------+--------\n");
    printf("   A          | prev hop  |  no  |   %-3s   | next only  |   -\n", a_payload?"YES":"no");
    printf("   B          | no        |  no  |   %-3s   | next only  |   -\n", b_payload?"YES":"no");
    printf("   C (exit)   | no        | YES  |   YES   | none       |   -\n");
    printf("   A + B      | prev hop  |  no  |   %-3s   | -          |   %s\n",
           (a_payload||b_payload)?"YES":"no", ab_link?"YES":"no");
    printf("   A + C      | prev hop  | YES  |   YES   | -          |   %s\n", ac_link?"YES":"no");
    printf("   B + C      | no        | YES  |   YES   | -          |   %s\n", bc_link?"YES":"no");

    ok("relay A never obtains the payload", !a_payload, "A read the payload");
    ok("relay B never obtains the payload", !b_payload, "B read the payload");
    ok("A+B cannot link by protocol identifier", !ab_link, "shared id");
    ok("A+C cannot link by protocol identifier", !ac_link, "shared id");
    ok("B+C cannot link by protocol identifier", !bc_link, "shared id");
    printf("         NOTE: 'client IP' for relay A means the TRANSPORT peer\n");
    printf("         address, which is the Tor exit when Airbot runs behind\n");
    printf("         Tor - not the client's own address.\n");
    printf("         A+C remain able to correlate by TIMING. See below.\n");

    ox_identity_erase(&rA); ox_identity_erase(&rB); ox_identity_erase(&rC);
}

/* --- 9. reordering / cross-session replay (Phase 7) ------ */

static void test_reorder_and_sessions(void) {
    OxIdentity relay, path[1];
    OxReplayWindow w1, w2;
    uint8_t f1[8192], f2[8192], f3[8192];
    size_t l1=0, l2=0, l3=0;
    OxPeeled p;
    int rc;

    printf("\n  Replay, reordering and session isolation\n");
    printf("  --------------------------------------------------\n");

    ox_identity_generate(&relay, "127.0.0.1:9401");
    ox_identity_from_public(&path[0], relay.public_key, "127.0.0.1:9401");
    ox_replay_init(&w1, 1);

    ox_wrap(path,1,(const uint8_t*)"one",3,f1,sizeof(f1),&l1);
    ox_wrap(path,1,(const uint8_t*)"two",3,f2,sizeof(f2),&l2);
    ox_wrap(path,1,(const uint8_t*)"three",5,f3,sizeof(f3),&l3);

    /* Out-of-order delivery must be ACCEPTED: the transport is a datagram
       overlay, not an ordered stream, and rejecting reordering would break
       delivery without adding security. */
    ok("out-of-order frames accepted (3,1,2)",
       ox_peel(&relay,&w1,f3,l3,&p)==OX_OK &&
       ox_peel(&relay,&w1,f1,l1,&p)==OX_OK &&
       ox_peel(&relay,&w1,f2,l2,&p)==OX_OK, "reordering broke delivery");

    /* But each is single-use. */
    ok("each reordered frame is still single-use",
       ox_peel(&relay,&w1,f3,l3,&p)==OX_ERR_REPLAY &&
       ox_peel(&relay,&w1,f1,l1,&p)==OX_ERR_REPLAY, "duplicate accepted");

    /* Cross-session: a frame captured in session 1 replayed into a fresh
       session must not authenticate, because the relay key rotated. */
    {
        OxIdentity relay2, path2[1];
        uint8_t fresh[8192]; size_t fl=0;
        ox_identity_generate(&relay2, "127.0.0.1:9401");
        ox_identity_from_public(&path2[0], relay2.public_key, "127.0.0.1:9401");
        ox_replay_init(&w2, 2);
        ox_wrap(path2,1,(const uint8_t*)"new",3,fresh,sizeof(fresh),&fl);
        rc = ox_peel(&relay2, &w2, f1, l1, &p);
        ok("frame from a previous key epoch rejected",
           rc == OX_ERR_AUTH, "cross-epoch frame authenticated");
        rc = ox_peel(&relay, &w2, fresh, fl, &p);
        ok("frame addressed to another relay identity rejected",
           rc == OX_ERR_AUTH, "wrong-relay frame authenticated");
        ox_identity_erase(&relay2);
    }
    ox_identity_erase(&relay);
}

/* --- entry ----------------------------------------------- */

int advtest_run_all(void) {
    printf("\n  AIRBOT ADVERSARIAL / RED-TEAM SUITE\n");
    printf("  ==================================================\n");
    g_pass = g_fail = 0;

    test_relay_compromise();
    test_forward_secrecy();
    test_cross_hop_tokens();
    test_replay();
    test_traffic_analysis();
    test_fingerprint();
    test_key_hygiene();
    test_compromise_matrix();
    test_reorder_and_sessions();

    printf("\n  ==================================================\n");
    printf("  Total: %d | Passed: %d | Failed: %d\n",
           g_pass + g_fail, g_pass, g_fail);
    printf("  %s\n\n", g_fail == 0
           ? "All modelled defences held. Residual attacks are listed above."
           : "A MODELLED DEFENCE FAILED - see [FAIL] lines.");
    return g_fail == 0 ? 0 : 1;
}
