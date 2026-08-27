/*
 * Airbot — Executable Information System
 * socks5.h — SOCKS5 client (RFC 1928) for Tor egress
 *
 * Two properties matter here beyond "it connects":
 *
 * 1. REMOTE NAME RESOLUTION. CONNECT requests are issued with
 *    ATYP = 0x03 (DOMAINNAME), handing the hostname to Tor as text.
 *    Tor resolves it at the exit relay. The client never calls
 *    getaddrinfo(), so no query reaches the local resolver, the router
 *    or the ISP. This is the mechanism that closes the DNS leak.
 *
 * 2. STREAM ISOLATION. Tor treats the SOCKS username/password pair as a
 *    circuit-isolation token (IsolateSOCKSAuth, on by default). Supplying
 *    fresh random credentials per connection places each connection on its
 *    own circuit, so two Airbot connections do not share an exit relay and
 *    cannot be trivially linked by a relay operator.
 */
#ifndef AIRBOT_SOCKS5_H
#define AIRBOT_SOCKS5_H

#include <stdint.h>
#include "transport.h"

#define SOCKS5_OK             0
#define SOCKS5_ERR_CONNECT   -20  /* could not reach the proxy at all */
#define SOCKS5_ERR_HANDSHAKE -21  /* proxy did not negotiate SOCKS5 */
#define SOCKS5_ERR_AUTH      -22  /* proxy rejected the isolation credentials */
#define SOCKS5_ERR_REQUEST   -23  /* CONNECT request write failed */
#define SOCKS5_ERR_REPLY     -24  /* malformed or refused CONNECT reply */
#define SOCKS5_ERR_HOSTLEN   -25  /* hostname longer than 255 bytes */
#define SOCKS5_ERR_POLICY    -26  /* policy gate refused the proxy hop */

/* SOCKS5 server reply codes, surfaced for diagnostics. */
#define SOCKS5_REP_SUCCESS        0x00
#define SOCKS5_REP_GENERAL_FAIL   0x01
#define SOCKS5_REP_NOT_ALLOWED    0x02
#define SOCKS5_REP_NET_UNREACH    0x03
#define SOCKS5_REP_HOST_UNREACH   0x04
#define SOCKS5_REP_REFUSED        0x05
#define SOCKS5_REP_TTL_EXPIRED    0x06
#define SOCKS5_REP_CMD_UNSUPP     0x07
#define SOCKS5_REP_ATYP_UNSUPP    0x08

/*
 * Open a TCP stream to host:port through the SOCKS5 proxy.
 *
 * `host` is transmitted as a domain name and resolved by Tor. It is never
 * passed to a local resolver. On success `conn` is an ordinary connected
 * stream whose peer is the proxy; application bytes written to it emerge
 * from the Tor exit.
 *
 * `isolation_token` selects the circuit. Pass NULL for fresh random
 * credentials (a new circuit); pass a stable string to deliberately share
 * a circuit across related connections.
 */
int socks5_connect(const char *proxy_host, uint16_t proxy_port,
                   const char *host, uint16_t port,
                   const char *isolation_token,
                   AirbConn *conn);

/*
 * Negotiate SOCKS5 against the proxy and disconnect without issuing a
 * CONNECT. Used by the preflight to distinguish "nothing is listening"
 * from "something is listening but is not a SOCKS5 proxy".
 */
int socks5_probe(const char *proxy_host, uint16_t proxy_port);

const char *socks5_strerror(int code);
const char *socks5_reply_name(uint8_t rep);

#endif /* AIRBOT_SOCKS5_H */
