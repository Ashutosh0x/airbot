#ifndef AIRBOT_BENCHMARK_H
#define AIRBOT_BENCHMARK_H

#include <stdint.h>
#include "bitstream.h"

// A single packet observation
typedef struct {
    double   timestamp;         // Seconds since flow start
    uint16_t size;              // Packet size in bytes
    uint8_t  direction;         // 0=outgoing, 1=incoming
} PacketObs;

// A flow observation (sequence of packets)
typedef struct {
    PacketObs packets[1024];    // Up to 1024 packets per flow
    uint16_t  num_packets;
    double    duration;         // Total flow duration
    uint32_t  total_bytes;      // Total bytes transferred
    uint8_t   protocol_label;   // 0=background, 1=IPv4, 2=Tor, 3=Airbot, 4=Airbot+Onion
} FlowTrace;

// Extracted features from a flow (for classification)
typedef struct {
    double mean_pkt_size;       // Mean packet size
    double std_pkt_size;        // Std dev of packet sizes
    double mean_iat;            // Mean inter-arrival time
    double std_iat;             // Std dev of inter-arrival time
    double burstiness;          // Burst ratio (packets in bursts / total)
    double duration;            // Flow duration
    double bytes_per_sec;       // Throughput
    uint16_t num_packets;       // Packet count
    double direction_ratio;     // Fraction of outgoing packets
    double entropy_size;        // Shannon entropy of packet size distribution
    double entropy_iat;         // Shannon entropy of IAT distribution
} FlowFeatures;

// Classification result for one flow
typedef struct {
    uint8_t  true_label;        // Ground truth
    uint8_t  predicted_label;   // Classifier prediction
    double   confidence;        // Classifier confidence [0,1]
} ClassResult;

// Aggregate classifier performance
typedef struct {
    uint32_t true_pos;
    uint32_t false_pos;
    uint32_t true_neg;
    uint32_t false_neg;
    double   precision;
    double   recall;
    double   f1;
    double   auc;               // Area under ROC curve (simplified)
    double   advantage;         // |P(A(V_0)=1) - P(A(V_1)=1)|
} ClassifierMetrics;

// Entry-Exit correlation result
typedef struct {
    uint32_t total_pairs;
    uint32_t correct_links;     // Correctly linked entry-exit pairs
    double   linking_accuracy;  // correct_links / total_pairs
    double   unlinkability;     // 1 - normalized linking advantage
    double   mutual_info;       // Estimated MI between entry/exit features
} CorrelationResult;

// Full benchmark comparison
typedef struct {
    // Per-protocol metrics
    ClassifierMetrics  detectability[5];  // 0=bg, 1=IPv4, 2=Tor, 3=Airbot, 4=Airbot+Onion
    CorrelationResult  correlation[5];    // Entry-exit correlation per protocol
    
    // Performance metrics
    double latency_ms[5];       // Mean latency
    double bandwidth_overhead[5]; // Overhead ratio
    double cpu_cycles[5];       // Relative CPU cost
    
    // Privacy vector (empirically derived)
    double H_empirical[5];      // From anonymity set experiment
    double U_empirical[5];      // From correlation experiment
    double T_empirical[5];      // From classifier experiment (1 - AUC)
    double C_empirical[5];      // Content confidentiality
    double epsilon_empirical[5]; // Privacy loss bound
    
    const char *protocol_names[5];
} BenchmarkComparison;

// Generate simulated traffic traces for each protocol
void bench_generate_traces(FlowTrace *traces, uint16_t num_traces, uint8_t protocol, uint32_t seed);

// Generate background traffic (web browsing simulation)
void bench_generate_background(FlowTrace *traces, uint16_t num_traces, uint32_t seed);

// Extract features from a flow trace
void bench_extract_features(const FlowTrace *trace, FlowFeatures *features);

// Simple nearest-neighbor classifier
// Trains on training set, predicts on test set
void bench_classify(const FlowFeatures *train, const uint8_t *train_labels, uint16_t num_train,
                    const FlowFeatures *test, ClassResult *results, uint16_t num_test);

// Compute classifier metrics from results
void bench_compute_metrics(const ClassResult *results, uint16_t num_results, ClassifierMetrics *metrics);

// Entry-exit correlation experiment
void bench_correlate(const FlowTrace *entry_flows, const FlowTrace *exit_flows,
                     uint16_t num_flows, CorrelationResult *result);

// Run full benchmark comparison
void bench_run_comparison(BenchmarkComparison *cmp);

// Print the final scorecard
void bench_print_scorecard(const BenchmarkComparison *cmp);

// Print detailed classifier results
void bench_print_classifier(const ClassifierMetrics *m, const char *name);

// Print correlation results
void bench_print_correlation(const CorrelationResult *r, const char *name);

#endif // AIRBOT_BENCHMARK_H
