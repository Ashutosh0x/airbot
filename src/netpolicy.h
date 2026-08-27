/*
 * Airbot — Executable Information System
 * netpolicy.h — Fail-closed network policy gate
 *
 * Every socket operation in the codebase must be authorized here first.
 * transport.c is the only module that issues syscalls, and each of its
 * entry points calls into this gate before touching Winsock.
 *
 * The gate has two modes:
 *
 *   AIRB_MODE_DIRECT   ordinary networking. Explicitly opt-in. Behaves as
 *                      the transport always has.
 *
 *   AIRB_MODE_PRIVACY  all traffic must egress through the configured Tor
 *                      SOCKS5 proxy. Direct dials, inbound listeners and
 *                      local DNS resolution are refused. There is no
 *                      fallback to direct networking: if Tor is missing,
 *                      misconfigured or unreachable, operations FAIL.
 *
 * The only sockets privacy mode permits are loopback connections to the
 * configured Tor SOCKS port, and those are re-validated as loopback at
 * the point of use rather than trusted from configuration.
 */
#ifndef AIRBOT_NETPOLICY_H
#define AIRBOT_NETPOLICY_H

#include <stdint.h>

typedef enum {
    AIRB_MODE_DIRECT  = 0,   /* ordinary networking, no privacy guarantees */
    AIRB_MODE_PRIVACY = 1,   /* privacy CLIENT: Tor-only, no inbound, no local DNS */
    AIRB_MODE_RELAY   = 2    /* RELAY node: listens by design, forwards onward */
} AirbNetMode;

/*
 * Role separation (Phase 9).
 *
 * AIRB_MODE_PRIVACY is a CLIENT role. It has no inbound surface at all: a
 * reachable listener is an address the client can be probed at, which is
 * exactly what an anonymity client must not have.
 *
 * AIRB_MODE_RELAY is infrastructure. A relay is *supposed* to be reachable,
 * so listeners are permitted. A relay is NOT anonymous — its address is
 * public by definition. Never run a privacy client in relay mode expecting
 * client anonymity; the two roles are deliberately not combinable.
 */

/* Policy refusal codes (distinct from transport AIRB_ERR_*). */
#define AIRB_POL_OK               0
#define AIRB_POL_DENY_DIRECT     -40  /* direct dial attempted in privacy mode */
#define AIRB_POL_DENY_LISTEN     -41  /* inbound listener attempted in privacy mode */
#define AIRB_POL_DENY_DNS        -42  /* local name resolution attempted in privacy mode */
#define AIRB_POL_NO_TOR          -43  /* Tor SOCKS proxy unreachable */
#define AIRB_POL_NOT_SOCKS       -44  /* endpoint did not speak SOCKS5 */
#define AIRB_POL_NOT_LOOPBACK    -45  /* configured proxy is not on loopback */
#define AIRB_POL_UNVALIDATED     -46  /* privacy mode active but preflight never passed */
#define AIRB_POL_BAD_CONFIG      -47
#define AIRB_POL_DENY_IPV6       -48  /* IPv6 refused in privacy mode */  /* proxy configuration unparseable */

/* Result of the startup validation sweep. Every field is 1 for pass, 0 for
   fail; `ok` is the AND of all of them. */
typedef struct {
    int privacy_mode_on;
    int proxy_config_parsed;
    int proxy_is_loopback;
    int tor_socks_reachable;
    int tor_speaks_socks5;
    int tor_remote_dns_ok;
    int direct_sockets_blocked;
    int local_dns_blocked;
    int inbound_listeners_blocked;
    int ok;
    char detail[256];
} AirbPreflight;

/* --- mode control ----------------------------------------- */

/* Set the active mode. Switching into privacy mode invalidates any prior
   preflight result, so validation must be re-run before traffic flows. */
void        netpolicy_set_mode(AirbNetMode mode);
AirbNetMode netpolicy_mode(void);
int         netpolicy_is_privacy(void);

/* Read AIRBOT_PRIVACY / AIRBOT_TOR_SOCKS from the environment. Called once
   at startup; command-line flags override afterwards. */
void netpolicy_init_from_env(void);

/* True when privacy mode was explicitly requested via AIRBOT_PRIVACY.
   A role that would weaken privacy (e.g. running as a relay) must REFUSE
   rather than override this - overriding it is a silent downgrade. */
int  netpolicy_privacy_requested(void);

/* Configure the Tor SOCKS5 endpoint (default 127.0.0.1:9050). */
int         netpolicy_set_proxy(const char *hostport);
const char *netpolicy_proxy_host(void);
uint16_t    netpolicy_proxy_port(void);

/* --- validation ------------------------------------------- */

/* Run the full startup sweep. In privacy mode this must return AIRB_POL_OK
   before any authorization call will succeed. Performs a live SOCKS5
   handshake against the proxy; does not fall back on failure. */
int  netpolicy_preflight(AirbPreflight *out);
void netpolicy_print_preflight(const AirbPreflight *p);

/* True only if a preflight has passed since the last mode change. */
int  netpolicy_is_validated(void);

/* --- authorization gate ----------------------------------- */

/* Authorize an outbound dial to host:port.
   `via_proxy` must be 1 when the caller is the SOCKS client connecting to
   the proxy itself, 0 for an ordinary application dial. */
int netpolicy_authorize_dial(const char *host, uint16_t port, int via_proxy);

/* Authorize binding an inbound listener. Always refused in privacy mode. */
int netpolicy_authorize_listen(uint16_t port);

/* Authorize a local getaddrinfo(). Always refused in privacy mode — names
   must be resolved remotely by Tor via SOCKS5 ATYP=DOMAINNAME. */
int netpolicy_authorize_dns(const char *host);

/* True for a dotted-quad literal, which getaddrinfo() resolves without
   emitting a query. Such hosts leak no destination name. */
int netpolicy_host_is_ip_literal(const char *host);

/*
 * IPv6 policy (explicit, not incidental). Privacy mode DENIES IPv6: Tor
 * chooses the outbound family behind the SOCKS boundary, so the client needs
 * no AF_INET6 socket, and an unpoliced IPv6 path could bypass Tor entirely.
 * Spec option B, chosen over A deliberately.
 */
int netpolicy_host_is_ipv6_literal(const char *host);
int netpolicy_authorize_family(const char *host);

const char *netpolicy_strerror(int code);

/* --- privacy-safe diagnostics (Phase 8) ------------------- */

/*
 * Network identifiers (peer IPs, resolved addresses, socket tuples) are
 * printed only when diagnostics are explicitly enabled via AIRBOT_DIAG=1.
 * Otherwise callers get a non-identifying description of the same event.
 *
 * This controls what AIRBOT writes. It cannot prevent the OS, netstat, a
 * packet capture or endpoint security software from observing the process's
 * sockets — see SECURITY.md.
 */
int  netlog_diagnostics_enabled(void);
void netlog_init_from_env(void);

/* Returns `addr` when diagnostics are on, and a non-identifying placeholder
   otherwise. Never returns NULL. */
const char *netlog_safe_addr(const char *addr);

/* --- observability for tests ------------------------------ */

/* Monotonic counters the leak-test harness asserts against. A privacy
   invariant violation is a non-zero denial counter paired with a completed
   connection, which the harness treats as a build failure. */
typedef struct {
    unsigned long direct_dials_denied;
    unsigned long listens_denied;
    unsigned long dns_denied;
    unsigned long proxy_dials_allowed;
    unsigned long direct_dials_allowed;
    unsigned long local_dns_performed;
    unsigned long ipv6_denied;
} AirbPolicyCounters;

const AirbPolicyCounters *netpolicy_counters(void);
void netpolicy_reset_counters(void);

/* Called by transport.c immediately before a real getaddrinfo() so the
   harness can prove privacy mode never reaches it. */
void netpolicy_note_local_dns(void);

#endif /* AIRBOT_NETPOLICY_H */
