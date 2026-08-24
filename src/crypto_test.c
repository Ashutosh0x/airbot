/*
 * Airbot — Executable Information System
 * crypto_test.c — Negative Test Suite for ChaCha20-Poly1305 AEAD
 *
 * Verifies correctness against RFC test vectors and validates
 * that authentication rejects all forms of tampering.
 *
 * Pure C99, no external dependencies.
 */

#include "crypto_test.h"
#include "chacha20.h"
#include "onion.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

/* ═══════════════════════════════════════════════════════════════
 * TEST 1: ChaCha20 RFC 7539 §2.4.2 Test Vector
 * ═══════════════════════════════════════════════════════════════ */
int test_chacha20_rfc_vector(void) {
    uint8_t key[32] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
    };
    uint8_t nonce[12] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4a,
        0x00, 0x00, 0x00, 0x00
    };

    const char *plaintext_str =
        "Ladies and Gentlemen of the class of '99: "
        "If I could offer you only one tip for the future, "
        "sunscreen would be it.";
    size_t len = strlen(plaintext_str);

    /* Expected ciphertext from RFC 7539 §2.4.2 */
    uint8_t expected_ct[] = {
        0x6e, 0x2e, 0x35, 0x9a, 0x25, 0x68, 0xf9, 0x80,
        0x41, 0xba, 0x07, 0x28, 0xdd, 0x0d, 0x69, 0x81,
        0xe9, 0x7e, 0x7a, 0xec, 0x1d, 0x43, 0x60, 0xc2,
        0x0a, 0x27, 0xaf, 0xcc, 0xfd, 0x9f, 0xae, 0x0b,
        0xf9, 0x1b, 0x65, 0xc5, 0x52, 0x47, 0x33, 0xab,
        0x8f, 0x59, 0x3d, 0xab, 0xcd, 0x62, 0xb3, 0x57,
        0x16, 0x39, 0xd6, 0x24, 0xe6, 0x51, 0x52, 0xab,
        0x8f, 0x53, 0x0c, 0x35, 0x9f, 0x08, 0x61, 0xd8,
        0x07, 0xca, 0x0d, 0xbf, 0x50, 0x0d, 0x6a, 0x61,
        0x56, 0xa3, 0x8e, 0x08, 0x8a, 0x22, 0xb6, 0x5e,
        0x52, 0xbc, 0x51, 0x4d, 0x16, 0xcc, 0xf8, 0x06,
        0x81, 0x8c, 0xe9, 0x1a, 0xb7, 0x79, 0x37, 0x36,
        0x5a, 0xf9, 0x0b, 0xbf, 0x74, 0xa3, 0x5b, 0xe6,
        0xb4, 0x0b, 0x8e, 0xed, 0xf2, 0x78, 0x5e, 0x42,
        0x87, 0x4d
    };

    uint8_t out[256];
    memcpy(out, plaintext_str, len);

    /* Use chacha20_init + chacha20_crypt with counter=1 per RFC 7539 §2.4.2 */
    ChaCha20State ctx;
    chacha20_init(&ctx, key, nonce, 1);
    chacha20_crypt(&ctx, out, len);

    if (memcmp(out, expected_ct, len) != 0) {
        return -1;
    }

    return 0;
}

/* ═══════════════════════════════════════════════════════════════
 * TEST 2: Poly1305 RFC 8439 §2.5.2 Test Vector
 * ═══════════════════════════════════════════════════════════════ */
int test_poly1305_rfc_vector(void) {
    uint8_t key[32] = {
        0x85, 0xd6, 0xbe, 0x78, 0x57, 0x55, 0x6d, 0x33,
        0x7f, 0x44, 0x52, 0xfe, 0x42, 0xd5, 0x06, 0xa8,
        0x01, 0x03, 0x80, 0x8a, 0xfb, 0x0d, 0xb2, 0xfd,
        0x4a, 0xbf, 0xf6, 0xaf, 0x41, 0x49, 0xf5, 0x1b
    };

    const char *msg = "Cryptographic Forum Research Group";
    size_t msg_len = strlen(msg);

    uint8_t expected_tag[16] = {
        0xa8, 0x06, 0x1d, 0xc1, 0x30, 0x51, 0x36, 0xc6,
        0xc2, 0x2b, 0x8b, 0xaf, 0x0c, 0x01, 0x27, 0xa9
    };

    uint8_t tag[16] = {0};
    poly1305_auth(tag, (const uint8_t *)msg, msg_len, key);

    if (memcmp(tag, expected_tag, 16) != 0) {
        return -1;
    }

    return 0;
}

/* ═══════════════════════════════════════════════════════════════
 * TEST 3: AEAD Round-trip (encrypt then decrypt)
 * ═══════════════════════════════════════════════════════════════ */
int test_aead_encrypt_decrypt(void) {
    uint8_t key[32];
    uint8_t nonce[12];
    memset(key, 0x11, 32);
    memset(nonce, 0x22, 12);

    uint8_t aad[] = {0xAA, 0xBB, 0xCC};
    const char *pt = "Test message for roundtrip AEAD";
    size_t pt_len = strlen(pt);

    uint8_t ct[128] = {0};
    uint8_t tag[16] = {0};
    uint8_t out_pt[128] = {0};

    if (chacha20_poly1305_encrypt(key, nonce, aad, sizeof(aad),
                                  (const uint8_t*)pt, pt_len,
                                  ct, tag) != 0) return -1;

    if (chacha20_poly1305_decrypt(key, nonce, aad, sizeof(aad),
                                  ct, pt_len, tag, out_pt) != 0) return -1;

    if (memcmp(pt, out_pt, pt_len) != 0) return -1;

    return 0;
}

/* ═══════════════════════════════════════════════════════════════
 * TEST 4: Modified ciphertext → authentication failure
 * ═══════════════════════════════════════════════════════════════ */
int test_modified_ciphertext(void) {
    uint8_t key[32], nonce[12];
    memset(key, 0x11, 32);
    memset(nonce, 0x22, 12);

    uint8_t aad[] = {0xAA};
    const char *pt = "Test message";
    size_t pt_len = strlen(pt);

    uint8_t ct[128] = {0};
    uint8_t tag[16] = {0};
    uint8_t out_pt[128] = {0};

    if (chacha20_poly1305_encrypt(key, nonce, aad, sizeof(aad),
                                  (const uint8_t*)pt, pt_len,
                                  ct, tag) != 0) return -1;

    ct[0] ^= 0x01; /* Flip 1 bit */

    /* Decryption MUST fail */
    if (chacha20_poly1305_decrypt(key, nonce, aad, sizeof(aad),
                                  ct, pt_len, tag, out_pt) == 0) {
        return -1;
    }

    return 0;
}

/* ═══════════════════════════════════════════════════════════════
 * TEST 5: Wrong key → authentication failure
 * ═══════════════════════════════════════════════════════════════ */
int test_wrong_key(void) {
    uint8_t key[32], wrong_key[32], nonce[12];
    memset(key, 0x11, 32);
    memset(wrong_key, 0x99, 32);
    memset(nonce, 0x22, 12);

    uint8_t aad[] = {0xAA};
    const char *pt = "Test message";
    size_t pt_len = strlen(pt);

    uint8_t ct[128] = {0};
    uint8_t tag[16] = {0};
    uint8_t out_pt[128] = {0};

    if (chacha20_poly1305_encrypt(key, nonce, aad, sizeof(aad),
                                  (const uint8_t*)pt, pt_len,
                                  ct, tag) != 0) return -1;

    if (chacha20_poly1305_decrypt(wrong_key, nonce, aad, sizeof(aad),
                                  ct, pt_len, tag, out_pt) == 0) {
        return -1;
    }

    return 0;
}

/* ═══════════════════════════════════════════════════════════════
 * TEST 6: Wrong nonce → authentication failure
 * ═══════════════════════════════════════════════════════════════ */
int test_wrong_nonce(void) {
    uint8_t key[32], nonce[12], wrong_nonce[12];
    memset(key, 0x11, 32);
    memset(nonce, 0x22, 12);
    memset(wrong_nonce, 0x33, 12);

    uint8_t aad[] = {0xAA};
    const char *pt = "Test message";
    size_t pt_len = strlen(pt);

    uint8_t ct[128] = {0};
    uint8_t tag[16] = {0};
    uint8_t out_pt[128] = {0};

    if (chacha20_poly1305_encrypt(key, nonce, aad, sizeof(aad),
                                  (const uint8_t*)pt, pt_len,
                                  ct, tag) != 0) return -1;

    if (chacha20_poly1305_decrypt(key, wrong_nonce, aad, sizeof(aad),
                                  ct, pt_len, tag, out_pt) == 0) {
        return -1;
    }

    return 0;
}

/* ═══════════════════════════════════════════════════════════════
 * TEST 7: Truncated ciphertext → authentication failure
 * ═══════════════════════════════════════════════════════════════ */
int test_truncated_packet(void) {
    uint8_t key[32], nonce[12];
    memset(key, 0x11, 32);
    memset(nonce, 0x22, 12);

    uint8_t aad[] = {0xAA};
    const char *pt = "Test message";
    size_t pt_len = strlen(pt);

    uint8_t ct[128] = {0};
    uint8_t tag[16] = {0};
    uint8_t out_pt[128] = {0};

    if (chacha20_poly1305_encrypt(key, nonce, aad, sizeof(aad),
                                  (const uint8_t*)pt, pt_len,
                                  ct, tag) != 0) return -1;

    /* Truncate by 1 byte */
    if (chacha20_poly1305_decrypt(key, nonce, aad, sizeof(aad),
                                  ct, pt_len - 1, tag, out_pt) == 0) {
        return -1;
    }

    return 0;
}

/* ═══════════════════════════════════════════════════════════════
 * TEST 8: Forged tag (all zeros) → authentication failure
 * ═══════════════════════════════════════════════════════════════ */
int test_tag_forgery(void) {
    uint8_t key[32], nonce[12];
    memset(key, 0x11, 32);
    memset(nonce, 0x22, 12);

    uint8_t aad[] = {0xAA};
    const char *pt = "Test message";
    size_t pt_len = strlen(pt);

    uint8_t ct[128] = {0};
    uint8_t tag[16] = {0};
    uint8_t forged_tag[16] = {0};
    uint8_t out_pt[128] = {0};

    if (chacha20_poly1305_encrypt(key, nonce, aad, sizeof(aad),
                                  (const uint8_t*)pt, pt_len,
                                  ct, tag) != 0) return -1;

    memset(forged_tag, 0, 16);

    if (chacha20_poly1305_decrypt(key, nonce, aad, sizeof(aad),
                                  ct, pt_len, forged_tag, out_pt) == 0) {
        return -1;
    }

    return 0;
}

/* ═══════════════════════════════════════════════════════════════
 * TEST 9: Tampered AAD (capability) → authentication failure
 * ═══════════════════════════════════════════════════════════════ */
int test_aad_tampering(void) {
    uint8_t key[32], nonce[12];
    memset(key, 0x11, 32);
    memset(nonce, 0x22, 12);

    uint8_t aad[] = {0xAA};
    uint8_t tampered_aad[] = {0xBB};
    const char *pt = "Test message";
    size_t pt_len = strlen(pt);

    uint8_t ct[128] = {0};
    uint8_t tag[16] = {0};
    uint8_t out_pt[128] = {0};

    if (chacha20_poly1305_encrypt(key, nonce, aad, sizeof(aad),
                                  (const uint8_t*)pt, pt_len,
                                  ct, tag) != 0) return -1;

    if (chacha20_poly1305_decrypt(key, nonce, tampered_aad, sizeof(tampered_aad),
                                  ct, pt_len, tag, out_pt) == 0) {
        return -1;
    }

    return 0;
}

/* ═══════════════════════════════════════════════════════════════
 * TEST 10: Nonce uniqueness — 1000 CSPRNG nonces must be unique
 * ═══════════════════════════════════════════════════════════════ */
int test_nonce_uniqueness(void) {
    uint8_t nonces[1000][12];

    for (int i = 0; i < 1000; i++) {
        csprng_bytes(nonces[i], 12);
    }

    /* Check all pairs for uniqueness */
    for (int i = 0; i < 1000; i++) {
        for (int j = i + 1; j < 1000; j++) {
            if (memcmp(nonces[i], nonces[j], 12) == 0) {
                return -1; /* Duplicate found */
            }
        }
    }

    return 0;
}

/* ═══════════════════════════════════════════════════════════════
 * TEST 11: Replay detection — seen nonce must be rejected
 * ═══════════════════════════════════════════════════════════════ */
int test_replay_detection(void) {
    ReplayFilter rf;
    replay_filter_init(&rf);

    uint8_t nonce[12] = {0xAB, 0xCD, 0xEF, 0x01, 0x02, 0x03,
                         0x04, 0x05, 0x06, 0x07, 0x08, 0x09};

    /* First check should be fresh (returns 0) */
    if (replay_filter_check(&rf, nonce) != 0) {
        return -1;
    }

    /* Second check should detect replay (returns 1) */
    if (replay_filter_check(&rf, nonce) != 1) {
        return -1;
    }

    /* Different nonce should be fresh */
    uint8_t nonce2[12] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66,
                          0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC};
    if (replay_filter_check(&rf, nonce2) != 0) {
        return -1;
    }

    return 0;
}

/* ═══════════════════════════════════════════════════════════════
 * TEST RUNNER
 * ═══════════════════════════════════════════════════════════════ */

int crypto_test_run_all(void) {
    int passes = 0;
    int fails = 0;

    printf("\n");
    printf("  ╔════════════════════════════════════════════════════════════════╗\n");
    printf("  ║  CRYPTOGRAPHIC NEGATIVE TEST SUITE                           ║\n");
    printf("  ║  ChaCha20-Poly1305 AEAD (RFC 8439) verification             ║\n");
    printf("  ╚════════════════════════════════════════════════════════════════╝\n\n");

#define RUN_TEST(test_func, desc) do { \
    if (test_func() == 0) { \
        printf("  [PASS] %-40s\n", desc); \
        passes++; \
    } else { \
        printf("  [FAIL] %-40s\n", desc); \
        fails++; \
    } \
} while(0)

    RUN_TEST(test_chacha20_rfc_vector,  "ChaCha20 RFC 7539 test vector");
    RUN_TEST(test_poly1305_rfc_vector,  "Poly1305 RFC 8439 test vector");
    RUN_TEST(test_aead_encrypt_decrypt, "AEAD encrypt/decrypt round-trip");
    RUN_TEST(test_modified_ciphertext,  "Modified ciphertext rejected");
    RUN_TEST(test_wrong_key,            "Wrong key rejected");
    RUN_TEST(test_wrong_nonce,          "Wrong nonce rejected");
    RUN_TEST(test_truncated_packet,     "Truncated packet rejected");
    RUN_TEST(test_tag_forgery,          "Forged tag rejected");
    RUN_TEST(test_aad_tampering,        "Tampered AAD rejected");
    RUN_TEST(test_nonce_uniqueness,     "1000 CSPRNG nonces unique");
    RUN_TEST(test_replay_detection,     "Replay detection works");

#undef RUN_TEST

    printf("\n  ──────────────────────────────────────────────────\n");
    printf("  Total: %d | Passed: %d | Failed: %d\n", passes + fails, passes, fails);

    if (fails == 0) {
        printf("  ✓ ALL CRYPTO TESTS PASSED\n");
    } else {
        printf("  ✗ %d TEST(S) FAILED\n", fails);
    }
    printf("\n");

    return (fails == 0) ? 0 : -1;
}
