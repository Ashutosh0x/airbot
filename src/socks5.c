/*
 * Airbot — Executable Information System
 * socks5.c — SOCKS5 client (RFC 1928) for Tor egress
 */
#include "socks5.h"
#include "netpolicy.h"
#include "chacha20.h"

#include <stdio.h>
#include <string.h>

/* --- helpers ---------------------------------------------- */

static int io_send(const AirbConn *c, const uint8_t *b, uint32_t n) {
    return transport_send_raw(c, b, n) == AIRB_OK ? 0 : -1;
}

/* SOCKS5 replies are short and fixed-shape; read exactly n bytes. */
static int io_recv_exact(const AirbConn *c, uint8_t *b, uint32_t n) {
    uint32_t got = 0;
    while (got < n) {
        int r = transport_recv_raw(c, b + got, n - got);
        if (r <= 0) return -1;
        got += (uint32_t)r;
    }
    return 0;
}

/* Random printable token used as SOCKS credentials. Tor keys circuit
   isolation off this pair, so a fresh value means a fresh circuit. */
static void make_isolation_token(char *out, size_t cap) {
    static const char AL[] = "abcdefghijklmnopqrstuvwxyz0123456789";
    uint8_t r[16];
    size_t i, n = cap - 1;
    if (n > 16) n = 16;
    csprng_bytes(r, n);
    for (i = 0; i < n; i++) out[i] = AL[r[i] % (sizeof(AL) - 1)];
    out[n] = 0;
}

const char *socks5_strerror(int code) {
    switch (code) {
        case SOCKS5_OK:             return "ok";
        case SOCKS5_ERR_CONNECT:    return "proxy unreachable";
        case SOCKS5_ERR_HANDSHAKE:  return "not a SOCKS5 endpoint";
        case SOCKS5_ERR_AUTH:       return "proxy rejected isolation credentials";
        case SOCKS5_ERR_REQUEST:    return "CONNECT request failed";
        case SOCKS5_ERR_REPLY:      return "malformed or refused CONNECT reply";
        case SOCKS5_ERR_HOSTLEN:    return "hostname too long for SOCKS5";
        case SOCKS5_ERR_POLICY:     return "policy gate refused the proxy hop";
        default:                    return "unknown socks5 error";
    }
}

const char *socks5_reply_name(uint8_t rep) {
    switch (rep) {
        case SOCKS5_REP_SUCCESS:      return "succeeded";
        case SOCKS5_REP_GENERAL_FAIL: return "general SOCKS server failure";
        case SOCKS5_REP_NOT_ALLOWED:  return "connection not allowed by ruleset";
        case SOCKS5_REP_NET_UNREACH:  return "network unreachable";
        case SOCKS5_REP_HOST_UNREACH: return "host unreachable";
        case SOCKS5_REP_REFUSED:      return "connection refused";
        case SOCKS5_REP_TTL_EXPIRED:  return "TTL expired";
        case SOCKS5_REP_CMD_UNSUPP:   return "command not supported";
        case SOCKS5_REP_ATYP_UNSUPP:  return "address type not supported";
        default:                      return "unassigned reply code";
    }
}

/* --- method negotiation ----------------------------------- */

/* Offer no-auth and username/password. Tor accepts both; the credentials
   carry isolation rather than authentication, so either selection works. */
static int negotiate(const AirbConn *c, const char *token, int *used_auth) {
    uint8_t greet[4], resp[2];
    greet[0] = 0x05;          /* VER */
    greet[1] = 0x02;          /* NMETHODS */
    greet[2] = 0x00;          /* NO AUTHENTICATION REQUIRED */
    greet[3] = 0x02;          /* USERNAME/PASSWORD */
    if (io_send(c, greet, 4) != 0) return SOCKS5_ERR_HANDSHAKE;
    if (io_recv_exact(c, resp, 2) != 0) return SOCKS5_ERR_HANDSHAKE;
    if (resp[0] != 0x05) return SOCKS5_ERR_HANDSHAKE;

    if (resp[1] == 0x02) {
        /* RFC 1929 username/password sub-negotiation. */
        uint8_t req[1 + 1 + 255 + 1 + 255], rep[2];
        size_t tl = strlen(token), n = 0;
        if (tl > 255) tl = 255;
        req[n++] = 0x01;                       /* sub-negotiation version */
        req[n++] = (uint8_t)tl;
        memcpy(req + n, token, tl); n += tl;
        req[n++] = (uint8_t)tl;
        memcpy(req + n, token, tl); n += tl;
        if (io_send(c, req, (uint32_t)n) != 0) return SOCKS5_ERR_AUTH;
        if (io_recv_exact(c, rep, 2) != 0) return SOCKS5_ERR_AUTH;
        if (rep[1] != 0x00) return SOCKS5_ERR_AUTH;
        *used_auth = 1;
        return SOCKS5_OK;
    }
    if (resp[1] == 0x00) { *used_auth = 0; return SOCKS5_OK; }
    return SOCKS5_ERR_HANDSHAKE;   /* 0xFF = no acceptable methods */
}

/* --- probe ------------------------------------------------ */

int socks5_probe(const char *proxy_host, uint16_t proxy_port) {
    AirbConn c;
    int rc, used_auth = 0;
    char token[17];

    rc = netpolicy_authorize_dial(proxy_host, proxy_port, 1);
    if (rc != AIRB_POL_OK) return SOCKS5_ERR_POLICY;

    rc = transport_dial_ex(proxy_host, proxy_port, 1, &c);
    if (rc == AIRB_ERR_POLICY) return SOCKS5_ERR_POLICY;
    if (rc != AIRB_OK) return SOCKS5_ERR_CONNECT;

    transport_set_timeout(&c, 8000);
    make_isolation_token(token, sizeof(token));
    rc = negotiate(&c, token, &used_auth);
    transport_close(&c);
    return rc;
}

/* --- connect ---------------------------------------------- */

int socks5_connect(const char *proxy_host, uint16_t proxy_port,
                   const char *host, uint16_t port,
                   const char *isolation_token,
                   AirbConn *conn) {
    uint8_t req[262], rep[4], scratch[256];
    char token[17];
    size_t hl, n = 0;
    int rc, used_auth = 0;

    if (!conn || !host) return SOCKS5_ERR_REQUEST;
    hl = strlen(host);
    if (hl == 0 || hl > 255) return SOCKS5_ERR_HOSTLEN;

    /* The proxy hop is the one socket privacy mode permits, and the gate
       re-checks that it terminates on loopback. */
    rc = netpolicy_authorize_dial(proxy_host, proxy_port, 1);
    if (rc != AIRB_POL_OK) return SOCKS5_ERR_POLICY;

    rc = transport_dial_ex(proxy_host, proxy_port, 1, conn);
    if (rc == AIRB_ERR_POLICY) return SOCKS5_ERR_POLICY;
    if (rc != AIRB_OK) return SOCKS5_ERR_CONNECT;
    transport_set_timeout(conn, 30000);

    if (isolation_token && *isolation_token) {
        strncpy(token, isolation_token, sizeof(token) - 1);
        token[sizeof(token) - 1] = 0;
    } else {
        make_isolation_token(token, sizeof(token));
    }

    rc = negotiate(conn, token, &used_auth);
    if (rc != SOCKS5_OK) { transport_close(conn); return rc; }

    /* CONNECT with ATYP=DOMAINNAME. The hostname crosses to Tor as text and
       is resolved at the exit relay — this is the DNS-leak fix. */
    req[n++] = 0x05;                 /* VER    */
    req[n++] = 0x01;                 /* CMD = CONNECT */
    req[n++] = 0x00;                 /* RSV    */
    req[n++] = 0x03;                 /* ATYP = DOMAINNAME */
    req[n++] = (uint8_t)hl;
    memcpy(req + n, host, hl); n += hl;
    req[n++] = (uint8_t)(port >> 8);
    req[n++] = (uint8_t)(port & 0xFF);

    if (io_send(conn, req, (uint32_t)n) != 0) {
        transport_close(conn); return SOCKS5_ERR_REQUEST;
    }

    /* Reply: VER REP RSV ATYP BND.ADDR BND.PORT */
    if (io_recv_exact(conn, rep, 4) != 0) {
        transport_close(conn); return SOCKS5_ERR_REPLY;
    }
    if (rep[0] != 0x05) { transport_close(conn); return SOCKS5_ERR_REPLY; }
    if (rep[1] != SOCKS5_REP_SUCCESS) {
        transport_close(conn);
        return SOCKS5_ERR_REPLY;
    }

    /* Drain the bound address so the stream starts at application data. */
    switch (rep[3]) {
        case 0x01: rc = io_recv_exact(conn, scratch, 4 + 2); break;   /* IPv4 */
        case 0x04: rc = io_recv_exact(conn, scratch, 16 + 2); break;  /* IPv6 */
        case 0x03: {
            uint8_t l;
            if (io_recv_exact(conn, &l, 1) != 0) { rc = -1; break; }
            rc = io_recv_exact(conn, scratch, (uint32_t)l + 2);
            break;
        }
        default: rc = -1;
    }
    if (rc != 0) { transport_close(conn); return SOCKS5_ERR_REPLY; }

    /* Deliberately record only the circuit label, never a resolved address:
       the client is not supposed to learn the destination IP, and storing
       it would put it into logs. */
    sprintf(conn->peer, "tor:circuit-%.8s", token);
    return SOCKS5_OK;
}
