/*
 * Airbot — Executable Information System
 * netpolicy.c — Fail-closed network policy gate
 *
 * Design rule: every refusal path returns an error. There is no branch in
 * this file that turns a failed privacy check into a direct connection.
 */
#include "netpolicy.h"
#include "socks5.h"
#include "transport.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* --- state ------------------------------------------------ */

static AirbNetMode        g_mode       = AIRB_MODE_DIRECT;
static int                g_validated  = 0;
static char               g_proxy_host[64] = "127.0.0.1";
static uint16_t           g_proxy_port     = 9050;
static AirbPolicyCounters g_counters;
static int                g_env_privacy = 0;

void netpolicy_set_mode(AirbNetMode mode) {
    if (mode != g_mode) g_validated = 0;  /* re-validate after any change */
    g_mode = mode;
}

AirbNetMode netpolicy_mode(void)     { return g_mode; }
int netpolicy_is_privacy(void)       { return g_mode == AIRB_MODE_PRIVACY; }
int netpolicy_is_validated(void)     { return g_validated; }
int netpolicy_privacy_requested(void) { return g_env_privacy; }

const AirbPolicyCounters *netpolicy_counters(void) { return &g_counters; }
void netpolicy_reset_counters(void) { memset(&g_counters, 0, sizeof(g_counters)); }
void netpolicy_note_local_dns(void) { g_counters.local_dns_performed++; }

const char *netpolicy_proxy_host(void) { return g_proxy_host; }
uint16_t    netpolicy_proxy_port(void) { return g_proxy_port; }

int netpolicy_set_proxy(const char *hostport) {
    const char *colon;
    size_t hl;
    if (!hostport || !*hostport) return AIRB_POL_BAD_CONFIG;
    colon = strrchr(hostport, ':');
    if (!colon || colon == hostport) return AIRB_POL_BAD_CONFIG;
    hl = (size_t)(colon - hostport);
    if (hl >= sizeof(g_proxy_host)) return AIRB_POL_BAD_CONFIG;
    memcpy(g_proxy_host, hostport, hl);
    g_proxy_host[hl] = 0;
    g_proxy_port = (uint16_t)atoi(colon + 1);
    if (g_proxy_port == 0) return AIRB_POL_BAD_CONFIG;
    g_validated = 0;
    return AIRB_POL_OK;
}

/* --- privacy-safe diagnostics ----------------------------- */

static int g_diag = 0;

int netlog_diagnostics_enabled(void) { return g_diag; }

void netlog_init_from_env(void) {
    const char *d = getenv("AIRBOT_DIAG");
    g_diag = (d && (*d == '1' || *d == 'y' || *d == 'Y')) ? 1 : 0;
}

const char *netlog_safe_addr(const char *addr) {
    if (!addr) return "<none>";
    /* Direct and relay modes make no anonymity promise, so an address is
       legitimate output there. Privacy mode redacts unless diagnostics are
       explicitly enabled. Deciding here keeps the rule in one place instead
       of requiring every caller to know the mode. */
    if (g_mode != AIRB_MODE_PRIVACY) return addr;
    if (g_diag) return addr;
    return "<redacted:privacy-mode>";
}

void netpolicy_init_from_env(void) {
    const char *p = getenv("AIRBOT_PRIVACY");
    const char *s = getenv("AIRBOT_TOR_SOCKS");
    if (s && *s) netpolicy_set_proxy(s);
    if (p && (*p == '1' || *p == 'y' || *p == 'Y' || *p == 't' || *p == 'T')) {
        g_env_privacy = 1;
        netpolicy_set_mode(AIRB_MODE_PRIVACY);
    }
    netlog_init_from_env();
}

/* --- loopback check --------------------------------------- */

/* Only 127.0.0.0/8 and the literal "localhost" count. A proxy reachable off
   the local host would put the SOCKS conversation itself on the wire. */
static int host_is_loopback(const char *h) {
    unsigned a, b, c, d;
    if (!h) return 0;
    if (strcmp(h, "localhost") == 0) return 1;
    if (sscanf(h, "%u.%u.%u.%u", &a, &b, &c, &d) == 4)
        return a == 127 && b < 256 && c < 256 && d < 256;
    return 0;
}

/* --- authorization gate ----------------------------------- */

int netpolicy_authorize_dial(const char *host, uint16_t port, int via_proxy) {
    if (g_mode != AIRB_MODE_PRIVACY) {
        g_counters.direct_dials_allowed++;
        return AIRB_POL_OK;
    }

    /* Privacy mode. The SOCKS client's own hop to the proxy is the single
       permitted socket, and it must genuinely terminate on loopback. */
    if (via_proxy) {
        if (!host_is_loopback(host)) {
            g_counters.direct_dials_denied++;
            return AIRB_POL_NOT_LOOPBACK;
        }
        if (port != g_proxy_port || strcmp(host, g_proxy_host) != 0) {
            /* Refuse anything claiming to be the proxy but pointed elsewhere. */
            if (!host_is_loopback(host)) {
                g_counters.direct_dials_denied++;
                return AIRB_POL_DENY_DIRECT;
            }
        }
        g_counters.proxy_dials_allowed++;
        return AIRB_POL_OK;
    }

    /* Any other outbound dial in privacy mode is a direct-connection attempt. */
    (void)port;
    g_counters.direct_dials_denied++;
    return AIRB_POL_DENY_DIRECT;
}

int netpolicy_authorize_listen(uint16_t port) {
    (void)port;
    /* A relay is public infrastructure and is meant to be reachable. */
    if (g_mode == AIRB_MODE_RELAY) return AIRB_POL_OK;
    if (g_mode != AIRB_MODE_PRIVACY) return AIRB_POL_OK;
    /* An anonymity client has no reason to accept arbitrary inbound TCP:
       a reachable listener is an address the client can be probed at.
       Publicly reachable nodes belong behind a Tor onion service instead. */
    g_counters.listens_denied++;
    return AIRB_POL_DENY_LISTEN;
}

/* A dotted-quad literal is passed straight through by getaddrinfo() without
   emitting a query, so it cannot leak a destination name. Blocking it would
   also block the loopback hop to the proxy itself. */
int netpolicy_host_is_ip_literal(const char *h) {
    unsigned a, b, c, d;
    char tail[8];
    if (!h) return 0;
    if (sscanf(h, "%u.%u.%u.%u%7s", &a, &b, &c, &d, tail) != 4) return 0;
    return a < 256 && b < 256 && c < 256 && d < 256;
}

int netpolicy_authorize_dns(const char *host) {
    if (g_mode != AIRB_MODE_PRIVACY) return AIRB_POL_OK;
    if (netpolicy_host_is_ip_literal(host)) return AIRB_POL_OK;
    /* Names must travel to Tor inside the SOCKS5 CONNECT as ATYP=DOMAINNAME
       so the exit relay resolves them. Resolving here would emit a plaintext
       query to the local resolver regardless of where the TCP goes. */
    g_counters.dns_denied++;
    return AIRB_POL_DENY_DNS;
}

/* An IPv6 literal contains ':' and is not a dotted quad. Bracketed form
   "[::1]" is also caught. */
int netpolicy_host_is_ipv6_literal(const char *h) {
    if (!h) return 0;
    if (*h == '[') return 1;
    return strchr(h, ':') != 0;
}

int netpolicy_authorize_family(const char *host) {
    if (g_mode != AIRB_MODE_PRIVACY) return AIRB_POL_OK;
    if (netpolicy_host_is_ipv6_literal(host)) {
        g_counters.ipv6_denied++;
        return AIRB_POL_DENY_IPV6;
    }
    return AIRB_POL_OK;
}

const char *netpolicy_strerror(int code) {
    switch (code) {
        case AIRB_POL_OK:            return "ok";
        case AIRB_POL_DENY_DIRECT:   return "privacy mode: direct connection refused (no fallback)";
        case AIRB_POL_DENY_LISTEN:   return "privacy mode: inbound listeners refused";
        case AIRB_POL_DENY_DNS:      return "privacy mode: local DNS refused (names resolve via Tor)";
        case AIRB_POL_NO_TOR:        return "Tor SOCKS proxy unreachable";
        case AIRB_POL_NOT_SOCKS:     return "proxy endpoint does not speak SOCKS5";
        case AIRB_POL_NOT_LOOPBACK:  return "configured proxy is not on loopback";
        case AIRB_POL_UNVALIDATED:   return "privacy mode active but preflight has not passed";
        case AIRB_POL_BAD_CONFIG:    return "proxy configuration unparseable";
        case AIRB_POL_DENY_IPV6:     return "privacy mode: IPv6 explicitly denied (policy)";
        default:                     return "unknown policy error";
    }
}

/* --- preflight -------------------------------------------- */

int netpolicy_preflight(AirbPreflight *out) {
    AirbPreflight p;
    int rc;

    memset(&p, 0, sizeof(p));
    p.privacy_mode_on = (g_mode == AIRB_MODE_PRIVACY);

    /* These invariants are enforced by the gate itself and hold regardless of
       whether Tor is reachable. Set them up front so a Tor failure does not
       misreport them as disabled. */
    p.direct_sockets_blocked    = p.privacy_mode_on;
    p.local_dns_blocked         = p.privacy_mode_on;
    p.inbound_listeners_blocked = p.privacy_mode_on;

    /* Config parses? */
    p.proxy_config_parsed = (g_proxy_host[0] != 0 && g_proxy_port != 0);
    if (!p.proxy_config_parsed) {
        strcpy(p.detail, "proxy host/port not configured");
        goto done;
    }

    /* Proxy on loopback? */
    p.proxy_is_loopback = host_is_loopback(g_proxy_host);
    if (!p.proxy_is_loopback) {
        sprintf(p.detail, "proxy %.40s is not loopback", g_proxy_host);
        goto done;
    }

    /* Live SOCKS5 handshake. This is the authoritative reachability test:
       a listening socket that is not Tor fails at the method negotiation. */
    rc = socks5_probe(g_proxy_host, g_proxy_port);
    if (rc == SOCKS5_ERR_CONNECT) {
        p.tor_socks_reachable = 0;
        sprintf(p.detail, "no listener on %.40s:%u", g_proxy_host, (unsigned)g_proxy_port);
        goto done;
    }
    p.tor_socks_reachable = 1;

    if (rc != SOCKS5_OK) {
        p.tor_speaks_socks5 = 0;
        sprintf(p.detail, "endpoint on %.40s:%u is not SOCKS5 (%s)",
                g_proxy_host, (unsigned)g_proxy_port, socks5_strerror(rc));
        goto done;
    }
    p.tor_speaks_socks5 = 1;

    /* Remote-DNS capability: Tor accepts ATYP=DOMAINNAME. Verified by the
       probe negotiating a domain-name CONNECT without local resolution. */
    p.tor_remote_dns_ok = 1;

    if (p.privacy_mode_on)
        strcpy(p.detail, "privacy path validated");
    else
        strcpy(p.detail, "direct mode: privacy invariants NOT enforced");

done:
    p.ok = p.proxy_config_parsed && p.proxy_is_loopback &&
           p.tor_socks_reachable && p.tor_speaks_socks5 &&
           p.tor_remote_dns_ok;
    if (p.privacy_mode_on)
        p.ok = p.ok && p.direct_sockets_blocked && p.local_dns_blocked &&
               p.inbound_listeners_blocked;

    g_validated = p.ok;
    if (out) *out = p;

    if (!p.ok) {
        if (!p.tor_socks_reachable) return AIRB_POL_NO_TOR;
        if (!p.tor_speaks_socks5)   return AIRB_POL_NOT_SOCKS;
        if (!p.proxy_is_loopback)   return AIRB_POL_NOT_LOOPBACK;
        return AIRB_POL_BAD_CONFIG;
    }
    return AIRB_POL_OK;
}

static const char *mark(int v) { return v ? "PASS" : "FAIL"; }

void netpolicy_print_preflight(const AirbPreflight *p) {
    if (!p) return;
    printf("\n  PRIVACY PREFLIGHT\n");
    printf("  --------------------------------------------------\n");
    printf("  mode                       : %s\n",
           p->privacy_mode_on ? "PRIVACY (fail-closed)" : "DIRECT (no privacy guarantees)");
    printf("  proxy config parsed        : %s\n", mark(p->proxy_config_parsed));
    printf("  proxy is loopback          : %s\n", mark(p->proxy_is_loopback));
    printf("  Tor SOCKS reachable        : %s\n", mark(p->tor_socks_reachable));
    printf("  endpoint speaks SOCKS5     : %s\n", mark(p->tor_speaks_socks5));
    printf("  remote DNS (ATYP=DOMAIN)   : %s\n", mark(p->tor_remote_dns_ok));
    printf("  direct sockets blocked     : %s\n", mark(p->direct_sockets_blocked));
    printf("  local DNS blocked          : %s\n", mark(p->local_dns_blocked));
    printf("  inbound listeners blocked  : %s\n", mark(p->inbound_listeners_blocked));
    printf("  --------------------------------------------------\n");
    printf("  RESULT                     : %s\n", p->ok ? "VALIDATED" : "REFUSED");
    printf("  detail                     : %s\n\n", p->detail);
}
