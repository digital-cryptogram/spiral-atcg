/* Search for exact affine input/output relations at the default 16 rounds.
 * Rows are [plaintext bits, ciphertext bits, 1]. Rank 641 gives witnesses
 * excluding every nonzero affine relation for each tested session.
 * It does not exclude biased linear approximations or weak untested keys.
 */
#include "spiral_atcg.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define COLS 641
#define WORDS 11
static uint64_t rng = UINT64_C(0x225aff1e20261109);
static uint8_t next_byte(void) {
    uint64_t z = (rng += UINT64_C(0x9e3779b97f4a7c15));
    z = (z ^ (z >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    z = (z ^ (z >> 27)) * UINT64_C(0x94d049bb133111eb);
    return (uint8_t)(z ^ (z >> 31));
}
static void fill(uint8_t *p, size_t n) {
    for (size_t i = 0; i < n; ++i) p[i] = next_byte();
}
int main(void) {
    for (int policy = 0; policy < 4; ++policy) {
        for (unsigned session = 0; session < 4; ++session) {
            uint64_t basis[COLS][WORDS] = {{0}};
            unsigned rank = 0;
            uint8_t key[32], nonce[16], sid[16];
            spiral_atcg_ctx_t ctx;
            fill(key, sizeof key); fill(nonce, sizeof nonce); fill(sid, sizeof sid);
            if (spiral_atcg_init_session(&ctx, key, nonce, sid, SPIRAL_ATCG_DEFAULT_ROUNDS,
                                        (spiral_atcg_policy_t)policy) != SPIRAL_ATCG_OK) return 1;
            for (unsigned sample = 0; sample < 768; ++sample) {
                uint8_t pc[80];
                uint64_t row[WORDS] = {0};
                fill(pc, 40);
                if (spiral_atcg_encrypt_block(&ctx, pc, pc + 40) != SPIRAL_ATCG_OK) return 1;
                for (unsigned b = 0; b < 80; ++b) row[b / 8] |= (uint64_t)pc[b] << (8 * (b % 8));
                row[10] = 1; /* affine constant */
                for (unsigned col = 0; col < COLS; ++col) {
                    unsigned word = col / 64;
                    uint64_t bit = UINT64_C(1) << (col % 64);
                    if (!(row[word] & bit)) continue;
                    if (basis[col][word] & bit) {
                        for (unsigned w = word; w < WORDS; ++w) row[w] ^= basis[col][w];
                    } else {
                        memcpy(basis[col], row, sizeof row);
                        ++rank;
                        break;
                    }
                }
            }
            if (rank != COLS) {
                fprintf(stderr, "AFFINE RANK FAIL policy=%s session=%u rank=%u/641\n",
                        spiral_atcg_policy_name((spiral_atcg_policy_t)policy), session, rank);
                return 2;
            }
        }
        printf("AFFINE RANK PASSED policy=%s: 4 sessions, rank=641/641 each\n",
               spiral_atcg_policy_name((spiral_atcg_policy_t)policy));
    }
    return 0;
}
