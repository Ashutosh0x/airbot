#include "onion.h"
#include "chacha20.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

/* ChaCha20-Poly1305 AEAD, csprng_bytes, replay filter
 * are all declared in chacha20.h (included via onion.h) */

/*
 * ChaCha20-Poly1305 AEAD per hop
 */

void replay_filter_init(ReplayFilter *rf) {
    if (!rf) return;
    memset(rf, 0, sizeof(ReplayFilter));
}

int replay_filter_check(ReplayFilter *rf, const uint8_t nonce[12]) {
    if (!rf) return 0;
    
    // FNV-1a hash for bucket selection
    uint32_t hash = 2166136261u;
    for (int i = 0; i < 12; i++) {
        hash ^= nonce[i];
        hash *= 16777619u;
    }
    
    uint32_t index = hash % REPLAY_FILTER_SIZE;
    
    // Linear probing if we wanted to handle collisions, but a simple 
    // single-slot or checking occupied is enough for this prototype.
    if (rf->occupied[index]) {
        if (memcmp(rf->entries[index], nonce, 12) == 0) {
            return 1; // Replay detected
        }
        // Collision (overwrite or ignore depending on policy, we just overwrite here for prototype)
    }
    
    // Store new nonce
    memcpy(rf->entries[index], nonce, 12);
    rf->occupied[index] = 1;
    rf->count++;
    
    return 0; // Fresh
}


void onion_circuit_init(OnionCircuit *circuit, uint8_t num_relays) {
    if (!circuit) return;
    memset(circuit, 0, sizeof(OnionCircuit));
    circuit->num_relays = num_relays > 8 ? 8 : num_relays;
    
    csprng_bytes((uint8_t*)&circuit->circuit_id, sizeof(circuit->circuit_id));
    csprng_bytes(circuit->nonce, 12);
    
    replay_filter_init(&circuit->replay_filter);
}

void onion_generate_relays(OnionCircuit *circuit, const uint8_t *seed, uint8_t seed_len) {
    Blake3State hasher;
    for (uint8_t i = 0; i < circuit->num_relays; i++) {
        uint8_t hash[32];
        blake3_init(&hasher);
        blake3_update(&hasher, seed, seed_len);
        blake3_update(&hasher, &i, sizeof(i));
        blake3_finalize(&hasher, hash);
        
        circuit->relays[i].node_id[0] = 0x0A;
        circuit->relays[i].node_id[1] = 0x0B;
        circuit->relays[i].node_id[2] = 0x0C;
        circuit->relays[i].node_id[3] = i;
        
        memcpy(circuit->relays[i].secret_key, hash, 32);
        
        // Capability token derived differently
        blake3_init(&hasher);
        blake3_update(&hasher, (const uint8_t *)"cap", 3);
        blake3_update(&hasher, hash, 32);
        blake3_finalize(&hasher, circuit->relays[i].capability);
    }
}

int onion_wrap(const OnionCircuit *circuit, const uint8_t *plaintext, uint16_t plain_len, OnionPacket *packet) {
    if (!circuit || !plaintext || !packet) return -1;
    
    packet->circuit_id = circuit->circuit_id;
    memcpy(packet->nonce, circuit->nonce, 12);
    packet->num_layers = circuit->num_relays;
    
    // Start with the innermost payload (the EIU)
    uint8_t temp_buffer[4096];
    uint16_t temp_len = plain_len;
    if (plain_len > sizeof(temp_buffer)) return -1;
    memcpy(temp_buffer, plaintext, plain_len);
    
    // Wrap from innermost to outermost
    for (int i = circuit->num_relays - 1; i >= 0; i--) {
        // Construct the layer payload: next_hop(4) | capability(32) | inner
        uint8_t layer_buffer[4096];
        uint16_t layer_len = 4 + 32 + temp_len;
        if (layer_len > 4096) return -1;
        
        if (i == circuit->num_relays - 1) {
            // Destination (no next hop, all zeros)
            memset(layer_buffer, 0, 4);
        } else {
            memcpy(layer_buffer, circuit->relays[i+1].node_id, 4);
        }
        memcpy(layer_buffer + 4, circuit->relays[i].capability, 32);
        memcpy(layer_buffer + 36, temp_buffer, temp_len);
        
        // Derive per-hop nonce
        uint8_t hop_nonce[12];
        Blake3State hasher;
        blake3_init(&hasher);
        blake3_update(&hasher, circuit->nonce, 12);
        uint8_t hop_idx = (uint8_t)i;
        blake3_update(&hasher, &hop_idx, sizeof(hop_idx));
        uint8_t hash_out[32];
        blake3_finalize(&hasher, hash_out);
        memcpy(hop_nonce, hash_out, 12);
        
        // Encrypt the layer
        chacha20_poly1305_encrypt(circuit->relays[i].secret_key, hop_nonce,
            circuit->relays[i].capability, 32,
            layer_buffer, layer_len,
            temp_buffer, packet->tags[i]);
        
        temp_len = layer_len;
    }
    
    memcpy(packet->payload, temp_buffer, temp_len);
    packet->payload_len = temp_len;
    packet->current_hop = 0; /* Unwrapping starts at outermost layer */
    
    return 0; /* OK */
}

int onion_unwrap(const OnionRelay *relay, const OnionPacket *packet, OnionUnwrapResult *result) {
    if (!relay || !packet || !result) return -1;
    
    if (packet->payload_len < 36) return -1; /* Must have at least next_hop + cap */
    if (packet->current_hop >= packet->num_layers) return -1;
    
    /* Use current_hop as the layer index — matches the relay index
     * used during wrapping (0 = outermost, num_relays-1 = innermost) */
    uint8_t hop_idx = packet->current_hop;
    
    /* Derive per-hop nonce: BLAKE3(circuit_nonce || hop_index)[0:12] */
    uint8_t hop_nonce[12];
    Blake3State hasher;
    blake3_init(&hasher);
    blake3_update(&hasher, packet->nonce, 12);
    blake3_update(&hasher, &hop_idx, sizeof(hop_idx));
    uint8_t hash_out[32];
    blake3_finalize(&hasher, hash_out);
    memcpy(hop_nonce, hash_out, 12);
    
    uint8_t layer_buffer[4096];
    
    /* Decrypt and verify the outer layer using AEAD */
    if (chacha20_poly1305_decrypt(relay->secret_key, hop_nonce,
        relay->capability, 32,
        packet->payload, packet->payload_len,
        packet->tags[hop_idx], layer_buffer) != 0) {
        result->valid = 0;
        return -1; /* Authentication failure — tampered or wrong key */
    }
    
    /* Extract info from decrypted layer */
    memcpy(result->next_hop, layer_buffer, 4);
    memcpy(result->capability, layer_buffer + 4, 32);
    
    result->inner_len = packet->payload_len - 36;
    memcpy(result->inner_payload, layer_buffer + 36, result->inner_len);
    
    /* Check if next_hop is all zeros (destination) */
    result->is_final = 1;
    for (int i = 0; i < 4; i++) {
        if (result->next_hop[i] != 0) {
            result->is_final = 0;
            break;
        }
    }
    
    /* Capability validation (already authenticated by AEAD, this is a semantic check) */
    result->valid = (memcmp(result->capability, relay->capability, 32) == 0);
    
    return 0;
}

int onion_verify_capability(const OnionRelay *relay, const uint8_t *capability) {
    return memcmp(relay->capability, capability, 32) == 0;
}

int onion_check_replay(ReplayFilter *rf, const uint8_t *nonce) {
    return replay_filter_check(rf, nonce);
}

void onion_compute_metrics(const OnionCircuit *circuit, OnionPrivacyMetrics *metrics) {
    if (!circuit || !metrics) return;
    
    metrics->num_hops = circuit->num_relays;
    metrics->entry_exit_separation = 1.0 / (double)(circuit->num_relays > 0 ? circuit->num_relays : 1); 
    metrics->relay_knowledge = 1.0 / (double)(circuit->num_relays > 0 ? circuit->num_relays : 1); 
    metrics->replay_protected = 1.0; 
    
    /* TODO: Replace with measured values from benchmark experiment */
    double N = 1000.0; // Theoretical users
    metrics->estimated_H = log(N) / log(2.0); // No log2 in pure C99/TCC without -lm issues potentially
    metrics->estimated_U = 1.0 - (1.0 / (double)(circuit->num_relays > 0 ? circuit->num_relays : 1)); 
}

void onion_print_circuit(const OnionCircuit *circuit) {
    printf("--- Onion Circuit ID: %08X ---\n", circuit->circuit_id);
    printf("Hops: %d\n", circuit->num_relays);
    for (int i = 0; i < circuit->num_relays; i++) {
        printf(" Relay %d ID: %02X%02X%02X%02X\n", i+1,
               circuit->relays[i].node_id[0], circuit->relays[i].node_id[1],
               circuit->relays[i].node_id[2], circuit->relays[i].node_id[3]);
    }
    printf("----------------------------------\n");
}

void onion_print_metrics(const OnionPrivacyMetrics *metrics) {
    printf("--- Onion Privacy Metrics ---\n");
    printf("Num Hops: %d\n", metrics->num_hops);
    printf("Entry-Exit Separation: %.2f\n", metrics->entry_exit_separation);
    printf("Relay Knowledge: %.2f\n", metrics->relay_knowledge);
    printf("Replay Protected: %.1f\n", metrics->replay_protected);
    printf("Estimated H (entropy): %.2f bits\n", metrics->estimated_H);
    printf("Estimated U (unlinkability): %.2f\n", metrics->estimated_U);
    printf("-----------------------------\n");
}

int onion_simulate(const OnionCircuit *circuit, const uint8_t *eiu_data, uint16_t eiu_len) {
    printf("\n=== Starting Onion Routing Simulation ===\n");
    OnionPacket packet;
    if (onion_wrap(circuit, eiu_data, eiu_len, &packet) != 0) {
        printf("Failed to wrap packet!\n");
        return -1;
    }
    printf("Packet wrapped successfully. Outer payload length: %d bytes\n", packet.payload_len);
    
    OnionPacket current_packet = packet;
    
    for (int i = 0; i < circuit->num_relays; i++) {
        printf("\n--- Hop %d ---\n", i+1);
        printf("Relay received packet with %d layers remaining.\n", current_packet.num_layers);
        
        OnionUnwrapResult result;
        if (onion_unwrap(&circuit->relays[i], &current_packet, &result) != 0) {
            printf("Failed to unwrap at relay %d!\n", i+1);
            return -1;
        }
        
        if (!result.valid) {
            printf("Capability check FAILED at relay %d!\n", i+1);
            return -1;
        }
        printf("Capability check PASSED.\n");
        
        if (result.is_final) {
            printf("Relay %d is the DESTINATION.\n", i+1);
            printf("Unwrapped inner payload length: %d bytes\n", result.inner_len);
            if (result.inner_len == eiu_len && memcmp(result.inner_payload, eiu_data, eiu_len) == 0) {
                printf("Integrity check PASSED. Inner payload matches original EIU data!\n");
            } else {
                printf("Integrity check FAILED. Payload mismatch.\n");
                return -1;
            }
        } else {
            printf("Relay %d unwrapped layer. Next hop is: %02X%02X%02X%02X\n", i+1,
                   result.next_hop[0], result.next_hop[1], result.next_hop[2], result.next_hop[3]);
            
            /* Prepare packet for next hop */
            current_packet.current_hop++;
            current_packet.payload_len = result.inner_len;
            memcpy(current_packet.payload, result.inner_payload, result.inner_len);
        }
    }
    
    printf("=== Onion Routing Simulation Complete ===\n");
    return 0;
}
