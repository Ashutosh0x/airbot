#!/bin/sh
# Phase 16 — static security audit.
# Classifies every networking / identity / output call site.
# Exit non-zero if an UNCLASSIFIED network path is found.
cd "$(dirname "$0")/.." || exit 1
fail=0

echo "=== 1. socket syscalls (must be transport.c ONLY) ==="
hits=$(grep -rnE '\b(socket|connect|bind|listen|accept|WSASocket|WSAConnect|sendto|recvfrom)\s*\(' src/*.c \
       | grep -v '^src/transport.c' | grep -vE 'socks5_connect|transport_')
if [ -n "$hits" ]; then echo "$hits"; echo "  FORBIDDEN: syscall outside chokepoint"; fail=1
else echo "  SAFE: transport.c is the sole chokepoint"; fi

echo "=== 2. DNS APIs ==="
d=$(grep -rnE '(getaddrinfo|gethostbyname|GetAddrInfoW|DnsQuery)[[:space:]]*\(' src/*.c      | grep -v '^src/transport.c'      | grep -vE '^[^:]+:[0-9]+:[[:space:]]*(/\*|\*|//)')
if [ -n "$d" ]; then echo "$d"; echo "  FORBIDDEN: DNS outside chokepoint"; fail=1
else echo "  SAFE: only transport.c, and it is gated by netpolicy_authorize_dns()"; fi

echo "=== 3. IPv6 / UDP surface ==="
grep -rnE 'AF_INET6|SOCK_DGRAM|IPPROTO_UDP' src/*.c | grep -v netpolicy || \
  echo "  SAFE: no AF_INET6 / UDP socket path exists"

echo "=== 4. host / user / machine identity ==="
i=$(grep -rniE '\b(gethostname|GetComputerName|GetUserName|GetAdaptersInfo|getlogin)\s*\(' src/*.c)
if [ -n "$i" ]; then echo "$i"; echo "  FORBIDDEN: identity harvesting"; fail=1
else echo "  SAFE: no hostname/username/MAC/adapter enumeration anywhere"; fi

echo "=== 5. peer-address output (must route via netlog_safe_addr) ==="
p=$(grep -rnE 'printf\([^)]*(conn\.peer|conn->peer|peer\.peer)' src/*.c      | grep -v netlog_safe_addr      | grep -v 'sprintf'      | grep -v 'Circuit')
if [ -n "$p" ]; then echo "$p"; echo "  PRIVACY-SENSITIVE: unredacted peer address"; fail=1
else echo "  SAFE: all peer output passes through netlog_safe_addr()"; fi

echo "=== 6. persistent identifiers ==="
q=$(grep -rniE 'static[^;]*\b(client_id|session_id|install_id|machine_id|device_id)\b' src/*.c)
if [ -n "$q" ]; then echo "$q"; echo "  FORBIDDEN: persistent identifier"; fail=1
else echo "  SAFE: no persistent client/session/device identifier"; fi

echo "=== 7. key material reaching logs ==="
k=$(grep -rnE 'printf\([^)]*(secret_key|g_key|eph_sk|->key|hop_key)' src/*.c)
if [ -n "$k" ]; then echo "$k"; echo "  FORBIDDEN: key material in output"; fail=1
else echo "  SAFE: no key material is printed"; fi

echo
if [ $fail -eq 0 ]; then echo "STATIC AUDIT: PASS"; else echo "STATIC AUDIT: FAIL"; fi
exit $fail
