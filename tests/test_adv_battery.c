/* Spiral-ATCG v2.2.4 advanced cryptanalytic battery.
 *
 * Three attack families beyond the basic security and quantum batteries:
 *
 * Family A: Quantum walk indicators (Feistel-style attacks)
 *   A1. Round-function bijectivity probe (counts collisions)
 *   A2. Kuwakado-Morii pattern test (4-round Simon distinguisher signature)
 *   A3. Inner-state recurrence test (Feistel-state period structure)
 *
 * Family B: Boomerang distinguisher
 *   B1. Standard boomerang quartet test at multiple delta sizes
 *   B2. Sandwich boomerang (3-segment switch point)
 *   B3. Related-key boomerang
 *
 * Family C: Algebraic attacks
 *   C1. Algebraic degree (per-output-bit) lower bound via random walk on inputs
 *   C2. Cube attack indicator (low-degree polynomial recovery test)
 *   C3. Higher-order differential (4th-order, 8th-order)
 *   C4. Linear approximation table (LAT) sample
 *
 * Output: structured human-readable summary + machine CSV. */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "spiral_atcg.h"

#define SAMPLES 4096

typedef struct {
    spiral_atcg_policy_t policy;
    const char *name;
} pol_t;
static const pol_t POLICIES[4] = {
    {SPIRAL_ATCG_POLICY_BASE, "BASE"},
    {SPIRAL_ATCG_POLICY_SAFE, "SAFE"},
    {SPIRAL_ATCG_POLICY_AGGR, "AGGR"},
    {SPIRAL_ATCG_POLICY_NULL, "NULL"},
};

static unsigned popcnt8(uint8_t x) { unsigned c=0; while(x){c+=x&1u;x>>=1;} return c; }
static unsigned hd320(const uint8_t *a, const uint8_t *b) {
    unsigned h = 0;
    for (int i = 0; i < 40; i++) h += popcnt8(a[i] ^ b[i]);
    return h;
}
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
    for (size_t i = (n / 8) * 8; i < n; i++) p[i] = (uint8_t)(splitmix(st) & 0xFF);
}
static void make_session(uint64_t *st, spiral_atcg_ctx_t *ctx, spiral_atcg_policy_t policy, uint32_t rounds) {
    uint8_t master[32], nonce[16], sid[16];
    fill_random(master, 32, st);
    fill_random(nonce, 16, st);
    fill_random(sid, 16, st);
    spiral_atcg_init_session(ctx, master, nonce, sid, rounds, policy);
}

/* =================================================================
 * FAMILY A: Quantum walk indicators
 * ================================================================= */

/* A1. Bijectivity probe: with fixed K, count P -> E_K(P) collisions.
 *     For a permutation: 0 collisions in N samples.
 *     For a non-bijective round function: collisions appear.
 *     We use a 32-bit prefix to keep memory bounded; expected random
 *     collisions in 4096 samples on 32-bit hash ≈ N²/2^33 ≈ 1.95. */
static long probe_a1_bijectivity(spiral_atcg_policy_t policy, uint64_t seed) {
    uint64_t st = seed;
    spiral_atcg_ctx_t ctx;
    make_session(&st, &ctx, policy, SPIRAL_ATCG_DEFAULT_ROUNDS);

    uint32_t *prefix = malloc(SAMPLES * sizeof(uint32_t));
    if (!prefix) return -1;

    for (int i = 0; i < SAMPLES; i++) {
        uint8_t pt[40], ct[40];
        fill_random(pt, 40, &st);
        spiral_atcg_encrypt_block(&ctx, pt, ct);
        memcpy(&prefix[i], ct, 4);
    }

    /* Sort + count adjacent dups */
    long collisions = 0;
    for (int i = 0; i < SAMPLES; i++) {
        for (int j = i + 1; j < SAMPLES; j++) {
            if (prefix[i] == prefix[j]) collisions++;
        }
    }
    free(prefix);
    return collisions;
}

/* A2. Kuwakado-Morii pattern: for hidden period s on 4-round Feistel,
 *     E(0||x) xor E(0||x xor s) is constant in x.
 *     We test the Spiral-ATCG variant with 5/5 split: probe
 *     E(0_left || x_right) xor E(0_left || (x_right xor s)) over fixed prefix.
 *     Test 16 candidate s patterns at reduced rounds {2, 4, 6}.
 *     Report worst single-byte concentration of the xor result. */
static double probe_a2_kuwakado_morii(spiral_atcg_policy_t policy, uint64_t seed,
                                        uint32_t rounds) {
    uint64_t st = seed;
    spiral_atcg_ctx_t ctx;
    make_session(&st, &ctx, policy, rounds);

    double worst = 0.0;
    for (int sb = 0; sb < 16; sb++) {
        uint8_t s_pattern[40];
        memset(s_pattern, 0, 40);
        s_pattern[20 + (sb / 8)] = (uint8_t)(1u << (sb % 8));  /* perturb right half */

        int byte_freq[256];
        memset(byte_freq, 0, sizeof(byte_freq));
        int N = 1024;
        for (int i = 0; i < N; i++) {
            uint8_t pt1[40], pt2[40], ct1[40], ct2[40];
            memset(pt1, 0, 40);  /* zero left half */
            uint8_t rnd[20];
            fill_random(rnd, 20, &st);
            memcpy(pt1 + 20, rnd, 20);  /* random right half */
            memcpy(pt2, pt1, 40);
            for (int b = 0; b < 40; b++) pt2[b] ^= s_pattern[b];
            spiral_atcg_encrypt_block(&ctx, pt1, ct1);
            spiral_atcg_encrypt_block(&ctx, pt2, ct2);
            byte_freq[ct1[0] ^ ct2[0]]++;
        }
        int mx = 0;
        for (int i = 0; i < 256; i++) if (byte_freq[i] > mx) mx = byte_freq[i];
        double consist = (double)mx / (double)N;
        if (consist > worst) worst = consist;
    }
    return worst;
}

/* A3. Inner-state recurrence: encrypt P, then re-encrypt that ciphertext, etc.
 *     For each step, measure HD with original. A short-period structure would
 *     cause the trajectory to revisit nearby states. */
static void probe_a3_recurrence(spiral_atcg_policy_t policy, uint64_t seed,
                                  unsigned *min_self_hd, unsigned *steps_to_min) {
    uint64_t st = seed;
    spiral_atcg_ctx_t ctx;
    make_session(&st, &ctx, policy, SPIRAL_ATCG_DEFAULT_ROUNDS);

    uint8_t state[40], original[40];
    fill_random(state, 40, &st);
    memcpy(original, state, 40);

    *min_self_hd = 320;
    *steps_to_min = 0;
    int N_STEPS = 256;
    for (int s = 1; s <= N_STEPS; s++) {
        uint8_t next[40];
        spiral_atcg_encrypt_block(&ctx, state, next);
        memcpy(state, next, 40);
        unsigned h = hd320(original, state);
        if (h < *min_self_hd) {
            *min_self_hd = h;
            *steps_to_min = s;
        }
    }
}

/* =================================================================
 * FAMILY B: Boomerang distinguisher
 * ================================================================= */

/* B1. Standard boomerang quartet:
 *     Given two random differences (alpha, beta), construct quartets:
 *       P1 random, P2 = P1 xor alpha
 *       C1 = E(P1), C2 = E(P2)
 *       C3 = C1 xor beta, C4 = C2 xor beta
 *       P3 = D(C3), P4 = D(C4)
 *     Boomerang signature: P3 xor P4 = alpha (with non-trivial probability).
 *     For random permutation, P3 xor P4 = alpha with probability 2^-320.
 *     Report fraction of N quartets where P3 xor P4 EXACTLY equals alpha,
 *     plus average HD between (P3 xor P4) and alpha. */
static void probe_b1_boomerang(spiral_atcg_policy_t policy, uint64_t seed,
                                long *exact_returns, double *avg_hd_to_alpha) {
    uint64_t st = seed;
    spiral_atcg_ctx_t ctx;
    make_session(&st, &ctx, policy, SPIRAL_ATCG_DEFAULT_ROUNDS);

    uint8_t alpha[40], beta[40];
    /* Use small-Hamming-weight differences (concentrated boomerang signature) */
    memset(alpha, 0, 40);
    memset(beta, 0, 40);
    alpha[0] = 0x01;     /* single bit alpha */
    beta[20] = 0x01;     /* single bit beta in opposite half */

    *exact_returns = 0;
    long sum_hd = 0;
    int n = 0;
    for (int i = 0; i < SAMPLES; i++) {
        uint8_t P1[40], P2[40], C1[40], C2[40], C3[40], C4[40], P3[40], P4[40];
        fill_random(P1, 40, &st);
        for (int b = 0; b < 40; b++) P2[b] = P1[b] ^ alpha[b];

        spiral_atcg_encrypt_block(&ctx, P1, C1);
        spiral_atcg_encrypt_block(&ctx, P2, C2);
        for (int b = 0; b < 40; b++) {
            C3[b] = C1[b] ^ beta[b];
            C4[b] = C2[b] ^ beta[b];
        }
        spiral_atcg_decrypt_block(&ctx, C3, P3);
        spiral_atcg_decrypt_block(&ctx, C4, P4);

        uint8_t observed[40];
        for (int b = 0; b < 40; b++) observed[b] = P3[b] ^ P4[b];
        if (memcmp(observed, alpha, 40) == 0) (*exact_returns)++;
        sum_hd += hd320(observed, alpha);
        n++;
    }
    *avg_hd_to_alpha = (double)sum_hd / (double)n;
}

/* B2. Sandwich boomerang (Biryukov-Khovratovich): same as B1 but at reduced
 *     rounds, simulating split-attack at the "middle" of the cipher.
 *     We test rounds 4 and 8. Same metric. */
static void probe_b2_sandwich(spiral_atcg_policy_t policy, uint64_t seed,
                                uint32_t rounds, long *exact_returns,
                                double *avg_hd) {
    uint64_t st = seed;
    spiral_atcg_ctx_t ctx;
    make_session(&st, &ctx, policy, rounds);

    uint8_t alpha[40], beta[40];
    memset(alpha, 0, 40);
    memset(beta, 0, 40);
    alpha[0] = 0x80;
    beta[20] = 0x80;

    *exact_returns = 0;
    long sum_hd = 0;
    for (int i = 0; i < SAMPLES; i++) {
        uint8_t P1[40], P2[40], C1[40], C2[40], C3[40], C4[40], P3[40], P4[40];
        fill_random(P1, 40, &st);
        for (int b = 0; b < 40; b++) P2[b] = P1[b] ^ alpha[b];
        spiral_atcg_encrypt_block(&ctx, P1, C1);
        spiral_atcg_encrypt_block(&ctx, P2, C2);
        for (int b = 0; b < 40; b++) {
            C3[b] = C1[b] ^ beta[b];
            C4[b] = C2[b] ^ beta[b];
        }
        spiral_atcg_decrypt_block(&ctx, C3, P3);
        spiral_atcg_decrypt_block(&ctx, C4, P4);
        uint8_t obs[40];
        for (int b = 0; b < 40; b++) obs[b] = P3[b] ^ P4[b];
        if (memcmp(obs, alpha, 40) == 0) (*exact_returns)++;
        sum_hd += hd320(obs, alpha);
    }
    *avg_hd = (double)sum_hd / (double)SAMPLES;
}

/* B3. Related-key boomerang: same boomerang with key delta on E side. */
static void probe_b3_rk_boomerang(spiral_atcg_policy_t policy, uint64_t seed,
                                    long *exact, double *avg_hd) {
    uint64_t st = seed;
    uint8_t master[32], master2[32], nonce[16], sid[16];
    fill_random(master, 32, &st);
    memcpy(master2, master, 32);
    master2[0] ^= 0x40;   /* small key delta */
    fill_random(nonce, 16, &st);
    fill_random(sid, 16, &st);

    spiral_atcg_ctx_t ctx, ctx2;
    spiral_atcg_init_session(&ctx,  master,  nonce, sid, SPIRAL_ATCG_DEFAULT_ROUNDS, policy);
    spiral_atcg_init_session(&ctx2, master2, nonce, sid, SPIRAL_ATCG_DEFAULT_ROUNDS, policy);

    uint8_t alpha[40] = {0}, beta[40] = {0};
    alpha[0] = 0x01; beta[20] = 0x01;

    *exact = 0;
    long sum_hd = 0;
    for (int i = 0; i < SAMPLES; i++) {
        uint8_t P1[40], P2[40], C1[40], C2[40], C3[40], C4[40], P3[40], P4[40];
        fill_random(P1, 40, &st);
        for (int b = 0; b < 40; b++) P2[b] = P1[b] ^ alpha[b];
        spiral_atcg_encrypt_block(&ctx, P1, C1);
        spiral_atcg_encrypt_block(&ctx, P2, C2);
        for (int b = 0; b < 40; b++) { C3[b] = C1[b] ^ beta[b]; C4[b] = C2[b] ^ beta[b]; }
        spiral_atcg_decrypt_block(&ctx2, C3, P3);
        spiral_atcg_decrypt_block(&ctx2, C4, P4);
        uint8_t obs[40];
        for (int b = 0; b < 40; b++) obs[b] = P3[b] ^ P4[b];
        if (memcmp(obs, alpha, 40) == 0) (*exact)++;
        sum_hd += hd320(obs, alpha);
    }
    *avg_hd = (double)sum_hd / (double)SAMPLES;
}

/* =================================================================
 * FAMILY C: Algebraic attacks
 * ================================================================= */

/* C1. Algebraic degree probe via higher-order differentials:
 *     If output bit b has algebraic degree d, then the d-th order derivative
 *     in any direction is non-zero, but the (d+1)-th and higher are all zero.
 *     We test d = {1, 2, 4, 8, 16} by computing the XOR sum over a
 *     d-dimensional subcube and checking if it's zero (degree < d) or non-zero (degree >= d).
 *     Healthy cipher: degree saturates at 320 (max); 16th-order derivative non-zero. */
static void probe_c1_alg_degree(spiral_atcg_policy_t policy, uint64_t seed,
                                  int *deg_results /* 5 entries */) {
    uint64_t st = seed;
    spiral_atcg_ctx_t ctx;
    make_session(&st, &ctx, policy, SPIRAL_ATCG_DEFAULT_ROUNDS);

    uint8_t base[40];
    fill_random(base, 40, &st);

    int orders[5] = {1, 2, 4, 8, 16};

    for (int oi = 0; oi < 5; oi++) {
        int order = orders[oi];
        /* Choose 'order' random positions to flip */
        int positions[16];
        for (int j = 0; j < order; j++) {
            positions[j] = (int)(splitmix(&st) % 320);
        }
        /* XOR sum over 2^order subcube */
        uint8_t accum[40] = {0};
        for (long mask = 0; mask < (1L << order); mask++) {
            uint8_t pt[40];
            memcpy(pt, base, 40);
            for (int b = 0; b < order; b++) {
                if ((mask >> b) & 1) {
                    pt[positions[b] / 8] ^= (uint8_t)(1u << (positions[b] % 8));
                }
            }
            uint8_t ct[40];
            spiral_atcg_encrypt_block(&ctx, pt, ct);
            for (int j = 0; j < 40; j++) accum[j] ^= ct[j];
        }
        /* Result = 0 means the 'order'-th derivative vanishes (low degree).
         * Result != 0 means degree >= order. */
        int nonzero = 0;
        for (int j = 0; j < 40; j++) if (accum[j] != 0) { nonzero = 1; break; }
        deg_results[oi] = nonzero;
    }
}

/* C2. Cube attack indicator:
 *     For each cube of dimension 'd', the superpoly p(x) is a polynomial in
 *     the remaining input bits. If degree(p) is small, cube attack works.
 *     We approximate by computing the cube sum, and checking if it equals
 *     a constant (= deg-0 superpoly) across multiple instances of cube position.
 *     Use d=8 cubes. Random 8-bit positions, repeat for K trials. */
static double probe_c2_cube_indicator(spiral_atcg_policy_t policy, uint64_t seed) {
    uint64_t st = seed;
    spiral_atcg_ctx_t ctx;
    make_session(&st, &ctx, policy, SPIRAL_ATCG_DEFAULT_ROUNDS);

    int CUBE_DIM = 8;
    int N_TRIALS = 32;
    int N_PER_TRIAL = 16;  /* sample cube sum at 16 different "free" assignments */

    /* For each cube position pattern, compute cube sum at multiple settings.
     * If cube sum is constant -> superpoly is degree 0 -> attack-friendly.
     * Metric: variance of cube sum across N_PER_TRIAL settings, averaged
     * over N_TRIALS cubes. Random expectation: variance high (uniform). */
    long total_unique = 0;
    int total = 0;

    for (int trial = 0; trial < N_TRIALS; trial++) {
        int cube_pos[8];
        for (int j = 0; j < CUBE_DIM; j++) cube_pos[j] = (int)(splitmix(&st) % 320);

        /* Track distinct cube-sum values */
        uint8_t cube_sums[16][40];
        for (int setting = 0; setting < N_PER_TRIAL; setting++) {
            uint8_t base[40];
            fill_random(base, 40, &st);
            uint8_t accum[40] = {0};
            for (long mask = 0; mask < (1L << CUBE_DIM); mask++) {
                uint8_t pt[40];
                memcpy(pt, base, 40);
                for (int b = 0; b < CUBE_DIM; b++) {
                    if ((mask >> b) & 1) {
                        pt[cube_pos[b] / 8] ^= (uint8_t)(1u << (cube_pos[b] % 8));
                    }
                }
                uint8_t ct[40];
                spiral_atcg_encrypt_block(&ctx, pt, ct);
                for (int j = 0; j < 40; j++) accum[j] ^= ct[j];
            }
            memcpy(cube_sums[setting], accum, 40);
        }
        /* Count unique cube sums */
        int unique = 0;
        for (int s = 0; s < N_PER_TRIAL; s++) {
            int dup = 0;
            for (int t = 0; t < s; t++) if (memcmp(cube_sums[s], cube_sums[t], 40) == 0) { dup = 1; break; }
            if (!dup) unique++;
        }
        total_unique += unique;
        total++;
    }
    /* High unique = high degree superpoly = healthy. */
    return (double)total_unique / (double)total;
}

/* C3. Higher-order differential at 4th and 8th order, comparing against
 *     random expectation. */
static void probe_c3_higher_order(spiral_atcg_policy_t policy, uint64_t seed,
                                    double *avg_hd_4th, double *avg_hd_8th) {
    uint64_t st = seed;
    spiral_atcg_ctx_t ctx;
    make_session(&st, &ctx, policy, SPIRAL_ATCG_DEFAULT_ROUNDS);

    long sum4 = 0, sum8 = 0;
    int n = 32;
    for (int trial = 0; trial < n; trial++) {
        for (int order = 4; order <= 8; order += 4) {
            uint8_t base[40];
            fill_random(base, 40, &st);
            int pos[8];
            for (int j = 0; j < order; j++) pos[j] = (int)(splitmix(&st) % 320);
            uint8_t accum[40] = {0};
            for (long mask = 0; mask < (1L << order); mask++) {
                uint8_t pt[40];
                memcpy(pt, base, 40);
                for (int b = 0; b < order; b++) {
                    if ((mask >> b) & 1) pt[pos[b]/8] ^= (uint8_t)(1u << (pos[b]%8));
                }
                uint8_t ct[40];
                spiral_atcg_encrypt_block(&ctx, pt, ct);
                for (int j = 0; j < 40; j++) accum[j] ^= ct[j];
            }
            unsigned hd = 0;
            for (int j = 0; j < 40; j++) hd += popcnt8(accum[j]);
            if (order == 4) sum4 += hd;
            else            sum8 += hd;
        }
    }
    *avg_hd_4th = (double)sum4 / (double)n;
    *avg_hd_8th = (double)sum8 / (double)n;
}

/* C4. Linear approximation table sample:
 *     Sample 64 (input mask, output mask) pairs, count <P,a> = <C,b>.
 *     Worst-case correlation = 2*p - 1 where p = matches/N.
 *     Healthy: |corr| < 1/sqrt(N). */
static double probe_c4_lat_worst(spiral_atcg_policy_t policy, uint64_t seed) {
    uint64_t st = seed;
    spiral_atcg_ctx_t ctx;
    make_session(&st, &ctx, policy, SPIRAL_ATCG_DEFAULT_ROUNDS);

    double worst = 0.0;
    /* Use random masks of low-medium Hamming weight */
    int N_SAMPLES = 4096;
    int N_PAIRS = 64;
    for (int p = 0; p < N_PAIRS; p++) {
        uint8_t alpha[40], beta[40];
        fill_random(alpha, 40, &st);
        fill_random(beta, 40, &st);
        /* Reduce to 1-3 bit masks for stronger LAT signal */
        for (int b = 0; b < 40; b++) { alpha[b] &= (uint8_t)(splitmix(&st) & 0xFF); beta[b] &= (uint8_t)(splitmix(&st) & 0xFF); }

        int matches = 0;
        for (int i = 0; i < N_SAMPLES; i++) {
            uint8_t pt[40], ct[40];
            fill_random(pt, 40, &st);
            spiral_atcg_encrypt_block(&ctx, pt, ct);
            unsigned pa = 0, cb = 0;
            for (int b = 0; b < 40; b++) { pa ^= popcnt8(pt[b] & alpha[b]); cb ^= popcnt8(ct[b] & beta[b]); }
            if ((pa & 1) == (cb & 1)) matches++;
        }
        double corr = fabs(2.0 * (double)matches / (double)N_SAMPLES - 1.0);
        if (corr > worst) worst = corr;
    }
    return worst;
}

/* =================================================================
 * Main
 * ================================================================= */
int main(void) {
    fprintf(stderr, "==========================================================\n");
    fprintf(stderr, "Spiral-ATCG v%s advanced cryptanalytic battery\n", SPIRAL_ATCG_VERSION_STRING);
    fprintf(stderr, "  Family A: quantum walk indicators\n");
    fprintf(stderr, "  Family B: boomerang distinguisher\n");
    fprintf(stderr, "  Family C: algebraic attacks\n");
    fprintf(stderr, "==========================================================\n\n");

    fprintf(stderr, "PROBE                              BASE         SAFE         AGGR         NULL\n");
    fprintf(stderr, "----------------------------------------------------------------------------\n");

    printf("policy,family,probe,metric,value\n");

    /* === Family A: quantum walk === */

    fprintf(stderr, "[A1] 32-bit prefix collisions  ");
    for (int p = 0; p < 4; p++) {
        long c = probe_a1_bijectivity(POLICIES[p].policy, 0xA1A1A1A1ULL + p);
        fprintf(stderr, "  %10ld", c);
        printf("Spiral-ATCG/%s,quantum_walk,bijectivity,collisions,%ld\n", POLICIES[p].name, c);
    }
    fprintf(stderr, "\n   expected (random N=4096):     ~2 (= N²/2³³)\n");

    fprintf(stderr, "[A2] Kuwakado-Morii @ r=2      ");
    for (int p = 0; p < 4; p++) {
        double c = probe_a2_kuwakado_morii(POLICIES[p].policy, 0xA2A2A2A2ULL + p, 2);
        fprintf(stderr, "  %10.5f", c);
        printf("Spiral-ATCG/%s,quantum_walk,kuwakado_morii_r2,worst_consistency,%.6f\n", POLICIES[p].name, c);
    }
    fprintf(stderr, "\n[A2] Kuwakado-Morii @ r=4      ");
    for (int p = 0; p < 4; p++) {
        double c = probe_a2_kuwakado_morii(POLICIES[p].policy, 0xA2A2A2A2ULL + p, 4);
        fprintf(stderr, "  %10.5f", c);
        printf("Spiral-ATCG/%s,quantum_walk,kuwakado_morii_r4,worst_consistency,%.6f\n", POLICIES[p].name, c);
    }
    fprintf(stderr, "\n[A2] Kuwakado-Morii @ r=16     ");
    for (int p = 0; p < 4; p++) {
        double c = probe_a2_kuwakado_morii(POLICIES[p].policy, 0xA2A2A2A2ULL + p, 16);
        fprintf(stderr, "  %10.5f", c);
        printf("Spiral-ATCG/%s,quantum_walk,kuwakado_morii_r16,worst_consistency,%.6f\n", POLICIES[p].name, c);
    }
    fprintf(stderr, "\n   random expectation (N=1024):  ~0.0104 (extreme value of binomial)\n\n");

    fprintf(stderr, "[A3] inner-state recurrence");
    fprintf(stderr, "\n     min HD to original         ");
    for (int p = 0; p < 4; p++) {
        unsigned mh; unsigned st;
        probe_a3_recurrence(POLICIES[p].policy, 0xA3A3A3A3ULL + p, &mh, &st);
        fprintf(stderr, "  %10u", mh);
        printf("Spiral-ATCG/%s,quantum_walk,recurrence,min_self_hd,%u\n", POLICIES[p].name, mh);
        printf("Spiral-ATCG/%s,quantum_walk,recurrence,steps_to_min,%u\n", POLICIES[p].name, st);
    }
    fprintf(stderr, "\n     steps to min HD            ");
    for (int p = 0; p < 4; p++) {
        unsigned mh; unsigned st;
        probe_a3_recurrence(POLICIES[p].policy, 0xA3A3A3A3ULL + p, &mh, &st);
        fprintf(stderr, "  %10u", st);
    }
    fprintf(stderr, "\n   random expectation (256 steps):  ~135 ± 9 (HD); steps uniform\n\n");

    /* === Family B: boomerang === */

    fprintf(stderr, "[B1] standard boomerang (16r)");
    fprintf(stderr, "\n     exact returns of alpha     ");
    for (int p = 0; p < 4; p++) {
        long e; double h;
        probe_b1_boomerang(POLICIES[p].policy, 0xB1B1B1B1ULL + p, &e, &h);
        fprintf(stderr, "  %10ld", e);
        printf("Spiral-ATCG/%s,boomerang,standard_r16,exact_returns,%ld\n", POLICIES[p].name, e);
        printf("Spiral-ATCG/%s,boomerang,standard_r16,avg_hd_to_alpha,%.4f\n", POLICIES[p].name, h);
    }
    fprintf(stderr, "\n     avg HD(P3^P4, alpha)       ");
    for (int p = 0; p < 4; p++) {
        long e; double h;
        probe_b1_boomerang(POLICIES[p].policy, 0xB1B1B1B1ULL + p, &e, &h);
        fprintf(stderr, "  %10.3f", h);
    }
    fprintf(stderr, "\n   expected (random): exact=0; avg HD ≈ 160 (P3^P4 random vs alpha=1-bit)\n\n");

    fprintf(stderr, "[B2] sandwich boomerang @ r=4 ");
    fprintf(stderr, "\n     exact returns of alpha     ");
    for (int p = 0; p < 4; p++) {
        long e; double h;
        probe_b2_sandwich(POLICIES[p].policy, 0xB2B2B2B2ULL + p, 4, &e, &h);
        fprintf(stderr, "  %10ld", e);
        printf("Spiral-ATCG/%s,boomerang,sandwich_r4,exact_returns,%ld\n", POLICIES[p].name, e);
        printf("Spiral-ATCG/%s,boomerang,sandwich_r4,avg_hd_to_alpha,%.4f\n", POLICIES[p].name, h);
    }
    fprintf(stderr, "\n     avg HD(P3^P4, alpha)       ");
    for (int p = 0; p < 4; p++) {
        long e; double h;
        probe_b2_sandwich(POLICIES[p].policy, 0xB2B2B2B2ULL + p, 4, &e, &h);
        fprintf(stderr, "  %10.3f", h);
    }
    fprintf(stderr, "\n[B2] sandwich boomerang @ r=8 ");
    fprintf(stderr, "\n     exact returns of alpha     ");
    for (int p = 0; p < 4; p++) {
        long e; double h;
        probe_b2_sandwich(POLICIES[p].policy, 0xB2B2B2B2ULL + p, 8, &e, &h);
        fprintf(stderr, "  %10ld", e);
        printf("Spiral-ATCG/%s,boomerang,sandwich_r8,exact_returns,%ld\n", POLICIES[p].name, e);
        printf("Spiral-ATCG/%s,boomerang,sandwich_r8,avg_hd_to_alpha,%.4f\n", POLICIES[p].name, h);
    }
    fprintf(stderr, "\n     avg HD(P3^P4, alpha)       ");
    for (int p = 0; p < 4; p++) {
        long e; double h;
        probe_b2_sandwich(POLICIES[p].policy, 0xB2B2B2B2ULL + p, 8, &e, &h);
        fprintf(stderr, "  %10.3f", h);
    }
    fprintf(stderr, "\n\n");

    fprintf(stderr, "[B3] related-key boomerang (16r)");
    fprintf(stderr, "\n     exact returns              ");
    for (int p = 0; p < 4; p++) {
        long e; double h;
        probe_b3_rk_boomerang(POLICIES[p].policy, 0xB3B3B3B3ULL + p, &e, &h);
        fprintf(stderr, "  %10ld", e);
        printf("Spiral-ATCG/%s,boomerang,rk_r16,exact_returns,%ld\n", POLICIES[p].name, e);
        printf("Spiral-ATCG/%s,boomerang,rk_r16,avg_hd_to_alpha,%.4f\n", POLICIES[p].name, h);
    }
    fprintf(stderr, "\n     avg HD                     ");
    for (int p = 0; p < 4; p++) {
        long e; double h;
        probe_b3_rk_boomerang(POLICIES[p].policy, 0xB3B3B3B3ULL + p, &e, &h);
        fprintf(stderr, "  %10.3f", h);
    }
    fprintf(stderr, "\n\n");

    /* === Family C: algebraic === */

    fprintf(stderr, "[C1] algebraic degree probe (1=non-zero result, healthy)\n");
    int orders[5] = {1, 2, 4, 8, 16};
    int deg_results[4][5];
    for (int p = 0; p < 4; p++) {
        probe_c1_alg_degree(POLICIES[p].policy, 0xC1C1C1C1ULL + p, deg_results[p]);
    }
    for (int oi = 0; oi < 5; oi++) {
        fprintf(stderr, "     %dth-order derivative      ", orders[oi]);
        for (int p = 0; p < 4; p++) {
            fprintf(stderr, "  %10d", deg_results[p][oi]);
            printf("Spiral-ATCG/%s,algebraic,deg_order_%d,nonzero,%d\n", POLICIES[p].name, orders[oi], deg_results[p][oi]);
        }
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "   expected (random): all orders nonzero (degree saturated)\n\n");

    fprintf(stderr, "[C2] cube indicator (avg unique sums per 32 cubes)\n");
    fprintf(stderr, "     unique cube sums (max=16)  ");
    for (int p = 0; p < 4; p++) {
        double u = probe_c2_cube_indicator(POLICIES[p].policy, 0xC2C2C2C2ULL + p);
        fprintf(stderr, "  %10.3f", u);
        printf("Spiral-ATCG/%s,algebraic,cube_indicator,avg_unique,%.4f\n", POLICIES[p].name, u);
    }
    fprintf(stderr, "\n   expected (random): close to 16 (all sums distinct)\n\n");

    fprintf(stderr, "[C3] higher-order differential (avg HD over 32 trials)\n");
    fprintf(stderr, "     4th-order avg HD           ");
    for (int p = 0; p < 4; p++) {
        double h4, h8;
        probe_c3_higher_order(POLICIES[p].policy, 0xC3C3C3C3ULL + p, &h4, &h8);
        fprintf(stderr, "  %10.3f", h4);
        printf("Spiral-ATCG/%s,algebraic,higher_diff_4th,avg_hd,%.4f\n", POLICIES[p].name, h4);
        printf("Spiral-ATCG/%s,algebraic,higher_diff_8th,avg_hd,%.4f\n", POLICIES[p].name, h8);
    }
    fprintf(stderr, "\n     8th-order avg HD           ");
    for (int p = 0; p < 4; p++) {
        double h4, h8;
        probe_c3_higher_order(POLICIES[p].policy, 0xC3C3C3C3ULL + p, &h4, &h8);
        fprintf(stderr, "  %10.3f", h8);
    }
    fprintf(stderr, "\n   expected (random): ~160; values << 160 indicate low-degree structure\n\n");

    fprintf(stderr, "[C4] LAT worst correlation     ");
    for (int p = 0; p < 4; p++) {
        double w = probe_c4_lat_worst(POLICIES[p].policy, 0xC4C4C4C4ULL + p);
        fprintf(stderr, "  %10.5f", w);
        printf("Spiral-ATCG/%s,algebraic,lat_worst_corr,value,%.6f\n", POLICIES[p].name, w);
    }
    fprintf(stderr, "\n   expected (random N=4096):  worst |corr| ≈ 0.04 (= sqrt(2 ln 64)/sqrt(4096))\n");

    fprintf(stderr, "\n==========================================================\n");
    fprintf(stderr, "Advanced battery complete. Machine CSV on stdout.\n");
    fprintf(stderr, "==========================================================\n");

    return 0;
}
