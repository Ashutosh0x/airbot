/*
 * Airbot — Executable Information System
 * transport.c — Real network transport for EIUs (TCP/IPv4, Winsock)
 *
 * TinyCC ships no <winsock2.h> and no ws2_32 import library, so under
 * __TINYC__ the handful of Winsock entry points this module needs are
 * declared directly and linked against tools/tcc/lib/ws2_32.def.
 * Under GCC/MinGW the real headers are used instead.
 */
#include "transport.h"
#include "blake3.h"
#include "netpolicy.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* --- Platform layer -------------------------------------- */

#ifdef __TINYC__

typedef unsigned long long SOCKET_T;
typedef unsigned short     u16_t;

struct sockaddr { unsigned short sa_family; char sa_data[14]; };
struct in_addr_t_ { unsigned long s_addr; };
struct sockaddr_in_t {
    short  sin_family;
    unsigned short sin_port;
    struct in_addr_t_ sin_addr;
    char   sin_zero[8];
};
struct addrinfo_t {
    int ai_flags; int ai_family; int ai_socktype; int ai_protocol;
    unsigned long long ai_addrlen;
    char *ai_canonname;
    struct sockaddr *ai_addr;
    struct addrinfo_t *ai_next;
};

int      WSAStartup(u16_t, void*);
int      WSACleanup(void);
int      WSAGetLastError(void);
SOCKET_T socket(int, int, int);
int      connect(SOCKET_T, const struct sockaddr*, int);
int      bind(SOCKET_T, const struct sockaddr*, int);
int      listen(SOCKET_T, int);
SOCKET_T accept(SOCKET_T, struct sockaddr*, int*);
int      send(SOCKET_T, const char*, int, int);
int      recv(SOCKET_T, char*, int, int);
int      closesocket(SOCKET_T);
int      shutdown(SOCKET_T, int);
int      setsockopt(SOCKET_T, int, int, const char*, int);
int      getaddrinfo(const char*, const char*, const struct addrinfo_t*, struct addrinfo_t**);
void     freeaddrinfo(struct addrinfo_t*);
unsigned short htons(unsigned short);
unsigned long  htonl(unsigned long);
char*    inet_ntoa(struct in_addr_t_);

#define AF_INET_       2
#define SOCK_STREAM_   1
#define IPPROTO_TCP_   6
#define SOL_SOCKET_    0xffff
#define SO_RCVTIMEO_   0x1006
#define SO_SNDTIMEO_   0x1005
#define SO_REUSEADDR_  0x0004
#define INVALID_SOCK_  ((SOCKET_T)~0ULL)

#else  /* GCC / MinGW / MSVC */

#include <winsock2.h>
#include <ws2tcpip.h>
typedef SOCKET SOCKET_T;
#define sockaddr_in_t sockaddr_in
#define in_addr_t_    in_addr
#define addrinfo_t    addrinfo
#define AF_INET_      AF_INET
#define SOCK_STREAM_  SOCK_STREAM
#define IPPROTO_TCP_  IPPROTO_TCP
#define SOL_SOCKET_   SOL_SOCKET
#define SO_RCVTIMEO_  SO_RCVTIMEO
#define SO_SNDTIMEO_  SO_SNDTIMEO
#define SO_REUSEADDR_ SO_REUSEADDR
#define INVALID_SOCK_ INVALID_SOCKET

#endif

#include <windows.h>

/* --- Lifecycle -------------------------------------------- */

static int g_initialized = 0;

int transport_init(void) {
    /* Oversized on purpose: WSADATA layout differs between Win32 and Win64
       and this module never reads the contents. */
    char wsadata[1024];
    if (g_initialized) return AIRB_OK;
    memset(wsadata, 0, sizeof(wsadata));
    if (WSAStartup(0x0202, wsadata) != 0) return AIRB_ERR_INIT;
    g_initialized = 1;
    return AIRB_OK;
}

void transport_cleanup(void) {
    if (g_initialized) { WSACleanup(); g_initialized = 0; }
}

int transport_last_oserror(void) { return WSAGetLastError(); }

const char *transport_strerror(int err) {
    switch (err) {
        case AIRB_OK:             return "ok";
        case AIRB_ERR_INIT:       return "winsock initialization failed";
        case AIRB_ERR_RESOLVE:    return "DNS resolution failed";
        case AIRB_ERR_SOCKET:     return "socket creation failed";
        case AIRB_ERR_CONNECT:    return "connection failed";
        case AIRB_ERR_SEND:       return "send failed";
        case AIRB_ERR_RECV:       return "receive failed";
        case AIRB_ERR_BADMAGIC:   return "bad frame magic (not an Airbot frame)";
        case AIRB_ERR_BADDIGEST:  return "payload digest mismatch";
        case AIRB_ERR_TOOBIG:     return "payload exceeds AIRB_MAX_PAYLOAD";
        case AIRB_ERR_BIND:       return "bind failed (port in use?)";
        case AIRB_ERR_LISTEN:     return "listen failed";
        case AIRB_ERR_ACCEPT:     return "accept failed";
        case AIRB_ERR_CLOSED:     return "peer closed connection";
        case AIRB_ERR_POLICY:     return "refused by privacy policy (fail-closed)";
        default:                  return "unknown error";
    }
}

double transport_now_ms(void) {
    LARGE_INTEGER f, c;
    QueryPerformanceFrequency(&f);
    QueryPerformanceCounter(&c);
    return (double)c.QuadPart * 1000.0 / (double)f.QuadPart;
}

/* --- Connection setup ------------------------------------- */

int transport_dial(const char *host, uint16_t port, AirbConn *conn) {
    return transport_dial_ex(host, port, 0, conn);
}

int transport_dial_ex(const char *host, uint16_t port, int via_proxy, AirbConn *conn) {
    struct addrinfo_t hints, *res = 0;
    char portstr[16];
    SOCKET_T s;
    int rc;

    if (!conn) return AIRB_ERR_CONNECT;
    memset(conn, 0, sizeof(*conn));

    /* Policy gate: refuses every non-proxy dial while privacy mode is on.
       There is no fallback branch below this point. */
    rc = netpolicy_authorize_dial(host, port, via_proxy);
    if (rc != AIRB_POL_OK) return AIRB_ERR_POLICY;

    /* Explicit address-family policy: no accidental IPv6 path may exist
       alongside the Tor path. */
    rc = netpolicy_authorize_family(host);
    if (rc != AIRB_POL_OK) return AIRB_ERR_POLICY;

    /* Name resolution gate: privacy mode never reaches getaddrinfo(),
       because socks5_connect() hands the hostname to Tor instead. */
    rc = netpolicy_authorize_dns(host);
    if (rc != AIRB_POL_OK) return AIRB_ERR_POLICY;

    if ((rc = transport_init()) != AIRB_OK) return rc;

    sprintf(portstr, "%u", (unsigned)port);
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET_;
    hints.ai_socktype = SOCK_STREAM_;
    hints.ai_protocol = IPPROTO_TCP_;

    /* Count only real name lookups. Resolving a dotted-quad emits no query,
       so counting it would make the leak metric read false-positive. */
    if (!netpolicy_host_is_ip_literal(host)) netpolicy_note_local_dns();
    if (getaddrinfo(host, portstr, &hints, &res) != 0 || !res)
        return AIRB_ERR_RESOLVE;

    /* Record the resolved address so callers can report where they went. */
    {
        struct sockaddr_in_t *sin = (struct sockaddr_in_t *)res->ai_addr;
        const char *ip = inet_ntoa(sin->sin_addr);
        sprintf(conn->peer, "%.40s:%u", ip ? ip : "?", (unsigned)port);
    }

    s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (s == INVALID_SOCK_) { freeaddrinfo(res); return AIRB_ERR_SOCKET; }

    if (connect(s, res->ai_addr, (int)res->ai_addrlen) != 0) {
        closesocket(s); freeaddrinfo(res); return AIRB_ERR_CONNECT;
    }
    freeaddrinfo(res);

    conn->sock  = (uint64_t)s;
    conn->valid = 1;
    return AIRB_OK;
}

int transport_listen(uint16_t port, AirbConn *listener) {
    struct sockaddr_in_t addr;
    SOCKET_T s;
    int yes = 1, rc;

    if (!listener) return AIRB_ERR_BIND;
    memset(listener, 0, sizeof(*listener));

    /* An anonymity client exposes no inbound surface. */
    rc = netpolicy_authorize_listen(port);
    if (rc != AIRB_POL_OK) return AIRB_ERR_POLICY;

    if ((rc = transport_init()) != AIRB_OK) return rc;

    s = socket(AF_INET_, SOCK_STREAM_, IPPROTO_TCP_);
    if (s == INVALID_SOCK_) return AIRB_ERR_SOCKET;

    setsockopt(s, SOL_SOCKET_, SO_REUSEADDR_, (const char *)&yes, sizeof(yes));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET_;
    addr.sin_port        = htons(port);
    addr.sin_addr.s_addr = htonl(0); /* INADDR_ANY */

    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        closesocket(s); return AIRB_ERR_BIND;
    }
    if (listen(s, 8) != 0) { closesocket(s); return AIRB_ERR_LISTEN; }

    listener->sock  = (uint64_t)s;
    listener->valid = 1;
    sprintf(listener->peer, "0.0.0.0:%u", (unsigned)port);
    return AIRB_OK;
}

int transport_accept(const AirbConn *listener, AirbConn *conn) {
    struct sockaddr_in_t peer;
    int plen = (int)sizeof(peer);
    SOCKET_T c;

    if (!listener || !listener->valid || !conn) return AIRB_ERR_ACCEPT;
    memset(conn, 0, sizeof(*conn));
    memset(&peer, 0, sizeof(peer));

    c = accept((SOCKET_T)listener->sock, (struct sockaddr *)&peer, &plen);
    if (c == INVALID_SOCK_) return AIRB_ERR_ACCEPT;

    {
        const char *ip = inet_ntoa(peer.sin_addr);
        sprintf(conn->peer, "%.40s", ip ? ip : "?");
    }
    conn->sock  = (uint64_t)c;
    conn->valid = 1;
    return AIRB_OK;
}

int transport_set_timeout(const AirbConn *conn, int millis) {
    if (!conn || !conn->valid) return AIRB_ERR_SOCKET;
    setsockopt((SOCKET_T)conn->sock, SOL_SOCKET_, SO_RCVTIMEO_,
               (const char *)&millis, sizeof(millis));
    setsockopt((SOCKET_T)conn->sock, SOL_SOCKET_, SO_SNDTIMEO_,
               (const char *)&millis, sizeof(millis));
    return AIRB_OK;
}

void transport_close(AirbConn *conn) {
    if (conn && conn->valid) {
        shutdown((SOCKET_T)conn->sock, 2 /* SD_BOTH */);
        closesocket((SOCKET_T)conn->sock);
        conn->valid = 0;
    }
}

/* --- Raw I/O ---------------------------------------------- */

/* TCP is a stream: a single send() may be split and a single recv() may
   return short. Both helpers loop until the full count is transferred. */
static int send_all(SOCKET_T s, const uint8_t *buf, uint32_t len) {
    uint32_t sent = 0;
    while (sent < len) {
        int n = send(s, (const char *)(buf + sent), (int)(len - sent), 0);
        if (n <= 0) return AIRB_ERR_SEND;
        sent += (uint32_t)n;
    }
    return AIRB_OK;
}

static int recv_all(SOCKET_T s, uint8_t *buf, uint32_t len) {
    uint32_t got = 0;
    while (got < len) {
        int n = recv(s, (char *)(buf + got), (int)(len - got), 0);
        if (n == 0) return AIRB_ERR_CLOSED;
        if (n < 0)  return AIRB_ERR_RECV;
        got += (uint32_t)n;
    }
    return AIRB_OK;
}

int transport_send_raw(const AirbConn *conn, const uint8_t *buf, uint32_t len) {
    if (!conn || !conn->valid) return AIRB_ERR_SEND;
    return send_all((SOCKET_T)conn->sock, buf, len);
}

int transport_recv_raw(const AirbConn *conn, uint8_t *buf, uint32_t cap) {
    int n;
    if (!conn || !conn->valid) return AIRB_ERR_RECV;
    n = recv((SOCKET_T)conn->sock, (char *)buf, (int)cap, 0);
    if (n == 0) return AIRB_ERR_CLOSED;
    if (n < 0)  return AIRB_ERR_RECV;
    return n;
}

/* --- Framed I/O ------------------------------------------- */

static void put_be32(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)(v >> 24); p[1] = (uint8_t)(v >> 16);
    p[2] = (uint8_t)(v >> 8);  p[3] = (uint8_t)(v);
}

static uint32_t get_be32(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8)  |  (uint32_t)p[3];
}

int transport_send_frame(const AirbConn *conn, uint8_t type, uint8_t hops_left,
                         const uint8_t *payload, uint32_t length) {
    uint8_t header[AIRB_HEADER_SIZE];
    uint8_t digest[AIRB_DIGEST_SIZE];
    int rc;

    if (!conn || !conn->valid) return AIRB_ERR_SEND;
    if (length > AIRB_MAX_PAYLOAD) return AIRB_ERR_TOOBIG;

    /*
     * FAIL CLOSED. This is the legacy cleartext framer: it emits an "AIRB"
     * magic, a cleartext hop counter and a stable content digest. A socket
     * capture of the production path proved those bytes were still going out
     * while onionx sat unused. Privacy mode now refuses this framer outright;
     * application data must go through airbchan_send(), which onion-encrypts.
     * Enforced here rather than at the call sites so no caller can forget.
     */
    if (netpolicy_is_privacy()) return AIRB_ERR_POLICY;

    header[0] = AIRB_MAGIC0; header[1] = AIRB_MAGIC1;
    header[2] = AIRB_MAGIC2; header[3] = AIRB_MAGIC3;
    header[4] = AIRB_WIRE_VERSION;
    header[5] = type;
    header[6] = hops_left;
    header[7] = 0;
    put_be32(header + 8, length);

    blake3_hash(payload, (size_t)length, digest);

    if ((rc = send_all((SOCKET_T)conn->sock, header, AIRB_HEADER_SIZE)) != AIRB_OK) return rc;
    if (length && (rc = send_all((SOCKET_T)conn->sock, payload, length)) != AIRB_OK) return rc;
    return send_all((SOCKET_T)conn->sock, digest, AIRB_DIGEST_SIZE);
}

int transport_recv_frame(const AirbConn *conn, AirbFrame *frame) {
    uint8_t header[AIRB_HEADER_SIZE];
    uint8_t computed[AIRB_DIGEST_SIZE];
    int rc;

    if (!conn || !conn->valid || !frame) return AIRB_ERR_RECV;
    /* Symmetric refusal: a privacy-mode peer must not accept cleartext
       frames either, or an attacker could downgrade the link by sending one. */
    if (netpolicy_is_privacy()) return AIRB_ERR_POLICY;
    memset(frame, 0, sizeof(*frame));

    if ((rc = recv_all((SOCKET_T)conn->sock, header, AIRB_HEADER_SIZE)) != AIRB_OK) return rc;

    if (header[0] != AIRB_MAGIC0 || header[1] != AIRB_MAGIC1 ||
        header[2] != AIRB_MAGIC2 || header[3] != AIRB_MAGIC3)
        return AIRB_ERR_BADMAGIC;

    frame->type      = header[5];
    frame->hops_left = header[6];
    frame->length    = get_be32(header + 8);
    if (frame->length > AIRB_MAX_PAYLOAD) return AIRB_ERR_TOOBIG;

    if (frame->length &&
        (rc = recv_all((SOCKET_T)conn->sock, frame->payload, frame->length)) != AIRB_OK)
        return rc;
    if ((rc = recv_all((SOCKET_T)conn->sock, frame->digest, AIRB_DIGEST_SIZE)) != AIRB_OK)
        return rc;

    blake3_hash(frame->payload, (size_t)frame->length, computed);
    frame->digest_ok = (memcmp(computed, frame->digest, AIRB_DIGEST_SIZE) == 0);
    if (!frame->digest_ok) return AIRB_ERR_BADDIGEST;

    return AIRB_OK;
}
