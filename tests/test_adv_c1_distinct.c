/* Re-test C1 algebraic degree with NON-COLLIDING cube positions.
 * The earlier C1 probe drew positions with replacement, allowing duplicates;
 * a duplicate position causes the cube to collapse and produce 0 trivially.
 * This version enforces distinct positions. */
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

/* Generate 'order' DISTINCT positions in [0, 320) */
static void distinct_positions(int *out, int order, uint64_t *st) {
    int taken[320] = {0};
    int got = 0;
    while (got < order) {
        int p = (int)(splitmix(st) % 320);
        if (!taken[p]) {
            taken[p] = 1;
            out[got++] = p;
        }
    }
}

static int popcount40(const uint8_t *a) {
    int hw = 0;
    for (int j = 0; j < 40; j++) {
        uint8_t x = a[j];
        while (x) { hw += x & 1; x >>= 1; }
    }
    return hw;
}

static int test_one(spiral_atcg_policy_t policy, int order, uint64_t *st) {
    uint8_t master[32], nonce[16], sid[16];
    fill_random(master, 32, st);
    fill_random(nonce, 16, st);
    fill_random(sid, 16, st);
    spiral_atcg_ctx_t ctx;
    spiral_atcg_init_session(&ctx, master, nonce, sid, 16, policy);

    uint8_t base[40];
    fill_random(base, 40, st);
    int positions[16];
    distinct_positions(positions, order, st);

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
    return popcount40(accum);
}

int main(void) {
    const char *names[4] = {"BASE", "SAFE", "AGGR", "NULL"};
    spiral_atcg_policy_t policies[4] = {
        SPIRAL_ATCG_POLICY_BASE, SPIRAL_ATCG_POLICY_SAFE,
        SPIRAL_ATCG_POLICY_AGGR, SPIRAL_ATCG_POLICY_NULL
    };

    int orders[2] = {8, 16};
    int N = 16;
    printf("Per-trial Hamming weight of n-th order derivative\n");
    printf("(positions enforced DISTINCT; N=%d trials per cell)\n", N);
    printf("Expected: 160 ± ~9 for random; HD == 0 means low algebraic degree\n\n");

    for (int oi = 0; oi < 2; oi++) {
        int order = orders[oi];
        printf("=== %d-th order derivative ===\n", order);
        printf("%-6s | trials...                                                  | mean   min   max  zeros\n", "policy");
        for (int p = 0; p < 4; p++) {
            uint64_t st = 0xDEADBEEF00000000ULL ^ ((uint64_t)p * 0x9E3779B97F4A7C15ULL);
            int sum = 0, zeros = 0, mn = 320, mx = 0;
            int hws[16];
            for (int t = 0; t < N; t++) {
                hws[t] = test_one(policies[p], order, &st);
                sum += hws[t];
                if (hws[t] == 0) zeros++;
                if (hws[t] < mn) mn = hws[t];
                if (hws[t] > mx) mx = hws[t];
            }
            printf("%-6s |", names[p]);
            for (int t = 0; t < N; t++) printf(" %3d", hws[t]);
            printf(" | %5.1f  %3d  %3d  %5d\n", (double)sum/N, mn, mx, zeros);
        }
        printf("\n");
    }
    return 0;
}
