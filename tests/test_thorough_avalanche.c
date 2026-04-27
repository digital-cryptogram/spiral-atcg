/* Thorough avalanche probe for v2.2-strength.
 * For each policy, tests all 320 plaintext bit positions across 8 (seed,nonce,sid) tuples.
 * Reports per-policy: avg HD, min HD, max HD, fraction of (bit, tuple) cells with all 10 lanes active. */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "spiral_atcg.h"

static unsigned popcnt8(uint8_t x) {
    unsigned c = 0;
    while (x) { c += (unsigned)(x & 1u); x >>= 1; }
    return c;
}

static unsigned hd_block(const uint8_t *a, const uint8_t *b) {
    unsigned h = 0;
    for (int i = 0; i < 40; i++) h += popcnt8((uint8_t)(a[i] ^ b[i]));
    return h;
}

static unsigned active_lanes(const uint8_t *a, const uint8_t *b) {
    unsigned act = 0;
    for (int lane = 0; lane < 10; lane++) {
        unsigned lh = 0;
        for (int b2 = 0; b2 < 4; b2++) lh += popcnt8((uint8_t)(a[lane*4+b2] ^ b[lane*4+b2]));
        if (lh != 0) act++;
    }
    return act;
}

/* Inline SplitMix64 */
static uint64_t splitmix(uint64_t *s) {
    uint64_t z = (*s += 0x9E3779B97F4A7C15ULL);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}

static void make_tuple(uint64_t seed, uint8_t master[32], uint8_t nonce[16], uint8_t sid[16], uint8_t pt[40]) {
    uint64_t st = seed;
    for (int i = 0; i < 32; i += 8) { uint64_t v = splitmix(&st); memcpy(master + i, &v, 8); }
    for (int i = 0; i < 16; i += 8) { uint64_t v = splitmix(&st); memcpy(nonce  + i, &v, 8); }
    for (int i = 0; i < 16; i += 8) { uint64_t v = splitmix(&st); memcpy(sid    + i, &v, 8); }
    for (int i = 0; i < 40; i += 8) { uint64_t v = splitmix(&st); memcpy(pt     + i, &v, 8); }
}

static void run_policy(spiral_atcg_policy_t policy) {
    const int TUPLES = 8;
    const int BIT_POS = 320;
    long total_hd = 0, total_cells = 0;
    unsigned min_hd = UINT32_MAX, max_hd = 0;
    long all_lanes_active_count = 0;
    long zero_diff_count = 0;

    for (int t = 0; t < TUPLES; t++) {
        uint8_t master[32], nonce[16], sid[16], pt[40], pt_flip[40];
        uint8_t ct[40], ct_flip[40];
        make_tuple(0xDEADBEEF12345678ULL ^ ((uint64_t)t * 0x9E3779B97F4A7C15ULL),
                   master, nonce, sid, pt);

        spiral_atcg_ctx_t ctx;
        spiral_atcg_init_session(&ctx, master, nonce, sid, SPIRAL_ATCG_DEFAULT_ROUNDS, policy);
        spiral_atcg_encrypt_block(&ctx, pt, ct);

        for (int bit = 0; bit < BIT_POS; bit++) {
            memcpy(pt_flip, pt, 40);
            pt_flip[bit / 8] ^= (uint8_t)(1u << (bit % 8));
            spiral_atcg_encrypt_block(&ctx, pt_flip, ct_flip);

            unsigned hd = hd_block(ct, ct_flip);
            unsigned al = active_lanes(ct, ct_flip);

            total_hd += hd;
            total_cells++;
            if (hd < min_hd) min_hd = hd;
            if (hd > max_hd) max_hd = hd;
            if (al == 10) all_lanes_active_count++;
            if (hd == 0) zero_diff_count++;
        }
    }

    double avg = (double)total_hd / (double)total_cells;
    double full_diff_pct = 100.0 * (double)all_lanes_active_count / (double)total_cells;

    printf("[%-4s]  cells=%ld  avg_hd=%6.2f  min=%3u  max=%3u  all-10-lanes=%5.2f%%  zero-diff=%ld\n",
           spiral_atcg_policy_name(policy), total_cells, avg, min_hd, max_hd, full_diff_pct, zero_diff_count);
}

int main(void) {
    printf("Spiral-ATCG v%s thorough avalanche probe\n", SPIRAL_ATCG_VERSION_STRING);
    printf("8 tuples x 320 plaintext bit positions = 2560 cells per policy\n\n");
    run_policy(SPIRAL_ATCG_POLICY_BASE);
    run_policy(SPIRAL_ATCG_POLICY_SAFE);
    run_policy(SPIRAL_ATCG_POLICY_AGGR);
    run_policy(SPIRAL_ATCG_POLICY_NULL);
    return 0;
}
