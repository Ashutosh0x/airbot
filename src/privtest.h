/*
 * Airbot — Executable Information System
 * privtest.h — Privacy invariant and leak regression harness
 */
#ifndef AIRBOT_PRIVTEST_H
#define AIRBOT_PRIVTEST_H

/* Returns 0 if every privacy invariant holds, 1 otherwise.
   Suitable as a build gate. */
int privtest_run_all(void);

#endif /* AIRBOT_PRIVTEST_H */
