/*
 * Airbot — Executable Information System
 * advtest.h — Adversarial / red-team regression suite
 */
#ifndef AIRBOT_ADVTEST_H
#define AIRBOT_ADVTEST_H

/* 0 if every modelled defence held. Residual attacks are reported in the
   output whether or not they cause a non-zero exit. */
int advtest_run_all(void);

#endif /* AIRBOT_ADVTEST_H */
