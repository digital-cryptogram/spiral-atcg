#include "spiral_atcg.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

static unsigned popcnt8(uint8_t x) {
    unsigned c = 0;
    while (x) { c += (unsigned)(x & 1u); x >>= 1; }
    return c;
}

static int run_policy(spiral_atcg_policy_t policy) {
    uint8_t seed[SPIRAL_ATCG_MASTER_BYTES];
    uint8_t nonce[SPIRAL_ATCG_NONCE_BYTES];
    uint8_t sid[SPIRAL_ATCG_SESSION_BYTES];
    uint8_t p0[SPIRAL_ATCG_BLOCK_BYTES];
    uint8_t p1[SPIRAL_ATCG_BLOCK_BYTES];
    uint8_t c0[SPIRAL_ATCG_BLOCK_BYTES];
    uint8_t c1[SPIRAL_ATCG_BLOCK_BYTES];
    spiral_atcg_ctx_t ctx;

    for (size_t i = 0; i < sizeof(seed); ++i) seed[i] = (uint8_t)(0x10u + i * 3u);
    for (size_t i = 0; i < sizeof(nonce); ++i) nonce[i] = (uint8_t)(0xA0u + i * 5u);
    for (size_t i = 0; i < sizeof(sid); ++i) sid[i] = (uint8_t)(0xC0u + i * 7u);
    for (size_t i = 0; i < sizeof(p0); ++i) p0[i] = (uint8_t)(0x30u + i * 11u);
    memcpy(p1, p0, sizeof(p0));
    p1[0] ^= 0x01u;

    spiral_status_t st = spiral_atcg_init_session(&ctx, seed, nonce, sid, SPIRAL_ATCG_DEFAULT_ROUNDS, policy);
    if (st != SPIRAL_ATCG_OK) {
        printf("init failed policy=%s st=%d\n", spiral_atcg_policy_name(policy), (int)st);
        return 1;
    }
    spiral_atcg_encrypt_block(&ctx, p0, c0);
    spiral_atcg_encrypt_block(&ctx, p1, c1);

    unsigned hd = 0;
    unsigned active_lanes = 0;
    printf("AVALANCHE policy=%s\n", spiral_atcg_policy_name(policy));
    for (size_t lane = 0; lane < SPIRAL_ATCG_WORDS; ++lane) {
        unsigned lane_hd = 0;
        for (size_t b = 0; b < 4; ++b) {
            lane_hd += popcnt8((uint8_t)(c0[lane * 4u + b] ^ c1[lane * 4u + b]));
        }
        if (lane_hd != 0u) active_lanes++;
        hd += lane_hd;
        printf("  lane %zu hd=%u\n", lane, lane_hd);
    }
    printf("  total_hd=%u active_lanes=%u/10\n", hd, active_lanes);

    if (active_lanes != SPIRAL_ATCG_WORDS) {
        printf("FAIL: expected all lanes active\n");
        return 1;
    }
    if (hd < 120u || hd > 200u) {
        printf("FAIL: expected HD roughly around 160, got %u\n", hd);
        return 1;
    }
    return 0;
}

int main(void) {
    int fail = 0;
    fail |= run_policy(SPIRAL_ATCG_POLICY_BASE);
    fail |= run_policy(SPIRAL_ATCG_POLICY_SAFE);
    fail |= run_policy(SPIRAL_ATCG_POLICY_AGGR);
    fail |= run_policy(SPIRAL_ATCG_POLICY_NULL);
    if (fail) return 1;
    printf("ALL AVALANCHE TESTS PASSED\n");
    return 0;
}
