#ifndef AIRBOT_CRYPTO_TEST_H
#define AIRBOT_CRYPTO_TEST_H

#include <stdint.h>

/*
 * ==============================================================================
 *  CRYPTO NEGATIVE TEST SUITE
 * ==============================================================================
 */

int test_chacha20_rfc_vector(void);
int test_poly1305_rfc_vector(void);
int test_aead_encrypt_decrypt(void);
int test_modified_ciphertext(void);
int test_wrong_key(void);
int test_wrong_nonce(void);
int test_truncated_packet(void);
int test_tag_forgery(void);
int test_aad_tampering(void);
int test_nonce_uniqueness(void);
int test_replay_detection(void);

/**
 * Run all crypto test cases.
 * Returns 0 if all tests pass, -1 if any test fails.
 */
int crypto_test_run_all(void);

#endif /* AIRBOT_CRYPTO_TEST_H */
