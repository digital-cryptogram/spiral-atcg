#include "spiral_atcg.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <time.h>
#endif

#ifndef ATCG_BENCH_BLOCKS
#define ATCG_BENCH_BLOCKS 5000
#endif

#ifndef ATCG_SETUP_ITERS
#define ATCG_SETUP_ITERS 100
#endif

#ifndef ATCG_WARMUP_BLOCKS
#define ATCG_WARMUP_BLOCKS 256
#endif

static void fill(uint8_t *p, size_t n, uint8_t seed) {
    for (size_t i = 0; i < n; ++i) p[i] = (uint8_t)(seed + i * 19u + (i >> 3));
}

static double now_sec(void) {
#if defined(_WIN32)
    static LARGE_INTEGER freq;
    LARGE_INTEGER counter;
    if (freq.QuadPart == 0) QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart / (double)freq.QuadPart;
#else
    struct timespec ts;
#if defined(CLOCK_MONOTONIC_RAW)
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
#else
    clock_gettime(CLOCK_MONOTONIC, &ts);
#endif
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1000000000.0;
#endif
}

static uint8_t checksum_buf(const uint8_t *p, size_t n) {
    uint8_t c = 0;
    for (size_t i = 0; i < n; ++i) c ^= (uint8_t)(p[i] + (uint8_t)i);
    return c;
}

static void prepare_blocks(uint8_t *p, size_t blocks, uint8_t seed) {
    fill(p, blocks * SPIRAL_ATCG_BLOCK_BYTES, seed);
    for (size_t b = 0; b < blocks; ++b) {
        p[b * SPIRAL_ATCG_BLOCK_BYTES + 0] ^= (uint8_t)b;
        p[b * SPIRAL_ATCG_BLOCK_BYTES + 17] += (uint8_t)(b >> 3);
        p[b * SPIRAL_ATCG_BLOCK_BYTES + 29] ^= (uint8_t)(b * 13u);
    }
}

static void warmup(const spiral_atcg_ctx_t *ctx) {
    uint8_t in[ATCG_WARMUP_BLOCKS * SPIRAL_ATCG_BLOCK_BYTES];
    uint8_t out[ATCG_WARMUP_BLOCKS * SPIRAL_ATCG_BLOCK_BYTES];
    prepare_blocks(in, ATCG_WARMUP_BLOCKS, 0x66);
    (void)spiral_atcg_encrypt_blocks(ctx, in, out, ATCG_WARMUP_BLOCKS);
    (void)spiral_atcg_decrypt_blocks(ctx, out, in, ATCG_WARMUP_BLOCKS);
}

static double bench_encrypt_batch(const spiral_atcg_ctx_t *ctx, int blocks, uint8_t *checksum_out) {
    size_t n = (size_t)blocks * SPIRAL_ATCG_BLOCK_BYTES;
    uint8_t *in = (uint8_t *)malloc(n);
    uint8_t *out = (uint8_t *)malloc(n);
    if (!in || !out) { free(in); free(out); return 0.0; }
    prepare_blocks(in, (size_t)blocks, 0x40);

    double t0 = now_sec();
    (void)spiral_atcg_encrypt_blocks(ctx, in, out, (size_t)blocks);
    double t1 = now_sec();
    *checksum_out ^= checksum_buf(out, n);
    free(in); free(out);
    return t1 - t0;
}

static double bench_decrypt_batch(const spiral_atcg_ctx_t *ctx, int blocks, uint8_t *checksum_out) {
    size_t n = (size_t)blocks * SPIRAL_ATCG_BLOCK_BYTES;
    uint8_t *plain = (uint8_t *)malloc(n);
    uint8_t *cipher = (uint8_t *)malloc(n);
    uint8_t *out = (uint8_t *)malloc(n);
    if (!plain || !cipher || !out) { free(plain); free(cipher); free(out); return 0.0; }
    prepare_blocks(plain, (size_t)blocks, 0x90);
    (void)spiral_atcg_encrypt_blocks(ctx, plain, cipher, (size_t)blocks);

    double t0 = now_sec();
    (void)spiral_atcg_decrypt_blocks(ctx, cipher, out, (size_t)blocks);
    double t1 = now_sec();
    *checksum_out ^= checksum_buf(out, n);
    free(plain); free(cipher); free(out);
    return t1 - t0;
}

int main(void) {
    uint8_t master[SPIRAL_ATCG_MASTER_BYTES];
    uint8_t nonce[SPIRAL_ATCG_NONCE_BYTES];
    uint8_t sid[SPIRAL_ATCG_SESSION_BYTES];
    fill(master, sizeof(master), 0x10);
    fill(nonce, sizeof(nonce), 0x20);
    fill(sid, sizeof(sid), 0x30);

    spiral_atcg_policy_t policies[] = {
        SPIRAL_ATCG_POLICY_BASE,
        SPIRAL_ATCG_POLICY_SAFE,
        SPIRAL_ATCG_POLICY_AGGR,
        SPIRAL_ATCG_POLICY_NULL
    };

    printf("Spiral-ATCG %s fast-scalar batch benchmark\n", SPIRAL_ATCG_VERSION_STRING);
    printf("rounds=%u block=%u data_blocks=%d setup_iters=%d warmup_blocks=%d\n",
           SPIRAL_ATCG_DEFAULT_ROUNDS,
           SPIRAL_ATCG_BLOCK_BYTES,
           ATCG_BENCH_BLOCKS,
           ATCG_SETUP_ITERS,
           ATCG_WARMUP_BLOCKS);
    printf("note: block path uses cached materials, fixed-16-round fast scalar dispatch, and batch API.\n\n");

    for (size_t pi = 0; pi < sizeof(policies) / sizeof(policies[0]); ++pi) {
        spiral_atcg_ctx_t ctx;
        uint8_t checksum = 0;

        double s0 = now_sec();
        for (int i = 0; i < ATCG_SETUP_ITERS; ++i) {
            nonce[0] = (uint8_t)(0x20u + (uint8_t)i);
            nonce[7] = (uint8_t)(0x55u ^ (uint8_t)(i >> 1));
            spiral_status_t st = spiral_atcg_init_session(&ctx, master, nonce, sid,
                                                           SPIRAL_ATCG_DEFAULT_ROUNDS,
                                                           policies[pi]);
            if (st != SPIRAL_ATCG_OK) {
                printf("init failed policy=%s st=%d\n", spiral_atcg_policy_name(policies[pi]), st);
                return 1;
            }
            checksum ^= ctx.materials[(unsigned)i % SPIRAL_ATCG_DEFAULT_ROUNDS].LM[0] & 0xffu;
        }
        double s1 = now_sec();

        warmup(&ctx);
        double enc_s = bench_encrypt_batch(&ctx, ATCG_BENCH_BLOCKS, &checksum);
        double dec_s = bench_decrypt_batch(&ctx, ATCG_BENCH_BLOCKS, &checksum);

        double mib = ((double)ATCG_BENCH_BLOCKS * (double)SPIRAL_ATCG_BLOCK_BYTES) / (1024.0 * 1024.0);
        double setup_us = ((s1 - s0) * 1000000.0) / (double)ATCG_SETUP_ITERS;
        double enc_mib = enc_s > 0.0 ? mib / enc_s : 0.0;
        double dec_mib = dec_s > 0.0 ? mib / dec_s : 0.0;
        double enc_ns = enc_s > 0.0 ? (enc_s * 1000000000.0) / (double)ATCG_BENCH_BLOCKS : 0.0;
        double dec_ns = dec_s > 0.0 ? (dec_s * 1000000000.0) / (double)ATCG_BENCH_BLOCKS : 0.0;

        printf("[%s]\n", spiral_atcg_policy_name(policies[pi]));
        printf("  setup   : %.3f us/session\n", setup_us);
        printf("  encrypt : %.3f MiB/s, %.1f ns/block\n", enc_mib, enc_ns);
        printf("  decrypt : %.3f MiB/s, %.1f ns/block\n", dec_mib, dec_ns);
        printf("  checksum: %02x\n\n", checksum);
    }
    return 0;
}
