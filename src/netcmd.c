/*
 * Airbot — Executable Information System
 * netcmd.c — CLI commands that use the real network transport
 *
 * These are the only commands in the project that touch a live network.
 * Every other command operates entirely in-process.
 */
#include "netcmd.h"
#include "transport.h"
#include "eiu.h"
#include "blake3.h"
#include "netpolicy.h"
#include "socks5.h"
#include "privframe.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* --- helpers ---------------------------------------------- */

static void print_hex(const uint8_t *b, int n) {
    int i;
    for (i = 0; i < n; i++) printf("%02X", b[i]);
}

static const char *arg_value(int argc, char **argv, const char *flag) {
    int i;
    for (i = 2; i < argc - 1; i++)
        if (strcmp(argv[i], flag) == 0) return argv[i + 1];
    return 0;
}

/* Build a small but structurally real EIU to put on the wire. */
static int build_test_eiu(uint8_t *out, size_t cap, size_t *out_len) {
    EIU eiu;
    /* HALT-terminated behavior; opcode values are irrelevant to transport,
       what matters is that this round-trips as a valid EIU. */
    static const uint8_t behavior[] = { 0x01, 0x00, 0x2A, 0x10, 0x00 };
    static const uint8_t data[]     = { 'A','I','R','B','O','T','-','N','E','T' };
    static const uint8_t state[]    = { 0x00, 0x00, 0x00, 0x01 };

    eiu_init(&eiu);
    eiu_set_fuel(&eiu, 1000);
    if (eiu_set_behavior(&eiu, behavior, (uint16_t)sizeof(behavior)) != 0) return -1;
    if (eiu_set_data(&eiu, data, (uint16_t)sizeof(data)) != 0) return -1;
    if (eiu_set_state(&eiu, state, (uint16_t)sizeof(state)) != 0) return -1;
    eiu_compute_hash(&eiu);
    return eiu_serialize(&eiu, out, cap, out_len);
}

/* --- net-probe -------------------------------------------- */
/* DNS resolve + TCP connect to a real host, with timing. */

int cmd_net_probe(int argc, char **argv) {
    const char *host = arg_value(argc, argv, "--host");
    const char *ports = arg_value(argc, argv, "--port");
    uint16_t port = ports ? (uint16_t)atoi(ports) : 80;
    AirbConn conn;
    double t0, t1;
    int rc;

    if (!host) { printf("usage: airbot net-probe --host <host> [--port N]\n"); return 1; }

    printf("\n  AIRBOT NETWORK PROBE\n");
    printf("  --------------------------------------------------\n");
    printf("  Target      : %s:%u\n", host, (unsigned)port);

    if (transport_init() != AIRB_OK) {
        printf("  Winsock     : FAILED\n"); return 1;
    }
    printf("  Winsock     : initialized\n");

    t0 = transport_now_ms();
    rc = transport_dial(host, port, &conn);
    t1 = transport_now_ms();

    if (rc != AIRB_OK) {
        printf("  Result      : FAILED (%s), os error %d\n",
               transport_strerror(rc), transport_last_oserror());
        transport_cleanup();
        return 1;
    }

    printf("  Resolved    : %s\n", netlog_safe_addr(conn.peer));
    printf("  TCP connect : ESTABLISHED in %.2f ms\n", t1 - t0);
    printf("  Result      : REACHABLE\n\n");

    transport_close(&conn);
    transport_cleanup();
    return 0;
}

/* --- net-echo --------------------------------------------- */
/* Push a real serialized EIU across the internet to a foreign echo
   server and verify byte-for-byte that what came back is what went out. */

int cmd_net_echo(int argc, char **argv) {
    const char *host = arg_value(argc, argv, "--host");
    const char *ports = arg_value(argc, argv, "--port");
    uint16_t port = ports ? (uint16_t)atoi(ports) : 4242;
    AirbConn conn;
    uint8_t  eiu_buf[1024];
    uint8_t  decoded[1024];
    char     line[4096];
    char     back[4096];
    size_t   eiu_len = 0;
    uint8_t  sent_digest[32], recv_digest[32];
    double   t0, t1;
    int      rc, total = 0, n, i, nl = -1;
    static const char *HEXD = "0123456789ABCDEF";

    if (!host) { printf("usage: airbot net-echo --host <host> [--port N]\n"); return 1; }

    if (build_test_eiu(eiu_buf, sizeof(eiu_buf), &eiu_len) != 0) {
        printf("  EIU construction FAILED\n"); return 1;
    }
    blake3_hash(eiu_buf, eiu_len, sent_digest);

    /* Public echo servers are line-oriented: a raw binary EIU containing a
       0x0A byte gets truncated at that byte. Hex-encode with a newline
       terminator so the exact EIU bytes survive an arbitrary echo endpoint,
       then decode and digest-verify what comes back. */
    for (i = 0; i < (int)eiu_len; i++) {
        line[i * 2]     = HEXD[(eiu_buf[i] >> 4) & 0xF];
        line[i * 2 + 1] = HEXD[eiu_buf[i] & 0xF];
    }
    line[eiu_len * 2] = '\n';
    line[eiu_len * 2 + 1] = 0;

    printf("\n  AIRBOT EIU INTERNET ROUND-TRIP\n");
    printf("  --------------------------------------------------\n");
    printf("  EIU size    : %u bytes (serialized TLV)\n", (unsigned)eiu_len);
    printf("  EIU digest  : "); print_hex(sent_digest, 32); printf("\n");
    printf("  Wire form   : %u hex chars + LF (binary-safe framing)\n",
           (unsigned)(eiu_len * 2));
    printf("  Target      : %s:%u\n", host, (unsigned)port);

    rc = transport_dial(host, port, &conn);
    if (rc != AIRB_OK) {
        printf("  Result      : FAILED (%s), os error %d\n",
               transport_strerror(rc), transport_last_oserror());
        return 1;
    }
    printf("  Resolved    : %s\n", netlog_safe_addr(conn.peer));
    transport_set_timeout(&conn, 15000);

    t0 = transport_now_ms();
    rc = transport_send_raw(&conn, (const uint8_t *)line, (uint32_t)(eiu_len * 2 + 1));
    if (rc != AIRB_OK) {
        printf("  Result      : send FAILED (%s)\n", transport_strerror(rc));
        transport_close(&conn); return 1;
    }
    printf("  Sent        : %u bytes on the wire\n", (unsigned)(eiu_len * 2 + 1));

    /* Read until the echoed line terminator arrives. */
    while (total < (int)sizeof(back) - 1) {
        n = transport_recv_raw(&conn, (uint8_t *)(back + total),
                               (uint32_t)(sizeof(back) - 1 - total));
        if (n < 0) break;
        total += n;
        back[total] = 0;
        for (i = 0; i < total; i++) if (back[i] == '\n') { nl = i; break; }
        if (nl >= 0) break;
    }
    t1 = transport_now_ms();

    printf("  Received    : %d bytes back\n", total);
    printf("  Round-trip  : %.2f ms\n", t1 - t0);

    if (nl < 0) { printf("  Result      : no line terminator returned\n\n");
                  transport_close(&conn); transport_cleanup(); return 1; }
    if (nl != (int)(eiu_len * 2)) {
        printf("  Result      : LENGTH MISMATCH (sent %u hex chars, got %d)\n",
               (unsigned)(eiu_len * 2), nl);
        transport_close(&conn); transport_cleanup(); return 1;
    }

    /* Decode the returned hex back into EIU bytes. */
    for (i = 0; i < nl; i += 2) {
        int hi = back[i], lo = back[i + 1];
        hi = (hi >= 'A') ? (hi - 'A' + 10) : (hi - '0');
        lo = (lo >= 'A') ? (lo - 'A' + 10) : (lo - '0');
        decoded[i / 2] = (uint8_t)((hi << 4) | lo);
    }

    blake3_hash(decoded, eiu_len, recv_digest);
    printf("  Return dig  : "); print_hex(recv_digest, 32); printf("\n");

    if (memcmp(sent_digest, recv_digest, 32) != 0) {
        printf("  Result      : DIGEST MISMATCH - EIU corrupted in transit\n\n");
        transport_close(&conn); transport_cleanup(); return 1;
    }
    if (memcmp(decoded, eiu_buf, eiu_len) != 0) {
        printf("  Result      : BYTE MISMATCH\n\n");
        transport_close(&conn); transport_cleanup(); return 1;
    }
    printf("  Byte compare: IDENTICAL (%u/%u bytes)\n",
           (unsigned)eiu_len, (unsigned)eiu_len);

    {
        EIU parsed;
        if (eiu_deserialize(&parsed, decoded, eiu_len) == 0 && eiu_validate(&parsed)) {
            printf("  Re-parse    : VALID EIU (fuel=%u, behavior=%u B, data=%u B, state=%u B)\n",
                   (unsigned)parsed.fuel, (unsigned)parsed.behavior_len,
                   (unsigned)parsed.data_len, (unsigned)parsed.state_len);
            printf("  Payload     : \"");
            for (i = 0; i < (int)parsed.data_len && i < 32; i++)
                putchar(parsed.data[i] >= 32 && parsed.data[i] < 127 ? parsed.data[i] : '.');
            printf("\"\n");
        } else {
            printf("  Re-parse    : FAILED to deserialize returned bytes\n");
            transport_close(&conn); transport_cleanup(); return 1;
        }
    }

    printf("  Result      : ROUND-TRIP VERIFIED over the public internet\n\n");
    transport_close(&conn);
    transport_cleanup();
    return 0;
}

/* --- relay ------------------------------------------------ */
/* A real listening relay. Accepts Airbot frames; either forwards to the
   next hop or terminates the circuit and reports what it received. */

int cmd_relay(int argc, char **argv) {
    const char *ports = arg_value(argc, argv, "--port");
    const char *next  = arg_value(argc, argv, "--next");
    const char *counts = arg_value(argc, argv, "--count");
    uint16_t port = ports ? (uint16_t)atoi(ports) : 9101;
    int want = counts ? atoi(counts) : 1;
    AirbConn listener, peer;
    int rc, served = 0;

    /* A relay is public infrastructure: it is meant to be reachable, so it
       runs in the relay role rather than the privacy-client role.
       If the operator explicitly asked for privacy mode, entering the relay
       role would silently weaken what they asked for - refuse instead. */
    if (netpolicy_privacy_requested()) {
        printf("relay: REFUSED - AIRBOT_PRIVACY=1 requests the privacy CLIENT
");
        printf("       role, which has no inbound surface. A relay is public by
");
        printf("       design and is not anonymous; the roles are not combinable.
");
        printf("       Unset AIRBOT_PRIVACY to run a relay. See SECURITY.md s1.
");
        return 1;
    }
    netpolicy_set_mode(AIRB_MODE_RELAY);

    rc = transport_listen(port, &listener);
    if (rc != AIRB_OK) {
        printf("relay: listen on %u FAILED (%s), os error %d\n",
               (unsigned)port, transport_strerror(rc), transport_last_oserror());
        return 1;
    }
    printf("[relay:%u] listening%s%s\n", (unsigned)port,
           next ? ", forwarding to " : " (terminal hop)", next ? next : "");
    fflush(stdout);

    while (served < want) {
        AirbFrame frame;
        rc = transport_accept(&listener, &peer);
        if (rc != AIRB_OK) { printf("[relay:%u] accept failed\n", (unsigned)port); break; }
        printf("[relay:%u] connection from %s\n", (unsigned)port, peer.peer);
        fflush(stdout);

        rc = transport_recv_frame(&peer, &frame);
        if (rc != AIRB_OK) {
            printf("[relay:%u] recv failed: %s\n", (unsigned)port, transport_strerror(rc));
            transport_close(&peer); served++; continue;
        }

        printf("[relay:%u] frame type=0x%02X len=%u hops_left=%u digest=",
               (unsigned)port, frame.type, frame.length, frame.hops_left);
        print_hex(frame.digest, 8);
        printf("... digest_ok=%s\n", frame.digest_ok ? "YES" : "NO");
        fflush(stdout);

        if (next && frame.hops_left > 0) {
            /* Forward to the next hop, decrementing the hop counter. */
            char host[128]; uint16_t nport = 0;
            const char *colon = strrchr(next, ':');
            if (colon) {
                size_t hl = (size_t)(colon - next);
                if (hl >= sizeof(host)) hl = sizeof(host) - 1;
                memcpy(host, next, hl); host[hl] = 0;
                nport = (uint16_t)atoi(colon + 1);
            }
            if (nport) {
                AirbConn fwd;
                if (transport_dial(host, nport, &fwd) == AIRB_OK) {
                    transport_send_frame(&fwd, frame.type,
                                         (uint8_t)(frame.hops_left - 1),
                                         frame.payload, frame.length);
                    printf("[relay:%u] forwarded to %s (hops_left=%u)\n",
                           (unsigned)port, fwd.peer, (unsigned)(frame.hops_left - 1));
                    transport_close(&fwd);
                } else {
                    printf("[relay:%u] forward to %s:%u FAILED\n",
                           (unsigned)port, host, (unsigned)nport);
                }
            }
        } else {
            printf("[relay:%u] TERMINAL — payload: ", (unsigned)port);
            {
                EIU parsed;
                if (eiu_deserialize(&parsed, frame.payload, frame.length) == 0
                    && eiu_validate(&parsed)) {
                    printf("valid EIU, data=\"");
                    {
                        unsigned i;
                        for (i = 0; i < parsed.data_len && i < 32; i++)
                            putchar(parsed.data[i] >= 32 && parsed.data[i] < 127
                                    ? parsed.data[i] : '.');
                    }
                    printf("\"\n");
                } else {
                    printf("%u raw bytes\n", frame.length);
                }
            }
        }
        fflush(stdout);
        transport_close(&peer);
        served++;
    }

    transport_close(&listener);
    transport_cleanup();
    printf("[relay:%u] done (%d served)\n", (unsigned)port, served);
    return 0;
}

/* --- send ------------------------------------------------- */

int cmd_net_send(int argc, char **argv) {
    const char *host  = arg_value(argc, argv, "--host");
    const char *ports = arg_value(argc, argv, "--port");
    const char *hops  = arg_value(argc, argv, "--hops");
    uint16_t port = ports ? (uint16_t)atoi(ports) : 9101;
    uint8_t hops_left = hops ? (uint8_t)atoi(hops) : 0;
    AirbConn conn;
    uint8_t  eiu_buf[1024];
    size_t   eiu_len = 0;
    uint8_t  digest[32];
    double   t0, t1;
    int rc;

    if (!host) { printf("usage: airbot net-send --host <host> [--port N] [--hops N]\n"); return 1; }

    if (build_test_eiu(eiu_buf, sizeof(eiu_buf), &eiu_len) != 0) {
        printf("EIU construction FAILED\n"); return 1;
    }
    blake3_hash(eiu_buf, eiu_len, digest);

    rc = transport_dial(host, port, &conn);
    if (rc != AIRB_OK) {
        printf("send: dial %s:%u FAILED (%s), os error %d\n", host, (unsigned)port,
               transport_strerror(rc), transport_last_oserror());
        return 1;
    }

    t0 = transport_now_ms();
    rc = transport_send_frame(&conn, AIRB_FRAME_EIU, hops_left, eiu_buf, (uint32_t)eiu_len);
    t1 = transport_now_ms();

    if (rc != AIRB_OK) {
        printf("send: FAILED (%s)\n", transport_strerror(rc));
        transport_close(&conn); return 1;
    }

    printf("[send] EIU %u B -> %s in %.2f ms, hops_left=%u, digest=",
           (unsigned)eiu_len, conn.peer, t1 - t0, (unsigned)hops_left);
    print_hex(digest, 8);
    printf("...\n");

    transport_close(&conn);
    transport_cleanup();
    return 0;
}


/* --- privacy-mode commands -------------------------------- */

/* Single place where a privacy-mode connection is established. Refuses
   unless preflight has passed; never falls back to a direct dial. */
static int privacy_dial(const char *host, uint16_t port, AirbConn *conn) {
    int rc;
    if (!netpolicy_is_privacy()) return AIRB_ERR_POLICY;
    if (!netpolicy_is_validated()) {
        AirbPreflight p;
        rc = netpolicy_preflight(&p);
        if (rc != AIRB_POL_OK) {
            printf("  REFUSED     : %s\n", netpolicy_strerror(rc));
            printf("  detail      : %s\n", p.detail);
            return AIRB_ERR_POLICY;
        }
    }
    rc = socks5_connect(netpolicy_proxy_host(), netpolicy_proxy_port(),
                        host, port, NULL, conn);
    if (rc != SOCKS5_OK) {
        printf("  REFUSED     : %s\n", socks5_strerror(rc));
        return AIRB_ERR_POLICY;
    }
    return AIRB_OK;
}

int cmd_privacy_preflight(int argc, char **argv) {
    AirbPreflight p;
    const char *proxy = arg_value(argc, argv, "--proxy");
    int rc;
    if (proxy) netpolicy_set_proxy(proxy);
    netpolicy_set_mode(AIRB_MODE_PRIVACY);
    rc = netpolicy_preflight(&p);
    netpolicy_print_preflight(&p);
    if (rc != AIRB_POL_OK)
        printf("  Privacy mode will REFUSE all network operations.\n\n");
    return rc == AIRB_POL_OK ? 0 : 1;
}

int cmd_privacy_probe(int argc, char **argv) {
    const char *host  = arg_value(argc, argv, "--host");
    const char *ports = arg_value(argc, argv, "--port");
    const char *proxy = arg_value(argc, argv, "--proxy");
    uint16_t port = ports ? (uint16_t)atoi(ports) : 80;
    AirbConn conn;
    double t0, t1;
    int rc;

    if (!host) { printf("usage: airbot privacy-probe --host H [--port N] [--proxy H:P]\n"); return 1; }
    if (proxy) netpolicy_set_proxy(proxy);
    netpolicy_set_mode(AIRB_MODE_PRIVACY);
    netpolicy_reset_counters();

    printf("\n  AIRBOT PRIVACY PROBE (fail-closed)\n");
    printf("  --------------------------------------------------\n");
    printf("  Target      : %s:%u  (name resolved by Tor, not locally)\n",
           host, (unsigned)port);
    printf("  Proxy       : %s:%u\n", netpolicy_proxy_host(),
           (unsigned)netpolicy_proxy_port());

    t0 = transport_now_ms();
    rc = privacy_dial(host, port, &conn);
    t1 = transport_now_ms();

    if (rc != AIRB_OK) {
        printf("  Result      : FAILED CLOSED - no direct connection attempted\n");
        printf("  local DNS   : %lu lookups (must be 0)\n",
               netpolicy_counters()->local_dns_performed);
        printf("  direct dials: %lu denied\n\n",
               netpolicy_counters()->direct_dials_denied);
        return 1;
    }

    printf("  Circuit     : %s\n", conn.peer);
    printf("  Established : %.2f ms via Tor\n", t1 - t0);
    printf("  local DNS   : %lu lookups (must be 0)\n",
           netpolicy_counters()->local_dns_performed);
    printf("  Result      : REACHABLE THROUGH TOR\n\n");
    transport_close(&conn);
    transport_cleanup();
    return 0;
}


/*
 * privacy-fetch — end-to-end proof through real Tor.
 *
 * Issues a minimal HTTP/1.1 request over the Tor-tunnelled stream and prints
 * the response body. Pointed at an address-reflection service this shows,
 * from the destination's own perspective, which address it observed.
 */
int cmd_privacy_fetch(int argc, char **argv) {
    const char *host  = arg_value(argc, argv, "--host");
    const char *path  = arg_value(argc, argv, "--path");
    const char *ports = arg_value(argc, argv, "--port");
    const char *proxy = arg_value(argc, argv, "--proxy");
    uint16_t port = ports ? (uint16_t)atoi(ports) : 80;
    AirbConn conn;
    char req[512];
    uint8_t buf[8192];
    int rc, n, total = 0;
    double t0, t1;

    if (!host) { printf("usage: airbot privacy-fetch --host H [--path P] [--port N] [--proxy H:P]\n"); return 1; }
    if (!path) path = "/";
    if (proxy) netpolicy_set_proxy(proxy);
    netpolicy_set_mode(AIRB_MODE_PRIVACY);
    netpolicy_reset_counters();

    printf("\n  AIRBOT PRIVACY FETCH (through real Tor)\n");
    printf("  --------------------------------------------------\n");
    printf("  Target      : %s%s\n", host, path);

    t0 = transport_now_ms();
    rc = privacy_dial(host, port, &conn);
    if (rc != AIRB_OK) {
        printf("  Result      : FAILED CLOSED (no direct connection attempted)\n\n");
        return 1;
    }
    printf("  Circuit     : %s\n", conn.peer);

    sprintf(req, "GET %.200s HTTP/1.1\r\nHost: %.100s\r\n"
                 "User-Agent: curl/8.0.1\r\nConnection: close\r\n\r\n", path, host);
    if (transport_send_raw(&conn, (const uint8_t *)req, (uint32_t)strlen(req)) != AIRB_OK) {
        printf("  Result      : send failed\n"); transport_close(&conn); return 1;
    }

    while (total < (int)sizeof(buf) - 1) {
        n = transport_recv_raw(&conn, buf + total, (uint32_t)(sizeof(buf) - 1 - total));
        if (n <= 0) break;
        total += n;
    }
    t1 = transport_now_ms();
    buf[total > 0 ? total : 0] = 0;

    printf("  Elapsed     : %.0f ms\n", t1 - t0);
    printf("  local DNS   : %lu lookups (must be 0)\n",
           netpolicy_counters()->local_dns_performed);
    if (total > 0) {
        char *body = strstr((char *)buf, "\r\n\r\n");
        printf("  --- destination reported ---\n  %s\n",
               body ? body + 4 : (char *)buf);
    } else {
        printf("  (no response)\n");
    }
    printf("\n");
    transport_close(&conn);
    transport_cleanup();
    return 0;
}
