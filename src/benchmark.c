#include "benchmark.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static uint32_t prng_state = 123456789;
static uint32_t next_rand() {
    prng_state = (1103515245 * prng_state + 12345) & 0x7fffffff;
    return prng_state;
}
static double rand_double() {
    return (double)next_rand() / (double)0x7fffffff;
}
static double rand_normal() {
    // Box-Muller
    double u1 = rand_double();
    double u2 = rand_double();
    if(u1 < 1e-9) u1 = 1e-9;
    return sqrt(-2.0 * log(u1)) * cos(2.0 * 3.1415926535 * u2);
}

void bench_generate_background(FlowTrace *traces, uint16_t num_traces, uint32_t seed) {
    prng_state = seed;
    for (uint16_t i = 0; i < num_traces; i++) {
        traces[i].protocol_label = 0;
        uint16_t np = 50 + (next_rand() % 950); // 50 to 1000
        traces[i].num_packets = np;
        
        double current_time = 0.0;
        uint32_t tot_bytes = 0;
        for (uint16_t p = 0; p < np; p++) {
            // Bursty: 3-10 packet bursts, 0.5-5s gaps
            if (p > 0 && p % (3 + (next_rand() % 8)) == 0) {
                current_time += 0.5 + (rand_double() * 4.5);
            } else {
                current_time += 0.001 + (rand_double() * 0.01);
            }
            traces[i].packets[p].timestamp = current_time;
            
            // Sizes: 200-1500 bytes, heavy-tailed
            double sz = 200.0 + (rand_double() * 1300.0);
            if (rand_double() < 0.1) sz = 1500;
            traces[i].packets[p].size = (uint16_t)sz;
            traces[i].packets[p].direction = (rand_double() < 0.6) ? 0 : 1;
            
            tot_bytes += traces[i].packets[p].size;
        }
        traces[i].duration = current_time;
        traces[i].total_bytes = tot_bytes;
    }
}

void bench_generate_traces(FlowTrace *traces, uint16_t num_traces, uint8_t protocol, uint32_t seed) {
    if (protocol == 0) {
        bench_generate_background(traces, num_traces, seed);
        return;
    }
    
    prng_state = seed + protocol * 1000;
    for (uint16_t i = 0; i < num_traces; i++) {
        traces[i].protocol_label = protocol;
        uint16_t np = 50 + (next_rand() % 500); 
        traces[i].num_packets = np;
        
        double current_time = 0.0;
        uint32_t tot_bytes = 0;
        for (uint16_t p = 0; p < np; p++) {
            if (protocol == 1) { // IPv4
                current_time += 0.05 + rand_normal() * 0.01;
                if (current_time < 0.001) current_time = 0.001;
                uint16_t sz = (p % 2 == 0) ? (40 + next_rand()%100) : (1000 + next_rand()%500);
                traces[i].packets[p].size = sz;
                traces[i].packets[p].direction = (p % 2 == 0) ? 0 : 1;
            } else if (protocol == 2) { // Tor
                current_time += 0.01 + rand_normal() * 0.002;
                if (current_time < 0.001) current_time = 0.001;
                traces[i].packets[p].size = 512;
                traces[i].packets[p].direction = (rand_double() < 0.5) ? 0 : 1;
            } else if (protocol == 3) { // Airbot
                current_time += 0.01 + rand_double() * 0.1; 
                traces[i].packets[p].size = 16 + next_rand() % 496;
                traces[i].packets[p].direction = (rand_double() < 0.5) ? 0 : 1;
            } else if (protocol == 4) { // Airbot+Onion
                current_time += 0.02 + rand_double() * 0.05; 
                traces[i].packets[p].size = 512;
                traces[i].packets[p].direction = (rand_double() < 0.5) ? 0 : 1;
            }
            if (p > 0) traces[i].packets[p].timestamp = traces[i].packets[p-1].timestamp + current_time;
            else traces[i].packets[p].timestamp = current_time;
            
            tot_bytes += traces[i].packets[p].size;
        }
        traces[i].duration = traces[i].packets[np-1].timestamp;
        traces[i].total_bytes = tot_bytes;
    }
}

void bench_extract_features(const FlowTrace *trace, FlowFeatures *features) {
    if (trace->num_packets == 0) return;
    
    features->num_packets = trace->num_packets;
    features->duration = trace->duration;
    features->bytes_per_sec = trace->total_bytes / (trace->duration > 0.001 ? trace->duration : 0.001);
    
    double sum_size = 0, sum_iat = 0;
    uint32_t out_pkts = 0;
    for (uint16_t p = 0; p < trace->num_packets; p++) {
        sum_size += trace->packets[p].size;
        if (trace->packets[p].direction == 0) out_pkts++;
        if (p > 0) sum_iat += (trace->packets[p].timestamp - trace->packets[p-1].timestamp);
    }
    features->mean_pkt_size = sum_size / trace->num_packets;
    features->mean_iat = (trace->num_packets > 1) ? sum_iat / (trace->num_packets - 1) : 0;
    features->direction_ratio = (double)out_pkts / trace->num_packets;
    
    double var_size = 0, var_iat = 0;
    uint32_t burst_pkts = 0;
    
    // For entropy
    int size_bins[16] = {0};
    int iat_bins[16] = {0};
    
    for (uint16_t p = 0; p < trace->num_packets; p++) {
        double diff = trace->packets[p].size - features->mean_pkt_size;
        var_size += diff * diff;
        
        int sbin = trace->packets[p].size / 100;
        if (sbin > 15) sbin = 15;
        size_bins[sbin]++;
        
        if (p > 0) {
            double iat = trace->packets[p].timestamp - trace->packets[p-1].timestamp;
            double diff_iat = iat - features->mean_iat;
            var_iat += diff_iat * diff_iat;
            if (iat < 0.01) burst_pkts++;
            
            int ibin = (int)(iat * 100);
            if (ibin > 15) ibin = 15;
            iat_bins[ibin]++;
        }
    }
    features->std_pkt_size = sqrt(var_size / trace->num_packets);
    features->std_iat = (trace->num_packets > 1) ? sqrt(var_iat / (trace->num_packets - 1)) : 0;
    features->burstiness = (double)burst_pkts / trace->num_packets;
    
    features->entropy_size = 0;
    for (int i = 0; i < 16; i++) {
        if (size_bins[i] > 0) {
            double p = (double)size_bins[i] / trace->num_packets;
            features->entropy_size -= p * (log(p)/log(2.0));
        }
    }
    
    features->entropy_iat = 0;
    int iat_count = trace->num_packets - 1;
    if (iat_count > 0) {
        for (int i = 0; i < 16; i++) {
            if (iat_bins[i] > 0) {
                double p = (double)iat_bins[i] / iat_count;
                features->entropy_iat -= p * (log(p)/log(2.0));
            }
        }
    } else {
        features->entropy_iat = 0;
    }
}

static double feat_dist(const FlowFeatures *f1, const FlowFeatures *f2) {
    double d = 0;
    // Normalize features simply by their rough typical scales
    double d_size = (f1->mean_pkt_size - f2->mean_pkt_size)/1000.0;
    double d_iat = (f1->mean_iat - f2->mean_iat)/0.1;
    double d_bps = (f1->bytes_per_sec - f2->bytes_per_sec)/100000.0;
    double d_std_size = (f1->std_pkt_size - f2->std_pkt_size)/500.0;
    double d_ent_size = (f1->entropy_size - f2->entropy_size)/2.0;
    
    d += d_size*d_size;
    d += d_iat*d_iat;
    d += d_bps*d_bps;
    d += d_std_size*d_std_size;
    d += d_ent_size*d_ent_size;
    
    return sqrt(d);
}

void bench_classify(const FlowFeatures *train, const uint8_t *train_labels, uint16_t num_train,
                    const FlowFeatures *test, ClassResult *results, uint16_t num_test) {
    for (uint16_t i = 0; i < num_test; i++) {
        double min_d = 1e9;
        uint8_t best_label = 0;
        
        // 1-NN
        for (uint16_t j = 0; j < num_train; j++) {
            double d = feat_dist(&test[i], &train[j]);
            if (d < min_d) {
                min_d = d;
                best_label = train_labels[j];
            }
        }
        
        results[i].predicted_label = best_label;
        // Simple confidence based on distance
        results[i].confidence = 1.0 / (1.0 + min_d);
    }
}

void bench_compute_metrics(const ClassResult *results, uint16_t num_results, ClassifierMetrics *metrics) {
    memset(metrics, 0, sizeof(ClassifierMetrics));
    uint32_t tp = 0, fp = 0, tn = 0, fn = 0;
    
    for (uint16_t i = 0; i < num_results; i++) {
        int actual_pos = (results[i].true_label != 0);
        int pred_pos = (results[i].predicted_label != 0);
        
        if (actual_pos && pred_pos) tp++;
        else if (!actual_pos && pred_pos) fp++;
        else if (!actual_pos && !pred_pos) tn++;
        else fn++;
    }
    
    metrics->true_pos = tp;
    metrics->false_pos = fp;
    metrics->true_neg = tn;
    metrics->false_neg = fn;
    
    metrics->precision = (tp + fp) > 0 ? (double)tp / (tp + fp) : 0;
    metrics->recall = (tp + fn) > 0 ? (double)tp / (tp + fn) : 0;
    metrics->f1 = (metrics->precision + metrics->recall) > 0 ? 2 * metrics->precision * metrics->recall / (metrics->precision + metrics->recall) : 0;
    
    // Simplified advantage: TPR - FPR
    double tpr = (tp + fn) > 0 ? (double)tp / (tp + fn) : 0;
    double fpr = (tn + fp) > 0 ? (double)fp / (tn + fp) : 0;
    metrics->advantage = fabs(tpr - fpr);
    
    // Simplistic AUC estimate
    metrics->auc = (tpr - fpr + 1.0) / 2.0;
}

void bench_correlate(const FlowTrace *entry_flows, const FlowTrace *exit_flows,
                     uint16_t num_flows, CorrelationResult *result) {
    result->total_pairs = num_flows;
    result->correct_links = 0;
    
    for (uint16_t i = 0; i < num_flows; i++) {
        FlowFeatures f_entry, f_exit;
        bench_extract_features(&entry_flows[i], &f_entry);
        
        // Find best match in exit flows
        double min_d = 1e9;
        uint16_t best_j = 0;
        
        for (uint16_t j = 0; j < num_flows; j++) {
            bench_extract_features(&exit_flows[j], &f_exit);
            double d = feat_dist(&f_entry, &f_exit);
            if (d < min_d) {
                min_d = d;
                best_j = j;
            }
        }
        
        if (best_j == i) result->correct_links++;
    }
    
    result->linking_accuracy = (double)result->correct_links / num_flows;
    double random_guess = 1.0 / num_flows;
    result->unlinkability = 1.0 - (result->linking_accuracy - random_guess) / (1.0 - random_guess);
    if (result->unlinkability < 0) result->unlinkability = 0;
    
    result->mutual_info = -log(result->unlinkability + 1e-9)/log(2.0); // Rough proxy
}

void bench_run_comparison(BenchmarkComparison *cmp) {
    const int NUM_PROTOCOLS = 5;
    const char* names[] = {"Background", "IPv4", "Tor", "Airbot", "A+Onion"};
    for(int i=0; i<5; i++) cmp->protocol_names[i] = names[i];
    
    const int NUM_TRAIN = 100;
    const int NUM_TEST = 100;
    
    for (int p = 1; p < NUM_PROTOCOLS; p++) {
        // Classifier experiment
        FlowTrace *train_traces = malloc((NUM_TRAIN*2) * sizeof(FlowTrace));
        FlowTrace *test_traces = malloc((NUM_TEST*2) * sizeof(FlowTrace));
        
        bench_generate_traces(train_traces, NUM_TRAIN, p, 100);
        bench_generate_traces(train_traces + NUM_TRAIN, NUM_TRAIN, 0, 200);
        
        bench_generate_traces(test_traces, NUM_TEST, p, 300);
        bench_generate_traces(test_traces + NUM_TEST, NUM_TEST, 0, 400);
        
        FlowFeatures *train_f = malloc((NUM_TRAIN*2) * sizeof(FlowFeatures));
        uint8_t *train_l = malloc((NUM_TRAIN*2) * sizeof(uint8_t));
        
        FlowFeatures *test_f = malloc((NUM_TEST*2) * sizeof(FlowFeatures));
        ClassResult *res = malloc((NUM_TEST*2) * sizeof(ClassResult));
        
        for (int i = 0; i < NUM_TRAIN*2; i++) {
            bench_extract_features(&train_traces[i], &train_f[i]);
            train_l[i] = train_traces[i].protocol_label;
        }
        for (int i = 0; i < NUM_TEST*2; i++) {
            bench_extract_features(&test_traces[i], &test_f[i]);
            res[i].true_label = test_traces[i].protocol_label;
        }
        
        bench_classify(train_f, train_l, NUM_TRAIN*2, test_f, res, NUM_TEST*2);
        bench_compute_metrics(res, NUM_TEST*2, &cmp->detectability[p]);
        
        free(train_traces);
        free(test_traces);
        free(train_f);
        free(train_l);
        free(test_f);
        free(res);
        
        /* ── Entry-Exit Correlation Experiment ──
         * Generate entry flows, then simulate protocol-specific
         * transforms to create exit flows. Each protocol adds
         * different amounts of noise, delay, and modification.
         */
        FlowTrace *entry_traces = (FlowTrace *)malloc(50 * sizeof(FlowTrace));
        FlowTrace *exit_traces  = (FlowTrace *)malloc(50 * sizeof(FlowTrace));
        
        bench_generate_traces(entry_traces, 50, p, 500 + p * 100);
        memcpy(exit_traces, entry_traces, 50 * sizeof(FlowTrace));
        
        for(int i = 0; i < 50; i++) {
            for(int k = 0; k < exit_traces[i].num_packets; k++) {
                if (p == 1) {
                    /* IPv4: nearly identical exit — trivially correlatable */
                    exit_traces[i].packets[k].timestamp += 0.001 + rand_double() * 0.002;
                } else if (p == 2) {
                    /* Tor: 3-hop relay delay + cell padding + some timing noise
                     * Tor uses fixed 512-byte cells, adds ~100ms per hop */
                    exit_traces[i].packets[k].timestamp += 0.3 + rand_normal() * 0.08;
                    exit_traces[i].packets[k].size = 512; /* Fixed cell size */
                    /* Add padding cells (extra packets) */
                    if (rand_double() < 0.15 && exit_traces[i].num_packets < 1020) {
                        int np = exit_traces[i].num_packets;
                        exit_traces[i].packets[np].timestamp = exit_traces[i].packets[k].timestamp + 0.001;
                        exit_traces[i].packets[np].size = 512;
                        exit_traces[i].packets[np].direction = (uint8_t)(rand_double() < 0.5 ? 0 : 1);
                        exit_traces[i].num_packets++;
                    }
                } else if (p == 3) {
                    /* Airbot (no onion): EIU processing delay + size variation */
                    exit_traces[i].packets[k].timestamp += 0.01 + rand_normal() * 0.02;
                    /* EIU adds overhead but keeps variable sizes */
                    exit_traces[i].packets[k].size += (uint16_t)(16 + (next_rand() % 32));
                    if (exit_traces[i].packets[k].size > 1500)
                        exit_traces[i].packets[k].size = 1500;
                } else if (p == 4) {
                    /* Airbot+Onion: 3-hop delay + cell quantization + capability check delay
                     * Onion adds significant timing noise and normalizes sizes */
                    exit_traces[i].packets[k].timestamp += 0.25 + rand_normal() * 0.12;
                    /* Quantize packet sizes to 128-byte boundaries (unlike Tor's fixed 512) */
                    uint16_t sz = exit_traces[i].packets[k].size;
                    sz = (uint16_t)(((sz + 127) / 128) * 128);
                    if (sz > 1536) sz = 1536;
                    exit_traces[i].packets[k].size = sz;
                    /* Capability verification adds jitter */
                    if (rand_double() < 0.1) {
                        exit_traces[i].packets[k].timestamp += rand_double() * 0.05;
                    }
                    /* Add decoy packets for traffic shaping */
                    if (rand_double() < 0.2 && exit_traces[i].num_packets < 1020) {
                        int np = exit_traces[i].num_packets;
                        exit_traces[i].packets[np].timestamp = exit_traces[i].packets[k].timestamp + rand_double() * 0.01;
                        exit_traces[i].packets[np].size = 128 + (uint16_t)((next_rand() % 4) * 128);
                        exit_traces[i].packets[np].direction = (uint8_t)(rand_double() < 0.5 ? 0 : 1);
                        exit_traces[i].num_packets++;
                    }
                }
            }
        }
        
        bench_correlate(entry_traces, exit_traces, 50, &cmp->correlation[p]);
        
        free(entry_traces);
        free(exit_traces);
        
        /* Performance metrics (estimated from protocol properties) */
        if (p == 1) { cmp->latency_ms[p] = 10;  cmp->bandwidth_overhead[p] = 1.05; cmp->cpu_cycles[p] = 1.0; }
        if (p == 2) { cmp->latency_ms[p] = 300; cmp->bandwidth_overhead[p] = 3.00; cmp->cpu_cycles[p] = 5.0; }
        if (p == 3) { cmp->latency_ms[p] = 50;  cmp->bandwidth_overhead[p] = 1.20; cmp->cpu_cycles[p] = 2.0; }
        if (p == 4) { cmp->latency_ms[p] = 200; cmp->bandwidth_overhead[p] = 2.50; cmp->cpu_cycles[p] = 4.0; }
        
        /* Empirical privacy vector derived from experiments */
        cmp->T_empirical[p] = 1.0 - cmp->detectability[p].auc;
        cmp->U_empirical[p] = cmp->correlation[p].unlinkability;
        cmp->epsilon_empirical[p] = cmp->detectability[p].advantage * 2.0;
    }
}

void bench_print_scorecard(const BenchmarkComparison *cmp) {
    printf("  ┌─── EMPIRICAL PRIVACY SCORECARD ─────────────────────\n");
    printf("  │\n");
    printf("  │  Metric              │ IPv4  │  Tor  │ Airbot │ A+Onion │ Winner\n");
    printf("  │  ────────────────────┼───────┼───────┼────────┼─────────┼───────\n");
    
    // Determine winners
    int w_auc=1, w_adv=1, w_link=1, w_u=1, w_eps=1, w_lat=1, w_bw=1;
    for(int i=2; i<=4; i++) {
        if(cmp->detectability[i].auc < cmp->detectability[w_auc].auc) w_auc = i;
        if(cmp->detectability[i].advantage < cmp->detectability[w_adv].advantage) w_adv = i;
        if(cmp->correlation[i].linking_accuracy < cmp->correlation[w_link].linking_accuracy) w_link = i;
        if(cmp->U_empirical[i] > cmp->U_empirical[w_u]) w_u = i;
        if(cmp->epsilon_empirical[i] < cmp->epsilon_empirical[w_eps]) w_eps = i;
        if(cmp->latency_ms[i] < cmp->latency_ms[w_lat]) w_lat = i;
        if(cmp->bandwidth_overhead[i] < cmp->bandwidth_overhead[w_bw]) w_bw = i;
    }
    
    printf("  │  Classifier AUC      │ %4.2f  │ %4.2f  │ %4.2f   │ %4.2f    │ %s\n", 
           cmp->detectability[1].auc, cmp->detectability[2].auc, cmp->detectability[3].auc, cmp->detectability[4].auc, cmp->protocol_names[w_auc]);
    printf("  │  Advantage Δ         │ %4.2f  │ %4.2f  │ %4.2f   │ %4.2f    │ %s\n", 
           cmp->detectability[1].advantage, cmp->detectability[2].advantage, cmp->detectability[3].advantage, cmp->detectability[4].advantage, cmp->protocol_names[w_adv]);
    printf("  │  Linking accuracy    │ %4.2f  │ %4.2f  │ %4.2f   │ %4.2f    │ %s\n", 
           cmp->correlation[1].linking_accuracy, cmp->correlation[2].linking_accuracy, cmp->correlation[3].linking_accuracy, cmp->correlation[4].linking_accuracy, cmp->protocol_names[w_link]);
    printf("  │  Unlinkability U     │ %4.2f  │ %4.2f  │ %4.2f   │ %4.2f    │ %s\n", 
           cmp->U_empirical[1], cmp->U_empirical[2], cmp->U_empirical[3], cmp->U_empirical[4], cmp->protocol_names[w_u]);
    printf("  │  Privacy loss ε      │ %4.2f  │ %4.2f  │ %4.2f   │ %4.2f    │ %s\n", 
           cmp->epsilon_empirical[1], cmp->epsilon_empirical[2], cmp->epsilon_empirical[3], cmp->epsilon_empirical[4], cmp->protocol_names[w_eps]);
    printf("  │  Latency (relative)  │ %4.0f  │ %4.0f  │ %4.0f   │ %4.0f    │ %s\n", 
           cmp->latency_ms[1], cmp->latency_ms[2], cmp->latency_ms[3], cmp->latency_ms[4], cmp->protocol_names[w_lat]);
    printf("  │  Bandwidth overhead  │ %4.2fx │ %4.2fx │ %4.2fx  │ %4.2fx   │ %s\n", 
           cmp->bandwidth_overhead[1], cmp->bandwidth_overhead[2], cmp->bandwidth_overhead[3], cmp->bandwidth_overhead[4], cmp->protocol_names[w_bw]);
    printf("  │\n");
    printf("  │  Unique Capabilities │ IPv4  │  Tor  │ Airbot │ A+Onion\n");
    printf("  │  ────────────────────┼───────┼───────┼────────┼─────────\n");
    printf("  │  Executable routing  │  No   │  No   │  Yes   │  Yes\n");
    printf("  │  State evolution     │  No   │  No   │  Yes   │  Yes\n");
    printf("  │  Capability auth     │  No   │  No   │  Yes   │  Yes\n");
    printf("  │  Replication         │  No   │  No   │  Yes   │  Yes\n");
    printf("  │  Onion routing       │  No   │  Yes  │  No    │  Yes\n");
    printf("  └─────────────────────────────────────────────────────\n");
}

void bench_print_classifier(const ClassifierMetrics *m, const char *name) {
    printf("Classifier Results for %s:\n", name);
    printf("  Precision: %.2f, Recall: %.2f, F1: %.2f\n", m->precision, m->recall, m->f1);
    printf("  AUC: %.2f, Advantage: %.2f\n", m->auc, m->advantage);
}

void bench_print_correlation(const CorrelationResult *r, const char *name) {
    printf("Correlation Results for %s:\n", name);
    printf("  Linking Accuracy: %.2f (%d/%d)\n", r->linking_accuracy, r->correct_links, r->total_pairs);
    printf("  Unlinkability: %.2f\n", r->unlinkability);
}
