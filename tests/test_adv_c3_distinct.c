#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "spiral_atcg.h"

static uint64_t splitmix(uint64_t *s) {
    uint64_t z = (*s += 0x9E3779B97F4A7C15ULL);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}
static void fill_random(uint8_t *p, size_t n, uint64_t *st) {
    for (size_t i = 0; i + 8 <= n; i += 8) {
        uint64_t v = splitmix(st);
        memcpy(p + i, &v, 8);
    }
}
static void distinct_positions(int *out, int order, uint64_t *st) {
    int taken[320] = {0};
    int got = 0;
    while (got < order) {
        int p = (int)(splitmix(st) % 320);
        if (!taken[p]) { taken[p] = 1; out[got++] = p; }
    }
}

int main(void) {
    const char *names[4] = {"BASE", "SAFE", "AGGR", "NULL"};
    spiral_atcg_policy_t policies[4] = {
        SPIRAL_ATCG_POLICY_BASE, SPIRAL_ATCG_POLICY_SAFE,
        SPIRAL_ATCG_POLICY_AGGR, SPIRAL_ATCG_POLICY_NULL
    };

    printf("Higher-order differential at orders 4, 8 (DISTINCT positions, 32 trials each):\n");
    printf("%-6s | order=4 (mean ± std)        | order=8 (mean ± std)\n", "policy");

    for (int p = 0; p < 4; p++) {
        for (int order = 4; order <= 8; order += 4) {
            uint64_t st = 0xDEADBEEF00000000ULL ^ ((uint64_t)p * 0x9E3779B97F4A7C15ULL ^ order);
            int N = 32;
            long sum = 0, sumsq = 0;
            int mn = 320, mx = 0;
            for (int trial = 0; trial < N; trial++) {
                uint8_t master[32], nonce[16], sid[16];
                fill_random(master, 32, &st);
                fill_random(nonce, 16, &st);
                fill_random(sid, 16, &st);
                spiral_atcg_ctx_t ctx;
                spiral_atcg_init_session(&ctx, master, nonce, sid, 16, policies[p]);
                uint8_t base[40];
                fill_random(base, 40, &st);
                int positions[8];
                distinct_positions(positions, order, &st);
                uint8_t accum[40] = {0};
                for (long mask = 0; mask < (1L << order); mask++) {
                    uint8_t pt[40];
                    memcpy(pt, base, 40);
                    for (int b = 0; b < order; b++) {
                        if ((mask >> b) & 1) pt[positions[b]/8] ^= (uint8_t)(1u << (positions[b]%8));
                    }
                    uint8_t ct[40];
                    spiral_atcg_encrypt_block(&ctx, pt, ct);
                    for (int j = 0; j < 40; j++) accum[j] ^= ct[j];
                }
                int hw = 0;
                for (int j = 0; j < 40; j++) {
                    uint8_t x = accum[j];
                    while (x) { hw += x & 1; x >>= 1; }
                }
                sum += hw; sumsq += (long)hw*hw;
                if (hw < mn) mn = hw;
                if (hw > mx) mx = hw;
            }
            double mean = (double)sum/N;
            double var = ((double)sumsq - (double)N*mean*mean)/(N-1);
            double std = var > 0 ? __builtin_sqrt(var) : 0;
            if (order == 4) printf("%-6s | mean=%6.2f std=%5.2f [%d,%d]", names[p], mean, std, mn, mx);
            else            printf(" | mean=%6.2f std=%5.2f [%d,%d]\n", mean, std, mn, mx);
        }
    }
    return 0;
}
