/*
 * Airbot — Executable Information System
 * main.c — CLI Entry Point
 *
 * The command-line interface to the entire Airbot system.
 * Pure C99. Zero external dependencies.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "eiu.h"
#include "eia.h"
#include "vm.h"
#include "verifier.h"
#include "environment.h"
#include "state.h"
#include "replicator.h"
#include "assembler.h"
#include "disassembler.h"
#include "metrics.h"
#include "experiments.h"
#include "matrix.h"
#include "blake3.h"
#include "visibility.h"
#include "privacy.h"
#include "onion.h"
#include "benchmark.h"
#include "crypto_test.h"

/* ═══════════════════════════════════════════════════════════════
 * Banner & Help
 * ═══════════════════════════════════════════════════════════════ */

static void print_banner(void) {
    printf("\n");
    printf("  ╔═══════════════════════════════════════════════════════╗\n");
    printf("  ║                                                       ║\n");
    printf("  ║     █████╗ ██╗██████╗ ██████╗  ██████╗ ████████╗     ║\n");
    printf("  ║    ██╔══██╗██║██╔══██╗██╔══██╗██╔═══██╗╚══██╔══╝     ║\n");
    printf("  ║    ███████║██║██████╔╝██████╔╝██║   ██║   ██║        ║\n");
    printf("  ║    ██╔══██║██║██╔══██╗██╔══██╗██║   ██║   ██║        ║\n");
    printf("  ║    ██║  ██║██║██║  ██║██████╔╝╚██████╔╝   ██║        ║\n");
    printf("  ║    ╚═╝  ╚═╝╚═╝╚═╝  ╚═╝╚═════╝  ╚═════╝    ╚═╝        ║\n");
    printf("  ║                                                       ║\n");
    printf("  ║    Executable Information System v0.1                  ║\n");
    printf("  ║    Custom-Built from Scratch — Zero Dependencies      ║\n");
    printf("  ║                                                       ║\n");
    printf("  ╚═══════════════════════════════════════════════════════╝\n");
    printf("\n");
}

static void print_usage(void) {
    printf("Usage: airbot <command> [options]\n\n");
    printf("Commands:\n");
    printf("  assemble <input.airasm> -o <output.eiu>   Assemble source to binary\n");
    printf("  disassemble <input.eiu>                    Disassemble binary to text\n");
    printf("  run <input.eiu> [--fuel N] [--trace]       Execute a single EIU\n");
    printf("  verify <input.eiu>                         Static verification\n");
    printf("  evolve <input.eiu> --steps N               State evolution\n");
    printf("  replicate <input.eiu> --count N            Test replication\n");
    printf("  experiment --hypothesis H1|H2|H3|ALL       Run experiments\n");
    printf("  metrics <input.eiu>                        Compute metrics\n");
    printf("  matrix --population N --steps T            Population simulation\n");
    printf("  visibility [--observer ISP|DPI|PASSIVE]     Visibility analysis\n");
    printf("  privacy [--observer ISP|DPI|PASSIVE]        Multi-protocol privacy comparison\n");
    printf("  onion                                      Onion routing simulation\n");
    printf("  benchmark                                  Empirical Tor vs Airbot comparison\n");
    printf("  info                                       System information\n");
    printf("  help                                       Show this help\n");
    printf("\n");
}

/* ═══════════════════════════════════════════════════════════════
 * File I/O Helpers
 * ═══════════════════════════════════════════════════════════════ */

static int read_file(const char *path, uint8_t *buf, size_t cap, size_t *out_len) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "Error: cannot open '%s'\n", path);
        return -1;
    }
    *out_len = fread(buf, 1, cap, f);
    fclose(f);
    return 0;
}

static int write_file(const char *path, const uint8_t *buf, size_t len) {
    FILE *f = fopen(path, "wb");
    if (!f) {
        fprintf(stderr, "Error: cannot create '%s'\n", path);
        return -1;
    }
    fwrite(buf, 1, len, f);
    fclose(f);
    return 0;
}

static int read_text_file(const char *path, char *buf, size_t cap, size_t *out_len) {
    FILE *f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "Error: cannot open '%s'\n", path);
        return -1;
    }
    *out_len = fread(buf, 1, cap - 1, f);
    buf[*out_len] = '\0';
    fclose(f);
    return 0;
}

/* ═══════════════════════════════════════════════════════════════
 * Command Handlers
 * ═══════════════════════════════════════════════════════════════ */

static int cmd_assemble(int argc, char **argv) {
    if (argc < 4) {
        fprintf(stderr, "Usage: airbot assemble <input.airasm> -o <output.eiu>\n");
        return 1;
    }

    const char *input = argv[2];
    const char *output = NULL;

    for (int i = 3; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            output = argv[i + 1];
            i++;
        }
    }

    if (!output) {
        fprintf(stderr, "Error: -o <output> required\n");
        return 1;
    }

    /* Read source file */
    char source[16384];
    size_t source_len;
    if (read_text_file(input, source, sizeof(source), &source_len) != 0) {
        return 1;
    }

    /* Assemble */
    AssemblerState as;
    asm_init(&as);
    int rc = asm_assemble(&as, source, source_len);
    if (rc != 0) {
        fprintf(stderr, "Assembly error at line %d: %s\n",
                as.error_line, as.error_msg);
        return 1;
    }

    /* Create EIU with assembled bytecode */
    EIU eiu;
    eiu_init(&eiu);
    eiu_set_fuel(&eiu, as.fuel > 0 ? as.fuel : 1000);
    eiu_set_behavior(&eiu, as.output, as.output_len);

    if (as.capabilities != 0) {
        Capability cap;
        cap_init(&cap, as.capabilities);
        eiu_set_capability(&eiu, &cap);
    }

    /* Serialize */
    uint8_t buf[EIU_MAX_SIZE];
    size_t out_len;
    rc = eiu_serialize(&eiu, buf, sizeof(buf), &out_len);
    if (rc != 0) {
        fprintf(stderr, "Serialization error\n");
        return 1;
    }

    /* Write output */
    if (write_file(output, buf, out_len) != 0) {
        return 1;
    }

    printf("  Assembled: %s -> %s\n", input, output);
    printf("  Instructions: %u\n", as.instruction_count);
    printf("  Bytecode: %u bytes\n", as.output_len);
    printf("  EIU size: %zu bytes (%zu bits)\n", out_len, out_len * 8);
    printf("  Fuel: %u\n", as.fuel > 0 ? as.fuel : 1000);

    return 0;
}

static int cmd_disassemble(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: airbot disassemble <input.eiu>\n");
        return 1;
    }

    uint8_t buf[EIU_MAX_SIZE];
    size_t len;
    if (read_file(argv[2], buf, sizeof(buf), &len) != 0) return 1;

    EIU eiu;
    eiu_init(&eiu);
    int rc = eiu_deserialize(&eiu, buf, len);
    if (rc != 0) {
        fprintf(stderr, "Error: invalid EIU file\n");
        return 1;
    }

    DisassemblerState ds;
    disasm_init(&ds);
    disasm_disassemble(&ds, eiu.behavior, eiu.behavior_len);

    printf("  ; Disassembly of %s\n", argv[2]);
    printf("  ; EIU version: %u, fuel: %u, size: %zu bytes\n\n",
           eiu.version, eiu.fuel, len);
    printf("%s", ds.output);

    return 0;
}

static int cmd_run(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: airbot run <input.eiu> [--fuel N] [--trace]\n");
        return 1;
    }

    uint16_t fuel_override = 0;
    int trace = 0;

    for (int i = 3; i < argc; i++) {
        if (strcmp(argv[i], "--fuel") == 0 && i + 1 < argc) {
            fuel_override = (uint16_t)atoi(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "--trace") == 0) {
            trace = 1;
        }
    }

    uint8_t buf[EIU_MAX_SIZE];
    size_t len;
    if (read_file(argv[2], buf, sizeof(buf), &len) != 0) return 1;

    EIU eiu;
    eiu_init(&eiu);
    if (eiu_deserialize(&eiu, buf, len) != 0) {
        fprintf(stderr, "Error: invalid EIU file\n");
        return 1;
    }

    if (fuel_override > 0) eiu.fuel = fuel_override;

    Environment env;
    env_init(&env);

    ExecutionResult exec;
    exec_result_init(&exec);
    int rc = env_execute(&env, &eiu, &exec);

    printf("  Execution %s\n", exec.success ? "SUCCEEDED" : "FAILED");
    printf("  Fuel: %u used / %u budget\n", exec.fuel_consumed, eiu.fuel);
    printf("  Instructions: %u\n", exec.instructions_executed);
    printf("  Outputs: %u\n", exec.output_count);

    for (int i = 0; i < exec.output_count; i++) {
        printf("    [%d] = %u (0x%04X)\n", i, exec.output[i], exec.output[i]);
    }

    if (exec.spawn_pending) {
        printf("  Spawn requested: %u bytes\n", exec.spawn_len);
    }

    if (rc != 0) {
        printf("  Error code: %d\n", exec.error_code);
    }

    (void)trace; /* TODO: implement trace mode */
    return 0;
}

static int cmd_verify(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: airbot verify <input.eiu>\n");
        return 1;
    }

    uint8_t buf[EIU_MAX_SIZE];
    size_t len;
    if (read_file(argv[2], buf, sizeof(buf), &len) != 0) return 1;

    EIU eiu;
    eiu_init(&eiu);
    if (eiu_deserialize(&eiu, buf, len) != 0) {
        fprintf(stderr, "Error: invalid EIU file\n");
        return 1;
    }

    VerificationResult vr;
    verifier_analyze(eiu.behavior, eiu.behavior_len,
                     eiu.capability.rights, &vr);

    printf("  Verification: %s\n", vr.passed ? "PASSED" : "FAILED");
    printf("  Checks: %d total, %d failed\n", vr.total_checks, vr.failed_checks);
    printf("  Instructions: %u\n", vr.instruction_count);
    printf("  Max stack depth: %u\n", vr.max_stack_depth);
    printf("  Has HALT: %s\n", vr.has_halt ? "yes" : "no");
    printf("  Uses SPAWN: %s\n", vr.uses_spawn ? "yes" : "no");
    printf("  Uses SELF: %s\n", vr.uses_self ? "yes" : "no");
    printf("  Required caps: 0x%llX\n", (unsigned long long)vr.required_caps);

    for (int i = 0; i < vr.message_count; i++) {
        printf("  [%d] %s\n", i + 1, vr.messages[i]);
    }

    return vr.passed ? 0 : 1;
}

static int cmd_evolve(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: airbot evolve <input.eiu> --steps N\n");
        return 1;
    }

    uint16_t steps = 50;
    for (int i = 3; i < argc; i++) {
        if (strcmp(argv[i], "--steps") == 0 && i + 1 < argc) {
            steps = (uint16_t)atoi(argv[i + 1]);
            i++;
        }
    }

    uint8_t buf[EIU_MAX_SIZE];
    size_t len;
    if (read_file(argv[2], buf, sizeof(buf), &len) != 0) return 1;

    EIU eiu;
    eiu_init(&eiu);
    if (eiu_deserialize(&eiu, buf, len) != 0) {
        fprintf(stderr, "Error: invalid EIU file\n");
        return 1;
    }

    Environment env;
    env_init(&env);

    EvolutionTrace trace;
    trace_init(&trace);

    int rc = state_evolve(&eiu, &env, steps, &trace);
    trace_detect_fixed_point(&trace);
    trace_detect_cycle(&trace);
    trace_print_summary(&trace);

    (void)rc;
    return 0;
}

static int cmd_replicate(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: airbot replicate <input.eiu> --count N\n");
        return 1;
    }

    uint16_t count = 5;
    for (int i = 3; i < argc; i++) {
        if (strcmp(argv[i], "--count") == 0 && i + 1 < argc) {
            count = (uint16_t)atoi(argv[i + 1]);
            i++;
        }
    }

    uint8_t buf[EIU_MAX_SIZE];
    size_t len;
    if (read_file(argv[2], buf, sizeof(buf), &len) != 0) return 1;

    EIU eiu;
    eiu_init(&eiu);
    if (eiu_deserialize(&eiu, buf, len) != 0) {
        fprintf(stderr, "Error: invalid EIU file\n");
        return 1;
    }

    Environment env;
    env_init(&env);
    env_set_capabilities(&env, CAP_ALL);

    ReplicationResult repl;
    repl_result_init(&repl);
    repl_execute(&eiu, &env, count, &repl);
    repl_print_summary(&repl);

    return 0;
}

static int cmd_experiment(int argc, char **argv) {
    const char *hyp = "ALL";
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--hypothesis") == 0 && i + 1 < argc) {
            hyp = argv[i + 1];
            i++;
        }
    }

    ExperimentResult r;

    if (strcmp(hyp, "H1") == 0) {
        experiment_h1(256, 32, &r);
        experiment_print_result(&r);
    } else if (strcmp(hyp, "H2") == 0) {
        experiment_h2(100, 10, &r);
        experiment_print_result(&r);
    } else if (strcmp(hyp, "H3") == 0) {
        experiment_h3(5, &r);
        experiment_print_result(&r);
    } else {
        experiments_run_all();
    }

    return 0;
}

static int cmd_metrics(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: airbot metrics <input.eiu>\n");
        return 1;
    }

    uint8_t buf[EIU_MAX_SIZE];
    size_t len;
    if (read_file(argv[2], buf, sizeof(buf), &len) != 0) return 1;

    EIU eiu;
    eiu_init(&eiu);
    if (eiu_deserialize(&eiu, buf, len) != 0) {
        fprintf(stderr, "Error: invalid EIU file\n");
        return 1;
    }

    EIUMetrics m;
    metrics_init(&m);
    metrics_analyze(&eiu, &m);
    metrics_print(&m);

    return 0;
}

static int cmd_matrix(int argc, char **argv) {
    uint16_t population = 10;
    uint16_t steps = 100;

    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--population") == 0 && i + 1 < argc) {
            population = (uint16_t)atoi(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "--steps") == 0 && i + 1 < argc) {
            steps = (uint16_t)atoi(argv[i + 1]);
            i++;
        }
    }

    /* Create a template EIU: counter that increments state */
    EIU template_eiu;
    eiu_init(&template_eiu);
    eiu_set_fuel(&template_eiu, 100);

    uint8_t code[10];
    uint16_t instr;

    instr = vm_encode_instruction(OP_LOAD, 0, 7, 0);
    code[0] = (uint8_t)(instr >> 8); code[1] = (uint8_t)(instr & 0xFF);
    instr = vm_encode_immediate(OP_ADDI, 0, 1);
    code[2] = (uint8_t)(instr >> 8); code[3] = (uint8_t)(instr & 0xFF);
    instr = vm_encode_instruction(OP_STORE, 0, 7, 0);
    code[4] = (uint8_t)(instr >> 8); code[5] = (uint8_t)(instr & 0xFF);
    instr = vm_encode_instruction(OP_EMIT, 0, 0, 0);
    code[6] = (uint8_t)(instr >> 8); code[7] = (uint8_t)(instr & 0xFF);
    instr = vm_encode_jump(OP_HALT, 0);
    code[8] = (uint8_t)(instr >> 8); code[9] = (uint8_t)(instr & 0xFF);

    eiu_set_behavior(&template_eiu, code, 10);

    uint8_t state[4] = {0, 0, 0, 0};
    eiu_set_state(&template_eiu, state, 4);

    Environment env;
    env_init(&env);

    PopulationMatrix mat;
    matrix_init(&mat, population, &template_eiu, &env);
    matrix_run(&mat, steps);
    matrix_print_report(&mat);

    return 0;
}

static int cmd_visibility(int argc, char **argv) {
    /* Parse observer type */
    const char *obs_type = "ISP"; /* Default */
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--observer") == 0 && i + 1 < argc) {
            obs_type = argv[i + 1];
            i++;
        }
    }

    /* Create observer */
    NetworkObserver obs;
    if (strcmp(obs_type, "DPI") == 0 || strcmp(obs_type, "dpi") == 0) {
        vis_observer_dpi(&obs);
    } else if (strcmp(obs_type, "PASSIVE") == 0 || strcmp(obs_type, "passive") == 0) {
        vis_observer_passive(&obs);
    } else {
        vis_observer_isp(&obs);
    }

    printf("\n  Observer Type: %s\n", obs_type);

    /* Create a sample EIU and EIA for analysis */
    EIU eiu;
    eiu_init(&eiu);
    eiu_set_fuel(&eiu, 100);

    uint8_t code[2];
    uint16_t instr = vm_encode_jump(OP_HALT, 0);
    code[0] = (uint8_t)(instr >> 8);
    code[1] = (uint8_t)(instr & 0xFF);
    eiu_set_behavior(&eiu, code, 2);

    EIA eia;
    eia_init(&eia);
    uint8_t digest[32] = {0};
    eia_set_digest(&eia, digest);
    eia_set_capability(&eia, CAP_READ | CAP_EXECUTE);
    eia_set_bytecode(&eia, code, 2);

    /* Run comparison */
    VisibilityAnalysis airbot_va, conv_va;
    vis_compare(&eiu, &eia, &obs, &airbot_va, &conv_va);
    vis_print_comparison(&airbot_va, &conv_va);

    /* Print the fundamental equations */
    printf("  ┌─── MATHEMATICAL FORMULATION ─────────────────────────────\n");
    printf("  │\n");
    printf("  │  Observability Equation:\n");
    printf("  │    O(A,N) = 1 - Product_i(1 - P_detect(C_i))\n");
    printf("  │\n");
    printf("  │  For Airbot (I_A = 0, no IP address):\n");
    printf("  │    P_address = 0  (channel eliminated in model)\n");
    printf("  │    P_route   = 0  (channel eliminated in model)\n");
    printf("  │    O(Airbot) = 1 - Product(1-P_traffic)(1-P_timing)...\n");
    printf("  │    O(Airbot) = %.6f\n", airbot_va.observability);
    printf("  │\n");
    printf("  │  Identity-Location Decoupling (structural property):\n");
    printf("  │    Identity(Airbot) != Location(Airbot)\n");
    printf("  │    kappa = %.4f (0 = decoupled in model)\n",
           airbot_va.identity_location_coupling);
    printf("  │\n");
    printf("  │  Address Efficiency (structural property):\n");
    printf("  │    eta = identity_cap_bits / total_address_bits\n");
    printf("  │    eta(IP)     = %.4f  (IP carries no identity info)\n",
           conv_va.address_efficiency);
    printf("  │    eta(Airbot) = %.4f  (EIA carries identity+capability)\n",
           airbot_va.address_efficiency);
    printf("  │\n");
    printf("  │  Estimated non-detection probability (under this model):\n");
    printf("  │    P(non-detect|IPv4)   = %.6f (%.2f%%)\n",
           conv_va.invisibility, conv_va.invisibility * 100.0);
    printf("  │    P(non-detect|Airbot) = %.6f (%.2f%%)\n",
           airbot_va.invisibility, airbot_va.invisibility * 100.0);
    printf("  │\n");
    printf("  │  These are modeled estimates. Signal parameters and\n");
    printf("  │  observer accuracies are defined in visibility.h/.c.\n");
    printf("  └──────────────────────────────────────────────────────────\n\n");

    return 0;
}

static int cmd_privacy(int argc, char **argv) {
    /* Parse observer type */
    const char *obs_type = "ISP";
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--observer") == 0 && i + 1 < argc) {
            obs_type = argv[i + 1];
            i++;
        }
    }

    NetworkObserver obs;
    if (strcmp(obs_type, "DPI") == 0 || strcmp(obs_type, "dpi") == 0) {
        vis_observer_dpi(&obs);
    } else if (strcmp(obs_type, "PASSIVE") == 0 || strcmp(obs_type, "passive") == 0) {
        vis_observer_passive(&obs);
    } else {
        vis_observer_isp(&obs);
    }

    printf("\n  Observer Model: %s\n", obs_type);
    printf("  Comparing: IPv4 vs IPv6 vs NDN vs Tor vs Airbot\n");

    ProtocolComparison cmp;
    priv_init_profiles(&cmp);
    priv_compare_all(&cmp, &obs);
    priv_print_comparison(&cmp);

    return 0;
}

static int cmd_onion(void) {
    printf("\n");
    printf("  ╔════════════════════════════════════════════════════════════════╗\n");
    printf("  ║  AIRBOT + ONION ROUTING SIMULATION                           ║\n");
    printf("  ║  Sphinx-like 3-hop circuit with per-hop capability auth       ║\n");
    printf("  ║  ChaCha20-Poly1305 AEAD — research prototype                 ║\n");
    printf("  ╚════════════════════════════════════════════════════════════════╝\n");

    /* Create a 3-relay circuit */
    OnionCircuit circuit;
    onion_circuit_init(&circuit, 3);

    /* Generate relay keys deterministically */
    uint8_t seed[] = "airbot-onion-experiment-seed-2026";
    onion_generate_relays(&circuit, seed, sizeof(seed) - 1);

    printf("\n  Circuit Configuration:\n");
    onion_print_circuit(&circuit);

    /* Create a sample EIU payload */
    uint8_t eiu_data[64];
    for (int i = 0; i < 64; i++) {
        eiu_data[i] = (uint8_t)(i ^ 0x42);
    }

    printf("\n  Running simulation with 64-byte EIU payload...\n");
    int result = onion_simulate(&circuit, eiu_data, 64);

    /* Compute and print privacy metrics */
    OnionPrivacyMetrics metrics;
    onion_compute_metrics(&circuit, &metrics);
    printf("\n");
    onion_print_metrics(&metrics);

    printf("\n  ┌─── What Each Relay Sees ─────────────────────────────────────\n");
    printf("  │\n");
    printf("  │  Relay 1 (Entry):  Source IP, encrypted blob\n");
    printf("  │                    CANNOT see: destination, Airbot identity, state, payload\n");
    printf("  │\n");
    printf("  │  Relay 2 (Middle): Encrypted blob from Relay 1\n");
    printf("  │                    CANNOT see: source, destination, Airbot identity\n");
    printf("  │\n");
    printf("  │  Relay 3 (Exit):   Encrypted blob from Relay 2, destination gateway\n");
    printf("  │                    CANNOT see: source, Airbot identity (still encrypted)\n");
    printf("  │\n");
    printf("  │  Destination:      Decrypted Airbot → full verification → EIA execution\n");
    printf("  │                    CAN see: Airbot identity, state, code, capabilities\n");
    printf("  │\n");
    printf("  │  KEY PROPERTY: No single relay knows both source AND destination.\n");
    printf("  │  This is the same principle as Tor's circuit-based anonymity.\n");
    printf("  │\n");
    printf("  └──────────────────────────────────────────────────────────────\n\n");

    if (result == 0) {
        printf("  Result: ONION ROUTING SIMULATION PASSED\n\n");
    } else {
        printf("  Result: ONION ROUTING SIMULATION FAILED\n\n");
    }

    return result;
}

static int cmd_benchmark(void) {
    printf("\n");
    printf("  ╔════════════════════════════════════════════════════════════════╗\n");
    printf("  ║  EMPIRICAL PRIVACY BENCHMARK                                 ║\n");
    printf("  ║  IPv4 vs Tor vs Airbot vs Airbot+Onion                       ║\n");
    printf("  ║  Classifier-based detectability + entry/exit correlation      ║\n");
    printf("  ╚════════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    printf("  Hypothesis:\n");
    printf("    H0: Airbot+Onion does NOT outperform Tor on privacy metrics\n");
    printf("    H1: Airbot+Onion improves on >= 1 dimension while retaining\n");
    printf("        executable routing, state evolution, capability auth\n");
    printf("\n");

    BenchmarkComparison *cmp = (BenchmarkComparison *)malloc(sizeof(BenchmarkComparison));
    if (!cmp) {
        printf("  ERROR: Failed to allocate benchmark memory\n");
        return -1;
    }
    memset(cmp, 0, sizeof(BenchmarkComparison));
    bench_run_comparison(cmp);
    bench_print_scorecard(cmp);
    free(cmp);

    return 0;
}

static int cmd_info(void) {
    printf("  Airbot Executable Information System v0.1\n\n");
    printf("  Architecture:\n");
    printf("    VM:           Airbot Bit Machine (ABM)\n");
    printf("    Word size:    16-bit\n");
    printf("    Registers:    8 x 16-bit (R0-R7)\n");
    printf("    Opcodes:      32 (5-bit encoding)\n");
    printf("    Instruction:  16-bit fixed width\n");
    printf("    Memory:       4096 bytes\n");
    printf("    Stack:        64 entries (data) + 32 entries (return)\n");
    printf("    Gas metering: 16-bit fuel counter\n\n");
    printf("  Wire Format:\n");
    printf("    EIU magic:    0xAB01\n");
    printf("    EIA magic:    0xEA\n");
    printf("    Encoding:     TLV with prefix varints\n");
    printf("    Hash:         BLAKE3-256\n\n");
    printf("  Capabilities:\n");
    printf("    READ, WRITE, EXECUTE, REPLICATE,\n");
    printf("    MUTATE, NETWORK, SPAWN, ADMIN, SELF_READ\n\n");
    printf("  Built with:     Pure C99 — zero dependencies\n");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════
 * Main Entry Point
 * ═══════════════════════════════════════════════════════════════ */

int main(int argc, char **argv) {
    if (argc < 2) {
        print_banner();
        print_usage();
        return 0;
    }

    const char *cmd = argv[1];

    if (strcmp(cmd, "assemble") == 0)     return cmd_assemble(argc, argv);
    if (strcmp(cmd, "disassemble") == 0)  return cmd_disassemble(argc, argv);
    if (strcmp(cmd, "run") == 0)          return cmd_run(argc, argv);
    if (strcmp(cmd, "verify") == 0)       return cmd_verify(argc, argv);
    if (strcmp(cmd, "evolve") == 0)       return cmd_evolve(argc, argv);
    if (strcmp(cmd, "replicate") == 0)    return cmd_replicate(argc, argv);
    if (strcmp(cmd, "experiment") == 0)   return cmd_experiment(argc, argv);
    if (strcmp(cmd, "metrics") == 0)      return cmd_metrics(argc, argv);
    if (strcmp(cmd, "matrix") == 0)       return cmd_matrix(argc, argv);
    if (strcmp(cmd, "visibility") == 0)  return cmd_visibility(argc, argv);
    if (strcmp(cmd, "privacy") == 0)     return cmd_privacy(argc, argv);
    if (strcmp(cmd, "onion") == 0)       return cmd_onion();
    if (strcmp(cmd, "benchmark") == 0)   return cmd_benchmark();
    if (strcmp(cmd, "crypto-test") == 0) return crypto_test_run_all();
    if (strcmp(cmd, "info") == 0)         return cmd_info();
    if (strcmp(cmd, "help") == 0)         { print_usage(); return 0; }

    fprintf(stderr, "Unknown command: %s\n", cmd);
    print_usage();
    return 1;
}
