/* Reject the v2.2.4 full-round affine parity and constant-difference flaws.
 * Finite regression checks, NOT a bound on cryptanalytic security.
 * All round counts get roundtrip/in-place checks. Structural checks apply
 * only at 16..32 rounds; reduced-round Feistel is for research, not security.
 */
#include "spiral_atcg.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static uint64_t rng = UINT64_C(0x2026110950495241);
static uint8_t next_byte(void) {
    uint64_t z = (rng += UINT64_C(0x9e3779b97f4a7c15));
    z = (z ^ (z >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    z = (z ^ (z >> 27)) * UINT64_C(0x94d049bb133111eb);
    return (uint8_t)(z ^ (z >> 31));
}
static void fill(uint8_t *p, size_t n) {
    for (size_t i = 0; i < n; ++i) p[i] = next_byte();
}
static unsigned parity(const uint8_t p[40], uint8_t mask) {
    unsigned v = 0;
    for (unsigned i = 0; i < 40; ++i) {
        unsigned x = p[i] & mask;
        while (x) { v ^= x & 1u; x >>= 1; }
    }
    return v;
}
#define OK(call) do { if ((call) != SPIRAL_ATCG_OK) { \
    fprintf(stderr, "API error at line %d\n", __LINE__); return 1; } } while (0)

int main(void) {
    const uint8_t masks[3] = {0x55, 0xaa, 0xff};
    unsigned roundtrips = 0, structural_samples = 0, differences = 0;
    for (int policy = 0; policy < 4; ++policy) {
        for (unsigned rounds = 1; rounds <= SPIRAL_ATCG_MAX_ROUNDS; ++rounds) {
            for (unsigned session = 0; session < 4; ++session) {
                uint8_t key[32], nonce[16], sid[16];
                spiral_atcg_ctx_t ctx;
                unsigned matches[3][3] = {{0}}, varied[3] = {0};
                uint8_t first_diff[3][40] = {{0}};
                fill(key, sizeof key); fill(nonce, sizeof nonce); fill(sid, sizeof sid);
                OK(spiral_atcg_init_session(&ctx, key, nonce, sid, rounds,
                                           (spiral_atcg_policy_t)policy));
                for (unsigned sample = 0; sample < 32; ++sample) {
                    uint8_t p[40], c[40], shifted_p[40], shifted_c[40], inplace[40];
                    fill(p, sizeof p);
                    OK(spiral_atcg_encrypt_block(&ctx, p, c));
                    memcpy(inplace, p, 40);
                    OK(spiral_atcg_encrypt_block(&ctx, inplace, inplace));
                    if (memcmp(inplace, c, 40)) return 2;
                    OK(spiral_atcg_decrypt_block(&ctx, inplace, inplace));
                    if (memcmp(inplace, p, 40)) return 2;
                    ++roundtrips;
                    if (rounds < SPIRAL_ATCG_DEFAULT_ROUNDS) continue;
                    ++structural_samples;
                    for (unsigned a = 0; a < 3; ++a)
                        for (unsigned b = 0; b < 3; ++b)
                            matches[a][b] += parity(p, masks[a]) == parity(c, masks[b]);
                    for (unsigned m = 0; m < 3; ++m) {
                        uint8_t diff[40];
                        for (unsigned b = 0; b < 40; ++b) shifted_p[b] = p[b] ^ masks[m];
                        OK(spiral_atcg_encrypt_block(&ctx, shifted_p, shifted_c));
                        for (unsigned b = 0; b < 40; ++b) diff[b] = c[b] ^ shifted_c[b];
                        if (!sample) memcpy(first_diff[m], diff, 40);
                        else if (memcmp(first_diff[m], diff, 40)) varied[m] = 1;
                        OK(spiral_atcg_decrypt_block(&ctx, shifted_c, inplace));
                        if (memcmp(inplace, shifted_p, 40)) return 2;
                        ++differences;
                    }
                }
                if (rounds < SPIRAL_ATCG_DEFAULT_ROUNDS) continue;
                for (unsigned a = 0; a < 3; ++a) {
                    for (unsigned b = 0; b < 3; ++b) {
                        if (matches[a][b] == 0 || matches[a][b] == 32) {
                            fprintf(stderr, "FAIL affine parity: policy=%s rounds=%u session=%u input=%02x output=%02x matches=%u/32\n",
                                    spiral_atcg_policy_name((spiral_atcg_policy_t)policy), rounds, session,
                                    masks[a], masks[b], matches[a][b]);
                            return 3;
                        }
                    }
                    if (!varied[a]) {
                        fprintf(stderr, "FAIL constant differential: policy=%s rounds=%u session=%u delta=%02x\n",
                                spiral_atcg_policy_name((spiral_atcg_policy_t)policy), rounds, session, masks[a]);
                        return 4;
                    }
                }
            }
        }
        printf("V2.2.5 REGRESSION PASSED policy=%s (roundtrip 1..32, structural 16..32)\n",
               spiral_atcg_policy_name((spiral_atcg_policy_t)policy));
    }
    printf("roundtrip/inplace=%u structural_samples=%u differential_samples=%u\n",
           roundtrips, structural_samples, differences);
    return 0;
}
