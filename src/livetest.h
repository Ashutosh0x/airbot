/*
 * Airbot — Executable Information System
 * livetest.h — Live socket integration test of the production onion path
 */
#ifndef AIRBOT_LIVETEST_H
#define AIRBOT_LIVETEST_H

/* Runs a real 3-relay chain over loopback sockets using the production
   send/forward/peel functions, and inspects the actual wire bytes.
   Returns 0 only if the live path is onion-protected. */
int livetest_run_all(void);

#endif /* AIRBOT_LIVETEST_H */
