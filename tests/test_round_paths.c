/* Internal dispatch equivalence check: same initialized 16-round session,
 * generic loop vs unrolled path, including decrypt of arbitrary ciphertext.
 * Include source directly to avoid exposing testing hooks in the public API.
 */
#include "../src/spiral_atcg.c"

int main(void) {
    uint8_t key[32], nonce[16], sid[16], input[40], a[40], b[40];
    for (int policy = 0; policy < 4; ++policy) {
        for (unsigned trial = 0; trial < 32; ++trial) {
            for (unsigned i = 0; i < 32; ++i) key[i] = (uint8_t)(i * 7 + trial);
            for (unsigned i = 0; i < 16; ++i) {
                nonce[i] = (uint8_t)(i * 11 + trial * 3);
                sid[i] = (uint8_t)(i * 13 + trial * 5);
            }
            for (unsigned i = 0; i < 40; ++i) input[i] = (uint8_t)(i * 17 + trial * 7);
            spiral_atcg_ctx_t ctx;
            if (spiral_atcg_init_session(&ctx, key, nonce, sid, 16,
                                        (spiral_atcg_policy_t)policy) != SPIRAL_ATCG_OK) return 1;
            encrypt_block16_fast_scalar(&ctx, input, a);
            encrypt_block_generic_scalar(&ctx, input, b);
            if (memcmp(a, b, 40)) return 2;
            decrypt_block16_fast_scalar(&ctx, input, a);
            decrypt_block_generic_scalar(&ctx, input, b);
            if (memcmp(a, b, 40)) return 3;
        }
    }
    puts("FAST/GENERIC PATHS PASSED (128 sessions, encrypt and decrypt)");
    return 0;
}
