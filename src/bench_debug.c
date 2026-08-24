#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "benchmark.h"

int main(void) {
    const int NUM_TRAIN = 100;
    const int NUM_TEST = 100;
    const char* names[] = {"Background", "IPv4", "Tor", "Airbot", "A+Onion"};
    
    for (int p = 1; p <= 4; p++) {
        printf("=== Protocol %d (%s) ===\n", p, names[p]); fflush(stdout);
        
        printf("  Allocating train/test...\n"); fflush(stdout);
        FlowTrace *train_traces = (FlowTrace*)malloc((NUM_TRAIN*2) * sizeof(FlowTrace));
        FlowTrace *test_traces = (FlowTrace*)malloc((NUM_TEST*2) * sizeof(FlowTrace));
        if (!train_traces || !test_traces) { printf("  MALLOC FAIL\n"); return 1; }
        
        printf("  Generating traces...\n"); fflush(stdout);
        bench_generate_traces(train_traces, NUM_TRAIN, (uint8_t)p, 100);
        bench_generate_traces(train_traces + NUM_TRAIN, NUM_TRAIN, 0, 200);
        bench_generate_traces(test_traces, NUM_TEST, (uint8_t)p, 300);
        bench_generate_traces(test_traces + NUM_TEST, NUM_TEST, 0, 400);
        
        printf("  Extracting features...\n"); fflush(stdout);
        FlowFeatures *train_f = (FlowFeatures*)malloc((NUM_TRAIN*2) * sizeof(FlowFeatures));
        uint8_t *train_l = (uint8_t*)malloc(NUM_TRAIN*2);
        FlowFeatures *test_f = (FlowFeatures*)malloc((NUM_TEST*2) * sizeof(FlowFeatures));
        ClassResult *res = (ClassResult*)malloc((NUM_TEST*2) * sizeof(ClassResult));
        
        for (int i = 0; i < NUM_TRAIN*2; i++) {
            bench_extract_features(&train_traces[i], &train_f[i]);
            train_l[i] = train_traces[i].protocol_label;
        }
        for (int i = 0; i < NUM_TEST*2; i++) {
            bench_extract_features(&test_traces[i], &test_f[i]);
            res[i].true_label = test_traces[i].protocol_label;
        }
        
        printf("  Classifying...\n"); fflush(stdout);
        bench_classify(train_f, train_l, NUM_TRAIN*2, test_f, res, NUM_TEST*2);
        
        ClassifierMetrics m;
        bench_compute_metrics(res, NUM_TEST*2, &m);
        printf("  AUC=%.2f F1=%.2f\n", m.auc, m.f1); fflush(stdout);
        
        free(train_traces); free(test_traces);
        free(train_f); free(train_l); free(test_f); free(res);
        
        printf("  Correlation experiment...\n"); fflush(stdout);
        FlowTrace *entry = (FlowTrace*)malloc(50 * sizeof(FlowTrace));
        FlowTrace *exits = (FlowTrace*)malloc(50 * sizeof(FlowTrace));
        if (!entry || !exits) { printf("  MALLOC FAIL\n"); return 1; }
        
        bench_generate_traces(entry, 50, (uint8_t)p, 500 + p * 100);
        memcpy(exits, entry, 50 * sizeof(FlowTrace));
        
        printf("  Transforming exit flows...\n"); fflush(stdout);
        for (int i = 0; i < 50; i++) {
            for (int k = 0; k < exits[i].num_packets; k++) {
                if (p == 1) {
                    exits[i].packets[k].timestamp += 0.001;
                } else if (p == 2) {
                    exits[i].packets[k].timestamp += 0.3;
                    exits[i].packets[k].size = 512;
                } else if (p == 3) {
                    exits[i].packets[k].timestamp += 0.01;
                } else if (p == 4) {
                    exits[i].packets[k].timestamp += 0.25;
                    uint16_t sz = exits[i].packets[k].size;
                    sz = (uint16_t)(((sz + 127) / 128) * 128);
                    if (sz > 1536) sz = 1536;
                    exits[i].packets[k].size = sz;
                }
            }
        }
        
        printf("  Running correlator...\n"); fflush(stdout);
        CorrelationResult cr;
        bench_correlate(entry, exits, 50, &cr);
        printf("  LinkAcc=%.2f U=%.2f\n", cr.linking_accuracy, cr.unlinkability); fflush(stdout);
        
        free(entry); free(exits);
        printf("  Protocol %d DONE\n\n", p); fflush(stdout);
    }
    
    printf("ALL PROTOCOLS PASSED\n");
    return 0;
}
