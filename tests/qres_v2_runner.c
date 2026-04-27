/* QRES probe runner for Spiral-ATCG v2.2-strength.
 *
 * Probes: input (320 plaintext bits), seed (256 master bits), nonce (128 nonce bits).
 * Per (policy, probe, round): runs N tuples × probed bit positions, reports
 * mean and std-dev of avg HD plus min HD, max HD, zero-diff count, full-diffusion %.
 *
 * Output: CSV to stdout. */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "spiral_atcg.h"

#define ROUND_COUNT 6
#define SAMPLE_TUPLES 16

static const uint32_t ROUNDS_LIST[ROUND_COUNT] = {2, 3, 4, 6, 8, 12};

typedef struct {
    spiral_atcg_policy_t policy;
    const char *name;
} policy_def_t;

static const policy_def_t POLICIES[4] = {
    {SPIRAL_ATCG_POLICY_BASE, "BASE"},
    {SPIRAL_ATCG_POLICY_SAFE, "SAFE"},
    {SPIRAL_ATCG_POLICY_AGGR, "AGGR"},
    {SPIRAL_ATCG_POLICY_NULL, "NULL"},
};

static unsigned popcnt8(uint8_t x) {
    unsigned c = 0;
    while (x) { c += (unsigned)(x & 1u); x >>= 1; }
    return c;
}

static unsigned hd320(const uint8_t *a, const uint8_t *b) {
    unsigned h = 0;
    for (int i = 0; i < 40; i++) h += popcnt8((uint8_t)(a[i] ^ b[i]));
    return h;
}

static unsigned active_lanes_count(const uint8_t *a, const uint8_t *b) {
    unsigned act = 0;
    for (int lane = 0; lane < 10; lane++) {
        unsigned lh = 0;
        for (int b2 = 0; b2 < 4; b2++)
            lh += popcnt8((uint8_t)(a[lane*4+b2] ^ b[lane*4+b2]));
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

static void make_tuple(uint64_t seed, uint8_t master[32], uint8_t nonce[16],
                       uint8_t sid[16], uint8_t pt[40]) {
    uint64_t st = seed;
    for (int i = 0; i < 32; i += 8) { uint64_t v = splitmix(&st); memcpy(master + i, &v, 8); }
    for (int i = 0; i < 16; i += 8) { uint64_t v = splitmix(&st); memcpy(nonce  + i, &v, 8); }
    for (int i = 0; i < 16; i += 8) { uint64_t v = splitmix(&st); memcpy(sid    + i, &v, 8); }
    for (int i = 0; i < 40; i += 8) { uint64_t v = splitmix(&st); memcpy(pt     + i, &v, 8); }
}

/* Per-tuple per-(policy,rounds) probe statistics. */
typedef struct {
    double avg_hd_sum;       /* sum of per-tuple avg_hd */
    double avg_hd_sumsq;     /* for std-dev */
    double min_hd_sum;
    double max_hd_sum;
    long   zero_diff_count;
    long   full_diff_count;
    long   total_cells;      /* sum of probed-bit counts across tuples */
    int    n_tuples;
} probe_acc_t;

/* Run one (policy, rounds, probe-type) configuration: average across N tuples. */
static int run_input_probe(spiral_atcg_policy_t policy, uint32_t rounds,
                            probe_acc_t *acc) {
    memset(acc, 0, sizeof(*acc));
    for (int t = 0; t < SAMPLE_TUPLES; t++) {
        uint8_t master[32], nonce[16], sid[16], pt[40], pt_flip[40], ct[40], ct_flip[40];
        make_tuple(0xDEADBEEF12345678ULL ^ ((uint64_t)t * 0x9E3779B97F4A7C15ULL),
                   master, nonce, sid, pt);

        spiral_atcg_ctx_t ctx;
        if (spiral_atcg_init_session(&ctx, master, nonce, sid, rounds, policy) != SPIRAL_ATCG_OK)
            return 1;
        if (spiral_atcg_encrypt_block(&ctx, pt, ct) != SPIRAL_ATCG_OK)
            return 1;

        long tuple_hd_sum = 0;
        unsigned tuple_min = UINT32_MAX;
        unsigned tuple_max = 0;
        long tuple_zero = 0;
        long tuple_full = 0;

        for (int bit = 0; bit < 320; bit++) {
            memcpy(pt_flip, pt, 40);
            pt_flip[bit / 8] ^= (uint8_t)(1u << (bit % 8));
            if (spiral_atcg_encrypt_block(&ctx, pt_flip, ct_flip) != SPIRAL_ATCG_OK)
                return 1;
            unsigned hd = hd320(ct, ct_flip);
            unsigned al = active_lanes_count(ct, ct_flip);
            tuple_hd_sum += hd;
            if (hd < tuple_min) tuple_min = hd;
            if (hd > tuple_max) tuple_max = hd;
            if (hd == 0) tuple_zero++;
            if (al == 10) tuple_full++;
        }
        double avg = (double)tuple_hd_sum / 320.0;
        acc->avg_hd_sum   += avg;
        acc->avg_hd_sumsq += avg * avg;
        acc->min_hd_sum   += (double)tuple_min;
        acc->max_hd_sum   += (double)tuple_max;
        acc->zero_diff_count += tuple_zero;
        acc->full_diff_count += tuple_full;
        acc->total_cells     += 320;
        acc->n_tuples++;
    }
    return 0;
}

static int run_seed_probe(spiral_atcg_policy_t policy, uint32_t rounds,
                           probe_acc_t *acc) {
    memset(acc, 0, sizeof(*acc));
    for (int t = 0; t < SAMPLE_TUPLES; t++) {
        uint8_t master[32], master_flip[32], nonce[16], sid[16], pt[40], ct[40], ct_flip[40];
        make_tuple(0xDEADBEEF12345678ULL ^ ((uint64_t)t * 0x9E3779B97F4A7C15ULL),
                   master, nonce, sid, pt);

        spiral_atcg_ctx_t ctx, ctx_flip;
        if (spiral_atcg_init_session(&ctx, master, nonce, sid, rounds, policy) != SPIRAL_ATCG_OK)
            return 1;
        if (spiral_atcg_encrypt_block(&ctx, pt, ct) != SPIRAL_ATCG_OK) return 1;

        long tuple_hd_sum = 0;
        unsigned tuple_min = UINT32_MAX;
        unsigned tuple_max = 0;
        long tuple_zero = 0;
        long tuple_full = 0;

        for (int bit = 0; bit < 256; bit++) {
            memcpy(master_flip, master, 32);
            master_flip[bit / 8] ^= (uint8_t)(1u << (bit % 8));
            if (spiral_atcg_init_session(&ctx_flip, master_flip, nonce, sid, rounds, policy) != SPIRAL_ATCG_OK)
                return 1;
            if (spiral_atcg_encrypt_block(&ctx_flip, pt, ct_flip) != SPIRAL_ATCG_OK)
                return 1;
            unsigned hd = hd320(ct, ct_flip);
            unsigned al = active_lanes_count(ct, ct_flip);
            tuple_hd_sum += hd;
            if (hd < tuple_min) tuple_min = hd;
            if (hd > tuple_max) tuple_max = hd;
            if (hd == 0) tuple_zero++;
            if (al == 10) tuple_full++;
        }
        double avg = (double)tuple_hd_sum / 256.0;
        acc->avg_hd_sum   += avg;
        acc->avg_hd_sumsq += avg * avg;
        acc->min_hd_sum   += (double)tuple_min;
        acc->max_hd_sum   += (double)tuple_max;
        acc->zero_diff_count += tuple_zero;
        acc->full_diff_count += tuple_full;
        acc->total_cells     += 256;
        acc->n_tuples++;
    }
    return 0;
}

static int run_nonce_probe(spiral_atcg_policy_t policy, uint32_t rounds,
                            probe_acc_t *acc) {
    memset(acc, 0, sizeof(*acc));
    for (int t = 0; t < SAMPLE_TUPLES; t++) {
        uint8_t master[32], nonce[16], nonce_flip[16], sid[16], pt[40], ct[40], ct_flip[40];
        make_tuple(0xDEADBEEF12345678ULL ^ ((uint64_t)t * 0x9E3779B97F4A7C15ULL),
                   master, nonce, sid, pt);

        spiral_atcg_ctx_t ctx, ctx_flip;
        if (spiral_atcg_init_session(&ctx, master, nonce, sid, rounds, policy) != SPIRAL_ATCG_OK)
            return 1;
        if (spiral_atcg_encrypt_block(&ctx, pt, ct) != SPIRAL_ATCG_OK) return 1;

        long tuple_hd_sum = 0;
        unsigned tuple_min = UINT32_MAX;
        unsigned tuple_max = 0;
        long tuple_zero = 0;
        long tuple_full = 0;

        for (int bit = 0; bit < 128; bit++) {
            memcpy(nonce_flip, nonce, 16);
            nonce_flip[bit / 8] ^= (uint8_t)(1u << (bit % 8));
            if (spiral_atcg_init_session(&ctx_flip, master, nonce_flip, sid, rounds, policy) != SPIRAL_ATCG_OK)
                return 1;
            if (spiral_atcg_encrypt_block(&ctx_flip, pt, ct_flip) != SPIRAL_ATCG_OK)
                return 1;
            unsigned hd = hd320(ct, ct_flip);
            unsigned al = active_lanes_count(ct, ct_flip);
            tuple_hd_sum += hd;
            if (hd < tuple_min) tuple_min = hd;
            if (hd > tuple_max) tuple_max = hd;
            if (hd == 0) tuple_zero++;
            if (al == 10) tuple_full++;
        }
        double avg = (double)tuple_hd_sum / 128.0;
        acc->avg_hd_sum   += avg;
        acc->avg_hd_sumsq += avg * avg;
        acc->min_hd_sum   += (double)tuple_min;
        acc->max_hd_sum   += (double)tuple_max;
        acc->zero_diff_count += tuple_zero;
        acc->full_diff_count += tuple_full;
        acc->total_cells     += 128;
        acc->n_tuples++;
    }
    return 0;
}

static double mean_of(double sum, int n) { return n > 0 ? sum / (double)n : 0.0; }
static double std_of(double sum, double sumsq, int n) {
    if (n < 2) return 0.0;
    double m = sum / (double)n;
    double v = (sumsq - (double)n * m * m) / (double)(n - 1);
    return v > 0.0 ? sqrt(v) : 0.0;
}

int main(void) {
    fprintf(stderr, "Spiral-ATCG v%s QRES v2 probe (4 policies × 3 probes × %d rounds × %d tuples)\n",
            SPIRAL_ATCG_VERSION_STRING, ROUND_COUNT, SAMPLE_TUPLES);

    /* CSV header */
    printf("policy,probe,rounds,n_tuples,avg_hd_mean,avg_hd_std,min_hd_mean,max_hd_mean,"
           "zero_diff_total,full_diff_pct\n");

    int total_configs = 0;
    for (int p = 0; p < 4; p++) {
        for (int probe = 0; probe < 3; probe++) {
            const char *probe_name = (probe == 0) ? "input" :
                                     (probe == 1) ? "seed" : "nonce";
            for (uint32_t i = 0; i < ROUND_COUNT; i++) {
                uint32_t rounds = ROUNDS_LIST[i];
                probe_acc_t acc;
                int rc = 1;
                if (probe == 0) rc = run_input_probe(POLICIES[p].policy, rounds, &acc);
                else if (probe == 1) rc = run_seed_probe(POLICIES[p].policy, rounds, &acc);
                else rc = run_nonce_probe(POLICIES[p].policy, rounds, &acc);
                if (rc != 0) {
                    fprintf(stderr, "probe failed: policy=%s probe=%s rounds=%u\n",
                            POLICIES[p].name, probe_name, rounds);
                    return 1;
                }
                double avg_mean = mean_of(acc.avg_hd_sum, acc.n_tuples);
                double avg_std  = std_of(acc.avg_hd_sum, acc.avg_hd_sumsq, acc.n_tuples);
                double min_mean = mean_of(acc.min_hd_sum, acc.n_tuples);
                double max_mean = mean_of(acc.max_hd_sum, acc.n_tuples);
                double full_pct = 100.0 * (double)acc.full_diff_count / (double)acc.total_cells;
                printf("Spiral-ATCG/%s,%s,%u,%d,%.4f,%.4f,%.4f,%.4f,%ld,%.2f\n",
                       POLICIES[p].name, probe_name, rounds, acc.n_tuples,
                       avg_mean, avg_std, min_mean, max_mean,
                       acc.zero_diff_count, full_pct);
                fflush(stdout);
                total_configs++;
            }
            fprintf(stderr, "  done: policy=%s probe=%s\n", POLICIES[p].name, probe_name);
        }
    }
    fprintf(stderr, "completed %d configurations\n", total_configs);
    return 0;
}
