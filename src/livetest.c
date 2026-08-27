/*
 * Airbot — Executable Information System
 * livetest.c — Live socket integration test of the production onion path
 *
 * A passing onion-test does not protect a socket. This test drives the REAL
 * production functions (airbchan_send / airbchan_recv / airbchan_forward)
 * over REAL loopback sockets through a three-relay chain, and inspects the
 * bytes that actually cross the wire.
 *
 * It exists because a socket capture previously showed the plaintext AIRB
 * frame on the production path while onionx and privframe — both fully
 * tested — were never called by any live command.
 */
#include "livetest.h"
#include "airbchan.h"
#include "onionx.h"
#include "transport.h"
#include "netpolicy.h"
#include "privframe.h"

#include <stdio.h>
#include <string.h>
#include <windows.h>

static int g_pass = 0, g_fail = 0;

static void ok(const char *name, int cond, const char *detail) {
    if (cond) { printf("  [PASS] %-54s\n", name); g_pass++; }
    else      { printf("  [FAIL] %-54s <- %s\n", name, detail ? detail : ""); g_fail++; }
}

/* The plaintext an observer must never see on any link. */
static const char SECRET[] = "LIVE-SOCKET-SECRET-PAYLOAD";

/* Captured wire bytes per link, filled by the relay threads. */
#define MAX_CAP 9000
typedef struct {
    uint8_t bytes[MAX_CAP];
    int     len;
} LinkCapture;

static LinkCapture g_cap[3];

typedef struct {
    uint16_t     listen_port;
    uint16_t     next_port;      /* 0 = exit hop */
    OxIdentity  *me;
    OxIdentity  *next_id;   /* public descriptor of the next relay */
    int          link_index;
    int          ready;
    int          done;
    int          peel_rc;
    int          is_exit;
    uint8_t      exit_payload[4096];
    uint16_t     exit_len;
    OxReplayWindow window;
} RelayCtx;

/* One relay: accept, capture the raw link bytes, peel exactly one layer,
   forward the remainder or surface the payload. */
static DWORD WINAPI relay_thread(LPVOID arg) {
    RelayCtx *ctx = (RelayCtx *)arg;
    AirbConn listener, peer;
    OxPeeled peeled;
    int rc;

    if (transport_listen(ctx->listen_port, &listener) != AIRB_OK) {
        ctx->done = 1; ctx->peel_rc = -999; return 0;
    }
    ctx->ready = 1;

    if (transport_accept(&listener, &peer) != AIRB_OK) {
        transport_close(&listener); ctx->done = 1; ctx->peel_rc = -998; return 0;
    }
    transport_set_timeout(&peer, 5000);

    /* Capture what actually arrived on this link, before any decryption. */
    {
        int n = transport_recv_raw(&peer, g_cap[ctx->link_index].bytes, MAX_CAP);
        if (n > 0) g_cap[ctx->link_index].len = n;
    }

    /* Peel using the captured bytes: bucket index byte, then the onion. */
    {
        const uint8_t *w = g_cap[ctx->link_index].bytes;
        int wl = g_cap[ctx->link_index].len;
        if (wl < 2) { ctx->peel_rc = -997; goto out; }
        {
            uint8_t onion[8192];
            uint16_t olen = 0;
            if (wl != AC_ENVELOPE_WIRE) { ctx->peel_rc = -996; goto out; }
            if (airbchan_open_buf(ctx->me, w, onion, &olen) != AC_OK) {
                ctx->peel_rc = -995; goto out;
            }
            rc = ox_peel(ctx->me, &ctx->window, onion, (size_t)olen, &peeled);
        }
        ctx->peel_rc = rc;
        if (rc != OX_OK) goto out;
        ctx->is_exit = peeled.is_exit;

        if (peeled.is_exit) {
            uint16_t l = 0;
            if (airbchan_exit_payload(&peeled, ctx->exit_payload,
                                      sizeof(ctx->exit_payload), &l) == AC_OK) {
                ctx->exit_len = l;
            }
        } else if (ctx->next_port) {
            AirbConn fwd;
            if (transport_dial("127.0.0.1", ctx->next_port, &fwd) == AIRB_OK) {
                airbchan_forward_to(&fwd, ctx->next_id,
                                    peeled.inner, peeled.inner_len);
                transport_close(&fwd);
            }
        }
    }
out:
    transport_close(&peer);
    transport_close(&listener);
    ctx->done = 1;
    return 0;
}

static int contains(const uint8_t *h, int hn, const void *n_, int nn) {
    int i;
    if (hn < nn) return 0;
    for (i = 0; i + nn <= hn; i++)
        if (memcmp(h + i, n_, (size_t)nn) == 0) return 1;
    return 0;
}

int livetest_run_all(void) {
    OxIdentity rA, rB, rC, path[3];
    RelayCtx cA, cB, cC;
    HANDLE hA, hB, hC;
    AirbConn out;
    int i, waited;
    AirbNetMode saved = netpolicy_mode();

    printf("\n  AIRBOT LIVE SOCKET INTEGRATION TEST\n");
    printf("  ==================================================\n");
    printf("  Drives the PRODUCTION path over real sockets:\n");
    printf("  client -> relay A -> relay B -> relay C(exit)\n\n");

    /* Relays are infrastructure and must be allowed to listen. */
    netpolicy_set_mode(AIRB_MODE_RELAY);

    memset(g_cap, 0, sizeof(g_cap));
    ox_identity_generate(&rA, "127.0.0.1:9801");
    ox_identity_generate(&rB, "127.0.0.1:9802");
    ox_identity_generate(&rC, "127.0.0.1:9803");
    ox_identity_from_public(&path[0], rA.public_key, "127.0.0.1:9801");
    ox_identity_from_public(&path[1], rB.public_key, "127.0.0.1:9802");
    ox_identity_from_public(&path[2], rC.public_key, "127.0.0.1:9803");

    memset(&cA, 0, sizeof(cA)); memset(&cB, 0, sizeof(cB)); memset(&cC, 0, sizeof(cC));
    cA.listen_port = 9801; cA.next_port = 9802; cA.me = &rA; cA.next_id = &path[1]; cA.link_index = 0;
    cB.listen_port = 9802; cB.next_port = 9803; cB.me = &rB; cB.next_id = &path[2]; cB.link_index = 1;
    cC.listen_port = 9803; cC.next_port = 0;    cC.me = &rC; cC.next_id = 0;        cC.link_index = 2;
    ox_replay_init(&cA.window, 1);
    ox_replay_init(&cB.window, 1);
    ox_replay_init(&cC.window, 1);

    hC = CreateThread(NULL, 0, relay_thread, &cC, 0, NULL);
    hB = CreateThread(NULL, 0, relay_thread, &cB, 0, NULL);
    hA = CreateThread(NULL, 0, relay_thread, &cA, 0, NULL);

    for (waited = 0; waited < 200 && !(cA.ready && cB.ready && cC.ready); waited++)
        Sleep(10);
    ok("three relays listening on real sockets",
       cA.ready && cB.ready && cC.ready, "relays failed to bind");

    /* Client sends through the production API. */
    if (transport_dial("127.0.0.1", 9801, &out) != AIRB_OK) {
        ok("client connected to relay A", 0, "dial failed");
        goto finish;
    }
    ok("client connected to relay A", 1, "");
    {
        int rc = airbchan_send(&out, path, 3,
                               (const uint8_t *)SECRET, (uint16_t)(sizeof(SECRET) - 1));
        ok("airbchan_send accepted by production path", rc == AC_OK,
           airbchan_strerror(rc));
    }
    transport_close(&out);

    for (waited = 0; waited < 500 && !(cA.done && cB.done && cC.done); waited++)
        Sleep(10);

    /* --- what the chain did --- */
    ok("relay A peeled exactly one layer", cA.peel_rc == OX_OK, ox_strerror(cA.peel_rc));
    ok("relay B peeled exactly one layer", cB.peel_rc == OX_OK, ox_strerror(cB.peel_rc));
    ok("relay C peeled exactly one layer", cC.peel_rc == OX_OK, ox_strerror(cC.peel_rc));
    ok("relay A is NOT the exit", !cA.is_exit, "A believed it was the exit");
    ok("relay B is NOT the exit", !cB.is_exit, "B believed it was the exit");
    ok("relay C IS the exit", cC.is_exit != 0, "C did not see the exit flag");
    ok("exit recovered the exact application payload",
       cC.exit_len == sizeof(SECRET) - 1 &&
       memcmp(cC.exit_payload, SECRET, cC.exit_len) == 0, "payload mismatch");

    /* --- socket-level evidence: the bytes that really crossed each link --- */
    printf("\n  Socket-level evidence (bytes captured on each link)\n");
    printf("  --------------------------------------------------\n");
    for (i = 0; i < 3; i++) {
        int j, printable = 0;
        printf("  link %d (%s): %d bytes, first 16: ",
               i + 1, i == 0 ? "client->A" : (i == 1 ? "A->B" : "B->C"),
               g_cap[i].len);
        for (j = 0; j < 16 && j < g_cap[i].len; j++) printf("%02X", g_cap[i].bytes[j]);
        for (j = 1; j < g_cap[i].len; j++)
            if (g_cap[i].bytes[j] >= 32 && g_cap[i].bytes[j] < 127) printable++;
        printf("\n           printable: %d%%\n",
               g_cap[i].len > 1 ? (printable * 100) / (g_cap[i].len - 1) : 0);
    }
    printf("\n");

    for (i = 0; i < 3; i++) {
        char nm[64];
        sprintf(nm, "link %d carries no plaintext payload", i + 1);
        ok(nm, !contains(g_cap[i].bytes, g_cap[i].len, SECRET, (int)sizeof(SECRET) - 1),
           "PLAINTEXT PAYLOAD ON THE WIRE");
        sprintf(nm, "link %d carries no AIRB magic", i + 1);
        ok(nm, !contains(g_cap[i].bytes, g_cap[i].len, "AIRB", 4),
           "legacy magic on the wire");
        /* A 2-byte pattern occurs by chance in ~1.57% of 1084-byte
           ciphertexts (measured), i.e. ~4.65% across three links, so a
           full-buffer scan for the EIU magic is statistically meaningless
           and produced a genuinely flaky failure. What matters is whether a
           real EIU HEADER sits at the structurally significant offset (the
           start of the ciphertext region). The 4-byte AIRB scan and the
           plaintext-payload scan above remain sound: the 4-byte collision
           rate was measured at 0/20000. */
        sprintf(nm, "link %d has no EIU header at the payload offset", i + 1);
        ok(nm, !(g_cap[i].len > 45 &&
                 g_cap[i].bytes[44] == 0xAB && g_cap[i].bytes[45] == 0x01),
           "EIU header at ciphertext offset 0");
    }

    /* Each link is bucketed, and no two links share a wire image. */
    {
        /* HOP-INDEX CONCEALMENT: every link must be the SAME constant size,
           so the wire no longer reveals how many layers remain. */
        ok("every link is the constant envelope size",
           g_cap[0].len == AC_ENVELOPE_WIRE &&
           g_cap[1].len == AC_ENVELOPE_WIRE &&
           g_cap[2].len == AC_ENVELOPE_WIRE, "link sizes differ");
        ok("frame size does NOT shrink along the chain (hop index hidden)",
           g_cap[0].len == g_cap[1].len && g_cap[1].len == g_cap[2].len,
           "HOP INDEX LEAKS VIA SIZE");
    }
    ok("no two links share the same wire image",
       !(g_cap[0].len == g_cap[1].len &&
         memcmp(g_cap[0].bytes, g_cap[1].bytes, (size_t)g_cap[0].len) == 0),
       "identical bytes on consecutive links");

    /* --- negative cases against the live peel --- */
    {
        OxPeeled p;
        OxReplayWindow w;
        uint8_t tampered[MAX_CAP];
        int rc;

        ox_replay_init(&w, 9);
        /* wrong relay key */
        {
            uint8_t on[8192]; uint16_t ol = 0;
            rc = airbchan_open_buf(&rB, g_cap[0].bytes, on, &ol);
            ok("wrong relay key cannot open the captured envelope",
               rc != AC_OK, "wrong key opened the envelope");
        }

        /* modified ciphertext */
        {
            uint8_t on[8192]; uint16_t ol = 0;
            memcpy(tampered, g_cap[0].bytes, (size_t)g_cap[0].len);
            tampered[60] ^= 0x01;
            rc = airbchan_open_buf(&rA, tampered, on, &ol);
            ok("modified ciphertext rejected on captured live frame",
               rc != AC_OK, "tampered frame accepted");
            /* padding is inside the AEAD: touching it must also fail */
            memcpy(tampered, g_cap[0].bytes, (size_t)g_cap[0].len);
            tampered[AC_ENVELOPE_WIRE - 20] ^= 0x01;
            rc = airbchan_open_buf(&rA, tampered, on, &ol);
            ok("modified outer padding rejected (padding is authenticated)",
               rc != AC_OK, "padding not covered by the tag");
        }

        /* replay into the relay's own window */
        {
            uint8_t on[8192]; uint16_t ol = 0;
            if (airbchan_open_buf(&rA, g_cap[0].bytes, on, &ol) == AC_OK) {
                rc = ox_peel(&rA, &cA.window, on, (size_t)ol, &p);
                ok("replay of the live frame rejected by relay A's window",
                   rc == OX_ERR_REPLAY, "replay accepted");
            } else ok("replay of the live frame rejected by relay A's window",
                      0, "could not reopen envelope");
        }

        /* cross-session: a fresh relay identity must not accept it */
        {
            OxIdentity rA2;
            OxReplayWindow w2;
            ox_identity_generate(&rA2, "127.0.0.1:9801");
            ox_replay_init(&w2, 11);
            uint8_t on[8192]; uint16_t ol = 0;
            rc = airbchan_open_buf(&rA2, g_cap[0].bytes, on, &ol);
            ok("frame from a previous key epoch rejected",
               rc != AC_OK, "cross-session frame accepted");
            ox_identity_erase(&rA2);
        }
    }

    /* --- bypass must be impossible in privacy mode --- */
    {
        AirbConn c;
        int rc;
        netpolicy_set_mode(AIRB_MODE_PRIVACY);
        memset(&c, 0, sizeof(c));
        c.valid = 1; c.sock = 0;
        rc = transport_send_frame(&c, 0x01, 3, (const uint8_t *)"x", 1);
        ok("legacy cleartext framer refused in privacy mode",
           rc == AIRB_ERR_POLICY, "cleartext framer still reachable");
        netpolicy_set_mode(AIRB_MODE_RELAY);
    }

finish:
    if (hA) { WaitForSingleObject(hA, 2000); CloseHandle(hA); }
    if (hB) { WaitForSingleObject(hB, 2000); CloseHandle(hB); }
    if (hC) { WaitForSingleObject(hC, 2000); CloseHandle(hC); }
    ox_identity_erase(&rA); ox_identity_erase(&rB); ox_identity_erase(&rC);
    netpolicy_set_mode(saved);

    printf("\n  ==================================================\n");
    printf("  Total: %d | Passed: %d | Failed: %d\n",
           g_pass + g_fail, g_pass, g_fail);
    printf("  %s\n\n", g_fail == 0
           ? "LIVE PRODUCTION PATH IS ONION-PROTECTED"
           : "LIVE PATH FAILED - the socket is not protected");
    return g_fail == 0 ? 0 : 1;
}
