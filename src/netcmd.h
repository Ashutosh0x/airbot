/*
 * Airbot — Executable Information System
 * netcmd.h — CLI commands backed by the real network transport
 */
#ifndef AIRBOT_NETCMD_H
#define AIRBOT_NETCMD_H

/* DNS resolve + TCP connect to a real host, with timing. */
int cmd_net_probe(int argc, char **argv);

/* Round-trip a serialized EIU through a foreign echo server and verify it. */
int cmd_net_echo(int argc, char **argv);

/* Run a listening relay that receives and optionally forwards Airbot frames. */
int cmd_relay(int argc, char **argv);

/* Send one framed EIU to a listening relay. */
int cmd_net_send(int argc, char **argv);

/* Validate the privacy path. Non-zero exit if it would not be safe. */
int cmd_privacy_preflight(int argc, char **argv);

/* Reachability probe that egresses only through Tor, or fails closed. */
int cmd_privacy_probe(int argc, char **argv);

/* End-to-end fetch through Tor; prints what the destination observed. */
int cmd_privacy_fetch(int argc, char **argv);

#endif /* AIRBOT_NETCMD_H */
