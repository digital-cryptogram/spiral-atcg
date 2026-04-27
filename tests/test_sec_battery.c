/* Spiral-ATCG v2.2.4 extended security battery.
 *
 * Goes beyond bundled tests with:
 *   1. Differential trail probability bound (input difference -> output Hamming distribution)
 *   2. Linear correlation probe (input/output mask correlation)
 *   3. Key-bit avalanche (master_seed bit flip -> ciphertext distance)
 *   4. Bit balance (ciphertext output bit ones-density across N samples)
 *   5. Related-key differential (key delta + plaintext delta)
 *   6. Slide attack indicator (search for E_K(P) = E_K(P') under shifted P)
 *   7. Fixed-point search (E_K(P) = P for some K, P)
 *   8. Reduced-round bias (avalanche at rounds 1..16)
 *
 * Output: human-readable summary + machine-readable CSV. */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "spiral_atcg.h"

#define SAMPLES 8192
#define KEY_AVAL_TUPLES 8

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

static unsigned popcnt8(uint8_t x) {
    unsigned c = 0;
    while (x) { c += x & 1u; x >>= 1; }
    return c;
}
static unsigned hd320(const uint8_t *a, const uint8_t *b) {
    unsigned h = 0;
    for (int i = 0; i < 40; i++) h += popcnt8(a[i] ^ b[i]);
    return h;
}

/* Deterministic SplitMix-64 */
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
    for (size_t i = (n / 8) * 8; i < n; i++) {
        p[i] = (uint8_t)(splitmix(st) & 0xFF);
    }
}

static void make_session(uint64_t *st, uint8_t master[32], uint8_t nonce[16],
                         uint8_t sid[16], uint8_t pt[40]) {
    fill_random(master, 32, st);
    fill_random(nonce,  16, st);
    fill_random(sid,    16, st);
    fill_random(pt,     40, st);
}

/* ============================================
 * Probe 1: Differential trail probability bound
 * ============================================
 * For a given input difference delta, encrypt N pairs (P, P xor delta)
 * and report (a) avg HD of output difference, (b) min HD, (c) max HD,
 * (d) variance, (e) max ciphertext-difference frequency (= probability
 * upper bound for any single output diff). */
static void probe_differential_trail(spiral_atcg_policy_t policy, uint64_t seed,
                                      double *avg, unsigned *minhd, unsigned *maxhd,
                                      double *max_freq) {
    uint64_t st = seed;
    uint8_t master[32], nonce[16], sid[16];
    fill_random(master, 32, &st);
    fill_random(nonce,  16, &st);
    fill_random(sid,    16, &st);

    spiral_atcg_ctx_t ctx;
    spiral_atcg_init_session(&ctx, master, nonce, sid, SPIRAL_ATCG_DEFAULT_ROUNDS, policy);

    /* Use a structured input difference: bit 0 of byte 0 */
    long sum_hd = 0;
    *minhd = 320;
    *maxhd = 0;

    /* Track output-difference frequency: hash to 16-bit bucket */
    uint32_t freq[65536];
    memset(freq, 0, sizeof(freq));

    for (int i = 0; i < SAMPLES; i++) {
        uint8_t pt[40], pt2[40], ct[40], ct2[40], diff[40];
        fill_random(pt, 40, &st);
        memcpy(pt2, pt, 40);
        pt2[0] ^= 0x01;  /* fixed input difference */
        spiral_atcg_encrypt_block(&ctx, pt, ct);
        spiral_atcg_encrypt_block(&ctx, pt2, ct2);
        for (int b = 0; b < 40; b++) diff[b] = ct[b] ^ ct2[b];
        unsigned h = hd320(ct, ct2);
        sum_hd += h;
        if (h < *minhd) *minhd = h;
        if (h > *maxhd) *maxhd = h;
        /* Bucket by first 16 output-diff bits */
        uint16_t bucket = (uint16_t)(diff[0] | (diff[1] << 8));
        freq[bucket]++;
    }
    *avg = (double)sum_hd / (double)SAMPLES;

    /* Max frequency */
    uint32_t max_count = 0;
    for (int b = 0; b < 65536; b++) if (freq[b] > max_count) max_count = freq[b];
    *max_freq = (double)max_count / (double)SAMPLES;
}

/* ============================================
 * Probe 2: Linear correlation
 * ============================================
 * For a fixed input mask (single bit) and a fixed output mask (single bit),
 * measure the bias |Pr[<P, alpha> = <C, beta>] - 1/2|.
 * Report worst-case |bias| over a sweep of (alpha_bit, beta_bit) pairs. */
static void probe_linear(spiral_atcg_policy_t policy, uint64_t seed,
                          double *worst_bias, double *avg_abs_bias) {
    uint64_t st = seed;
    uint8_t master[32], nonce[16], sid[16];
    fill_random(master, 32, &st);
    fill_random(nonce,  16, &st);
    fill_random(sid,    16, &st);

    spiral_atcg_ctx_t ctx;
    spiral_atcg_init_session(&ctx, master, nonce, sid, SPIRAL_ATCG_DEFAULT_ROUNDS, policy);

    /* Sweep 16 (alpha_bit, beta_bit) pairs, sample SAMPLES plaintexts each */
    *worst_bias = 0.0;
    double sum_abs = 0.0;
    int pair_count = 0;

    int alpha_bits[8] = {0, 8, 16, 64, 128, 200, 256, 319};
    int beta_bits[8]  = {0, 32, 64, 128, 192, 224, 280, 319};

    for (int ai = 0; ai < 8; ai++) {
        for (int bi = 0; bi < 8; bi++) {
            int a = alpha_bits[ai];
            int b = beta_bits[bi];
            int matches = 0;
            uint64_t local_st = st;
            for (int i = 0; i < SAMPLES; i++) {
                uint8_t pt[40], ct[40];
                fill_random(pt, 40, &local_st);
                spiral_atcg_encrypt_block(&ctx, pt, ct);
                int p_bit = (pt[a / 8] >> (a % 8)) & 1;
                int c_bit = (ct[b / 8] >> (b % 8)) & 1;
                if (p_bit == c_bit) matches++;
            }
            double bias = fabs((double)matches / (double)SAMPLES - 0.5);
            sum_abs += bias;
            pair_count++;
            if (bias > *worst_bias) *worst_bias = bias;
        }
    }
    *avg_abs_bias = sum_abs / (double)pair_count;
}

/* ============================================
 * Probe 3: Key-bit avalanche
 * ============================================
 * For a fixed plaintext, flip each bit of master_seed and measure
 * the output Hamming distance. Average over multiple sessions. */
static void probe_key_avalanche(spiral_atcg_policy_t policy,
                                 double *avg_hd, unsigned *minhd, unsigned *maxhd) {
    long sum_hd = 0;
    long total = 0;
    *minhd = 320;
    *maxhd = 0;

    for (int t = 0; t < KEY_AVAL_TUPLES; t++) {
        uint64_t st = 0xCAFEBABE00000000ULL ^ ((uint64_t)t * 0x9E3779B97F4A7C15ULL);
        uint8_t master[32], master2[32], nonce[16], sid[16], pt[40];
        fill_random(master, 32, &st);
        fill_random(nonce,  16, &st);
        fill_random(sid,    16, &st);
        fill_random(pt,     40, &st);

        spiral_atcg_ctx_t ctx, ctx2;
        spiral_atcg_init_session(&ctx, master, nonce, sid, SPIRAL_ATCG_DEFAULT_ROUNDS, policy);
        uint8_t ct[40];
        spiral_atcg_encrypt_block(&ctx, pt, ct);

        for (int b = 0; b < 256; b++) {
            memcpy(master2, master, 32);
            master2[b / 8] ^= (uint8_t)(1u << (b % 8));
            spiral_atcg_init_session(&ctx2, master2, nonce, sid, SPIRAL_ATCG_DEFAULT_ROUNDS, policy);
            uint8_t ct2[40];
            spiral_atcg_encrypt_block(&ctx2, pt, ct2);
            unsigned h = hd320(ct, ct2);
            sum_hd += h;
            total++;
            if (h < *minhd) *minhd = h;
            if (h > *maxhd) *maxhd = h;
        }
    }
    *avg_hd = (double)sum_hd / (double)total;
}

/* ============================================
 * Probe 4: Bit-balance / output uniformity
 * ============================================
 * For random inputs, count ones at each ciphertext bit position.
 * Report worst deviation from 0.5 across all 320 bit positions. */
static void probe_bit_balance(spiral_atcg_policy_t policy, uint64_t seed,
                               double *worst_dev, double *avg_dev) {
    uint64_t st = seed;
    uint8_t master[32], nonce[16], sid[16];
    fill_random(master, 32, &st);
    fill_random(nonce,  16, &st);
    fill_random(sid,    16, &st);

    spiral_atcg_ctx_t ctx;
    spiral_atcg_init_session(&ctx, master, nonce, sid, SPIRAL_ATCG_DEFAULT_ROUNDS, policy);

    int ones[320];
    memset(ones, 0, sizeof(ones));
    for (int i = 0; i < SAMPLES; i++) {
        uint8_t pt[40], ct[40];
        fill_random(pt, 40, &st);
        spiral_atcg_encrypt_block(&ctx, pt, ct);
        for (int b = 0; b < 320; b++) {
            ones[b] += (ct[b / 8] >> (b % 8)) & 1;
        }
    }
    *worst_dev = 0.0;
    double sum_dev = 0.0;
    for (int b = 0; b < 320; b++) {
        double dev = fabs((double)ones[b] / (double)SAMPLES - 0.5);
        sum_dev += dev;
        if (dev > *worst_dev) *worst_dev = dev;
    }
    *avg_dev = sum_dev / 320.0;
}

/* ============================================
 * Probe 5: Related-key differential
 * ============================================
 * Run a key-difference + plaintext-difference probe.
 * Sample N (P, K, deltaP, deltaK), encrypt (P,K) and (P xor deltaP, K xor deltaK),
 * report avg HD. A related-key vulnerability would show HD strongly biased away from 160. */
static void probe_related_key(spiral_atcg_policy_t policy, uint64_t seed, double *avg_hd) {
    uint64_t st = seed;
    long sum = 0;
    int n = 1024;  /* fewer samples since each requires 2 init_session */

    for (int i = 0; i < n; i++) {
        uint8_t master[32], master2[32], nonce[16], sid[16], pt[40], pt2[40];
        fill_random(master, 32, &st);
        fill_random(nonce,  16, &st);
        fill_random(sid,    16, &st);
        fill_random(pt,     40, &st);
        memcpy(master2, master, 32);
        memcpy(pt2, pt, 40);
        /* Random delta pattern */
        uint8_t dk = (uint8_t)splitmix(&st);
        uint8_t dp = (uint8_t)splitmix(&st);
        master2[0] ^= dk;
        pt2[0] ^= dp;
        spiral_atcg_ctx_t ctx, ctx2;
        spiral_atcg_init_session(&ctx,  master,  nonce, sid, SPIRAL_ATCG_DEFAULT_ROUNDS, policy);
        spiral_atcg_init_session(&ctx2, master2, nonce, sid, SPIRAL_ATCG_DEFAULT_ROUNDS, policy);
        uint8_t ct[40], ct2[40];
        spiral_atcg_encrypt_block(&ctx,  pt,  ct);
        spiral_atcg_encrypt_block(&ctx2, pt2, ct2);
        sum += hd320(ct, ct2);
    }
    *avg_hd = (double)sum / (double)n;
}

/* ============================================
 * Probe 6: Slide-attack indicator (degenerate-key search)
 * ============================================
 * For random K, P, search for P' such that E_K(P) = E_K(P').
 * Birthday bound predicts ~2^160 trials. We do far fewer; finding ANY
 * collision in our budget would be a red flag. */
static void probe_slide_collision(spiral_atcg_policy_t policy, uint64_t seed,
                                   long *trials, long *collisions) {
    uint64_t st = seed;
    uint8_t master[32], nonce[16], sid[16], pt[40], ct[40], pt2[40], ct2[40];
    fill_random(master, 32, &st);
    fill_random(nonce,  16, &st);
    fill_random(sid,    16, &st);
    fill_random(pt, 40, &st);

    spiral_atcg_ctx_t ctx;
    spiral_atcg_init_session(&ctx, master, nonce, sid, SPIRAL_ATCG_DEFAULT_ROUNDS, policy);
    spiral_atcg_encrypt_block(&ctx, pt, ct);

    *trials = SAMPLES;
    *collisions = 0;
    for (int i = 0; i < SAMPLES; i++) {
        fill_random(pt2, 40, &st);
        if (memcmp(pt2, pt, 40) == 0) continue;  /* skip identical input */
        spiral_atcg_encrypt_block(&ctx, pt2, ct2);
        if (memcmp(ct, ct2, 40) == 0) (*collisions)++;
    }
}

/* ============================================
 * Probe 7: Fixed-point search
 * ============================================
 * For random K, search for any P with E_K(P) = P. Should be ~2^-320 per random sample. */
static void probe_fixed_point(spiral_atcg_policy_t policy, uint64_t seed, long *fixed_points) {
    uint64_t st = seed;
    uint8_t master[32], nonce[16], sid[16], pt[40], ct[40];
    fill_random(master, 32, &st);
    fill_random(nonce,  16, &st);
    fill_random(sid,    16, &st);

    spiral_atcg_ctx_t ctx;
    spiral_atcg_init_session(&ctx, master, nonce, sid, SPIRAL_ATCG_DEFAULT_ROUNDS, policy);

    *fixed_points = 0;
    for (int i = 0; i < SAMPLES; i++) {
        fill_random(pt, 40, &st);
        spiral_atcg_encrypt_block(&ctx, pt, ct);
        if (memcmp(pt, ct, 40) == 0) (*fixed_points)++;
    }
}

/* ============================================
 * Probe 8: Reduced-round avalanche
 * ============================================
 * Test 1-bit pt avalanche at rounds = {1, 2, 3, 4, 5, 6, 8, 12, 16}.
 * Reports first round where avg HD reaches >= 158 (within 1% of ideal 160). */
static int probe_reduced_round_diffusion(spiral_atcg_policy_t policy, uint64_t seed,
                                           double hd_per_round[10]) {
    uint32_t rounds[9] = {1, 2, 3, 4, 5, 6, 8, 12, 16};
    int first_full = -1;
    for (int r_idx = 0; r_idx < 9; r_idx++) {
        uint64_t st = seed;
        uint32_t rounds_n = rounds[r_idx];
        long sum = 0;
        int N = 256;  /* 256 random tuples, single bit flip each */
        for (int i = 0; i < N; i++) {
            uint8_t master[32], nonce[16], sid[16], pt[40], pt2[40], ct[40], ct2[40];
            fill_random(master, 32, &st);
            fill_random(nonce,  16, &st);
            fill_random(sid,    16, &st);
            fill_random(pt,     40, &st);
            memcpy(pt2, pt, 40);
            pt2[0] ^= 0x01;
            spiral_atcg_ctx_t ctx;
            spiral_atcg_init_session(&ctx, master, nonce, sid, rounds_n, policy);
            spiral_atcg_encrypt_block(&ctx, pt, ct);
            spiral_atcg_encrypt_block(&ctx, pt2, ct2);
            sum += hd320(ct, ct2);
        }
        hd_per_round[r_idx] = (double)sum / (double)N;
        if (first_full < 0 && hd_per_round[r_idx] >= 158.0) first_full = (int)rounds_n;
    }
    return first_full;
}

int main(void) {
    printf("==========================================================\n");
    printf("Spiral-ATCG v%s extended security battery\n", SPIRAL_ATCG_VERSION_STRING);
    printf("==========================================================\n\n");

    /* Per-policy summary table goes to stderr (so stdout is clean CSV) */
    fprintf(stderr, "PROBE                       BASE        SAFE        AGGR        NULL\n");
    fprintf(stderr, "----------------------------------------------------------------------\n");

    printf("policy,probe,metric,value\n");

    /* Probe 1: Differential */
    printf("# === Probe 1: Differential (single-bit pt diff x %d samples) ===\n", SAMPLES);
    fprintf(stderr, "%-26s", "[1] avg out diff HD");
    for (int p = 0; p < 4; p++) {
        double avg, max_freq;
        unsigned minhd, maxhd;
        probe_differential_trail(POLICIES[p].policy, 0xA1A1A1A100000000ULL, &avg, &minhd, &maxhd, &max_freq);
        fprintf(stderr, "  %8.3f", avg);
        printf("Spiral-ATCG/%s,differential,avg_hd,%.4f\n", POLICIES[p].name, avg);
        printf("Spiral-ATCG/%s,differential,min_hd,%u\n", POLICIES[p].name, minhd);
        printf("Spiral-ATCG/%s,differential,max_hd,%u\n", POLICIES[p].name, maxhd);
        printf("Spiral-ATCG/%s,differential,max_bucket_freq,%.6f\n", POLICIES[p].name, max_freq);
    }
    fprintf(stderr, "\n%-26s", "    range [min,max]");
    for (int p = 0; p < 4; p++) {
        double avg, max_freq; unsigned mi, mx;
        probe_differential_trail(POLICIES[p].policy, 0xA1A1A1A100000000ULL, &avg, &mi, &mx, &max_freq);
        fprintf(stderr, "  [%3u,%3u]", mi, mx);
    }
    fprintf(stderr, "\n%-26s", "    max bucket freq");
    for (int p = 0; p < 4; p++) {
        double avg, mf; unsigned mi, mx;
        probe_differential_trail(POLICIES[p].policy, 0xA1A1A1A100000000ULL, &avg, &mi, &mx, &mf);
        fprintf(stderr, "  %.5f", mf);
    }
    fprintf(stderr, "\n\n");

    /* Probe 2: Linear */
    printf("# === Probe 2: Linear (64 mask pairs x %d samples) ===\n", SAMPLES);
    fprintf(stderr, "%-26s", "[2] worst |bias|");
    for (int p = 0; p < 4; p++) {
        double wb, ab;
        probe_linear(POLICIES[p].policy, 0xB2B2B2B200000000ULL, &wb, &ab);
        fprintf(stderr, "  %8.5f", wb);
        printf("Spiral-ATCG/%s,linear,worst_abs_bias,%.6f\n", POLICIES[p].name, wb);
        printf("Spiral-ATCG/%s,linear,avg_abs_bias,%.6f\n", POLICIES[p].name, ab);
    }
    fprintf(stderr, "\n%-26s", "    avg |bias|");
    for (int p = 0; p < 4; p++) {
        double wb, ab;
        probe_linear(POLICIES[p].policy, 0xB2B2B2B200000000ULL, &wb, &ab);
        fprintf(stderr, "  %8.5f", ab);
    }
    fprintf(stderr, "\n\n");

    /* Probe 3: Key-bit avalanche */
    printf("# === Probe 3: Key-bit avalanche (256 bits x 8 tuples = 2048 cells) ===\n");
    fprintf(stderr, "%-26s", "[3] avg key-flip HD");
    for (int p = 0; p < 4; p++) {
        double avg; unsigned mi, mx;
        probe_key_avalanche(POLICIES[p].policy, &avg, &mi, &mx);
        fprintf(stderr, "  %8.3f", avg);
        printf("Spiral-ATCG/%s,key_avalanche,avg_hd,%.4f\n", POLICIES[p].name, avg);
        printf("Spiral-ATCG/%s,key_avalanche,min_hd,%u\n", POLICIES[p].name, mi);
        printf("Spiral-ATCG/%s,key_avalanche,max_hd,%u\n", POLICIES[p].name, mx);
    }
    fprintf(stderr, "\n%-26s", "    range [min,max]");
    for (int p = 0; p < 4; p++) {
        double avg; unsigned mi, mx;
        probe_key_avalanche(POLICIES[p].policy, &avg, &mi, &mx);
        fprintf(stderr, "  [%3u,%3u]", mi, mx);
    }
    fprintf(stderr, "\n\n");

    /* Probe 4: Bit balance */
    printf("# === Probe 4: Bit balance (320 output bits x %d samples) ===\n", SAMPLES);
    fprintf(stderr, "%-26s", "[4] worst |dev| from .5");
    for (int p = 0; p < 4; p++) {
        double wd, ad;
        probe_bit_balance(POLICIES[p].policy, 0xD4D4D4D400000000ULL, &wd, &ad);
        fprintf(stderr, "  %8.5f", wd);
        printf("Spiral-ATCG/%s,bit_balance,worst_dev,%.6f\n", POLICIES[p].name, wd);
        printf("Spiral-ATCG/%s,bit_balance,avg_dev,%.6f\n", POLICIES[p].name, ad);
    }
    fprintf(stderr, "\n%-26s", "    avg |dev|");
    for (int p = 0; p < 4; p++) {
        double wd, ad;
        probe_bit_balance(POLICIES[p].policy, 0xD4D4D4D400000000ULL, &wd, &ad);
        fprintf(stderr, "  %8.5f", ad);
    }
    fprintf(stderr, "\n\n");

    /* Probe 5: Related-key differential */
    printf("# === Probe 5: Related-key differential (1024 random tuples) ===\n");
    fprintf(stderr, "%-26s", "[5] avg related-K HD");
    for (int p = 0; p < 4; p++) {
        double avg;
        probe_related_key(POLICIES[p].policy, 0xE5E5E5E500000000ULL, &avg);
        fprintf(stderr, "  %8.3f", avg);
        printf("Spiral-ATCG/%s,related_key,avg_hd,%.4f\n", POLICIES[p].name, avg);
    }
    fprintf(stderr, "\n\n");

    /* Probe 6: Slide collision */
    printf("# === Probe 6: Slide-attack collision search (%d trials per policy) ===\n", SAMPLES);
    fprintf(stderr, "%-26s", "[6] slide collisions");
    for (int p = 0; p < 4; p++) {
        long t, c;
        probe_slide_collision(POLICIES[p].policy, 0xF6F6F6F600000000ULL, &t, &c);
        fprintf(stderr, "  %8ld", c);
        printf("Spiral-ATCG/%s,slide_collision,trials,%ld\n", POLICIES[p].name, t);
        printf("Spiral-ATCG/%s,slide_collision,collisions,%ld\n", POLICIES[p].name, c);
    }
    fprintf(stderr, "\n\n");

    /* Probe 7: Fixed point */
    printf("# === Probe 7: Fixed-point search (%d trials per policy) ===\n", SAMPLES);
    fprintf(stderr, "%-26s", "[7] fixed points");
    for (int p = 0; p < 4; p++) {
        long fp;
        probe_fixed_point(POLICIES[p].policy, 0xA7A7A7A700000000ULL, &fp);
        fprintf(stderr, "  %8ld", fp);
        printf("Spiral-ATCG/%s,fixed_point,trials,%d\n", POLICIES[p].name, SAMPLES);
        printf("Spiral-ATCG/%s,fixed_point,fixed_points,%ld\n", POLICIES[p].name, fp);
    }
    fprintf(stderr, "\n\n");

    /* Probe 8: Reduced-round diffusion */
    printf("# === Probe 8: Reduced-round diffusion (1-bit pt avalanche, 256 tuples) ===\n");
    fprintf(stderr, "%-26s", "[8] HD by round (r=1..16)");
    fprintf(stderr, "\n");
    int first_full[4];
    double hd_table[4][10];
    for (int p = 0; p < 4; p++) {
        first_full[p] = probe_reduced_round_diffusion(POLICIES[p].policy, 0xC8C8C8C800000000ULL, hd_table[p]);
    }
    int rounds[9] = {1, 2, 3, 4, 5, 6, 8, 12, 16};
    for (int r = 0; r < 9; r++) {
        fprintf(stderr, "    r=%-2d                ", rounds[r]);
        for (int p = 0; p < 4; p++) {
            fprintf(stderr, "  %8.2f", hd_table[p][r]);
            printf("Spiral-ATCG/%s,reduced_round,r%d_avg_hd,%.4f\n", POLICIES[p].name, rounds[r], hd_table[p][r]);
        }
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n%-26s", "    first round HD>=158");
    for (int p = 0; p < 4; p++) {
        if (first_full[p] > 0) fprintf(stderr, "  %8d", first_full[p]);
        else fprintf(stderr, "  %8s", ">16");
        printf("Spiral-ATCG/%s,reduced_round,first_full_round,%d\n", POLICIES[p].name, first_full[p]);
    }
    fprintf(stderr, "\n");

    fprintf(stderr, "\n==========================================================\n");
    fprintf(stderr, "Battery complete. Machine-readable CSV on stdout.\n");
    fprintf(stderr, "==========================================================\n");
    return 0;
}
