/*
 * Airbot — Executable Information System
 * transport.h — Real network transport for EIUs (TCP/IPv4, Winsock)
 *
 * This module moves EIUs over actual sockets. Everything above it in the
 * project (onion.c, environment.c, matrix.c) operates in-process; this is
 * the boundary where an EIU becomes bytes on the wire.
 *
 * Wire frame layout (big-endian lengths, 12-byte header):
 *
 *   off  size  field
 *   0    4     magic       "AIRB"
 *   4    1     version     0x01
 *   5    1     type        AIRB_FRAME_*
 *   6    1     hops_left   decremented per relay
 *   7    1     reserved    0
 *   8    4     length      payload length
 *   12   N     payload
 *   12+N 32    digest      BLAKE3-256 over payload
 *
 * The digest is carried on the wire so corruption or truncation in transit is
 * detected at the receiver rather than silently accepted.
 */
#ifndef AIRBOT_TRANSPORT_H
#define AIRBOT_TRANSPORT_H

#include <stdint.h>
#include <stddef.h>

#define AIRB_MAGIC0 'A'
#define AIRB_MAGIC1 'I'
#define AIRB_MAGIC2 'R'
#define AIRB_MAGIC3 'B'

#define AIRB_WIRE_VERSION   0x01
#define AIRB_HEADER_SIZE    12
#define AIRB_DIGEST_SIZE    32
#define AIRB_MAX_PAYLOAD    8192

/* Frame types */
#define AIRB_FRAME_EIU      0x01  /* A serialized EIU */
#define AIRB_FRAME_ONION    0x02  /* An onion-wrapped packet */
#define AIRB_FRAME_PING     0x03  /* Reachability probe */
#define AIRB_FRAME_PONG     0x04  /* Probe reply */
#define AIRB_FRAME_ECHO     0x05  /* Echo test (round-trip integrity) */

/* Error codes */
#define AIRB_OK              0
#define AIRB_ERR_INIT       -1
#define AIRB_ERR_RESOLVE    -2
#define AIRB_ERR_SOCKET     -3
#define AIRB_ERR_CONNECT    -4
#define AIRB_ERR_SEND       -5
#define AIRB_ERR_RECV       -6
#define AIRB_ERR_BADMAGIC   -7
#define AIRB_ERR_BADDIGEST  -8
#define AIRB_ERR_TOOBIG     -9
#define AIRB_ERR_BIND      -10
#define AIRB_ERR_LISTEN    -11
#define AIRB_ERR_ACCEPT    -12
#define AIRB_ERR_CLOSED    -13
#define AIRB_ERR_POLICY    -14  /* refused by the fail-closed privacy gate */

/* An open connection. Opaque handle over a platform socket. */
typedef struct {
    uint64_t sock;          /* SOCKET value */
    char     peer[64];      /* Human-readable peer address */
    int      valid;
} AirbConn;

/* A decoded inbound frame. */
typedef struct {
    uint8_t  type;
    uint8_t  hops_left;
    uint32_t length;
    uint8_t  payload[AIRB_MAX_PAYLOAD];
    uint8_t  digest[AIRB_DIGEST_SIZE];
    int      digest_ok;     /* 1 if recomputed digest matched the wire digest */
} AirbFrame;

/* Library lifecycle (WSAStartup / WSACleanup on Windows). */
int  transport_init(void);
void transport_cleanup(void);

/* Human-readable form of an AIRB_ERR_* code. */
const char *transport_strerror(int err);

/* Last platform-level socket error (WSAGetLastError). */
int  transport_last_oserror(void);

/*
 * Resolve `host` and open a TCP connection to `port`.
 * `host` may be a hostname or dotted-quad IPv4 literal.
 * Writes the resolved dotted-quad into conn->peer.
 */
int  transport_dial(const char *host, uint16_t port, AirbConn *conn);

/*
 * As transport_dial, but declares whether this is the SOCKS client's hop to
 * the local Tor proxy. Privacy mode permits only via_proxy=1 dials; every
 * via_proxy=0 dial is refused with AIRB_ERR_POLICY and no socket is created.
 */
int  transport_dial_ex(const char *host, uint16_t port, int via_proxy, AirbConn *conn);

/* Bind and listen on `port` (all interfaces). */
int  transport_listen(uint16_t port, AirbConn *listener);

/* Block until a peer connects. */
int  transport_accept(const AirbConn *listener, AirbConn *conn);

/*
 * LEGACY CLEARTEXT FRAMER - DIRECT MODE ONLY.
 * Emits AIRB magic + cleartext hop counter + stable BLAKE3 digest. All three
 * are correlation handles. REFUSED in privacy mode (AIRB_ERR_POLICY); use
 * airbchan_send() there, which onion-encrypts per hop.
 */
int  transport_send_frame(const AirbConn *conn, uint8_t type, uint8_t hops_left,
                          const uint8_t *payload, uint32_t length);

/* Receive one framed message. Verifies magic and digest. */
int  transport_recv_frame(const AirbConn *conn, AirbFrame *frame);

/* Send raw bytes with no framing (for speaking to foreign servers). */
int  transport_send_raw(const AirbConn *conn, const uint8_t *buf, uint32_t len);

/* Receive up to `cap` raw bytes; returns count read or negative error. */
int  transport_recv_raw(const AirbConn *conn, uint8_t *buf, uint32_t cap);

/* Set send/recv timeout in milliseconds. */
int  transport_set_timeout(const AirbConn *conn, int millis);

void transport_close(AirbConn *conn);

/* Milliseconds since an arbitrary fixed origin — for latency measurement. */
double transport_now_ms(void);

#endif /* AIRBOT_TRANSPORT_H */
