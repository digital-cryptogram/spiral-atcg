#include "spiral_atcg.h"
#include <stdio.h>
#include <string.h>

#define BLOCKS 8

static void fill(uint8_t *p, size_t n, uint8_t seed) {
    for (size_t i = 0; i < n; ++i) p[i] = (uint8_t)(seed + i * 17u + (i >> 1));
}

int main(void) {
    spiral_atcg_policy_t policies[] = { SPIRAL_ATCG_POLICY_BASE, SPIRAL_ATCG_POLICY_SAFE, SPIRAL_ATCG_POLICY_AGGR, SPIRAL_ATCG_POLICY_NULL };
    uint8_t master[32], nonce[16], sid[16];
    uint8_t in[BLOCKS * SPIRAL_ATCG_BLOCK_BYTES];
    uint8_t batch_ct[BLOCKS * SPIRAL_ATCG_BLOCK_BYTES];
    uint8_t batch_pt[BLOCKS * SPIRAL_ATCG_BLOCK_BYTES];
    uint8_t single_ct[BLOCKS * SPIRAL_ATCG_BLOCK_BYTES];
    uint8_t single_pt[BLOCKS * SPIRAL_ATCG_BLOCK_BYTES];
    fill(master, sizeof(master), 0x31);
    fill(nonce, sizeof(nonce), 0x42);
    fill(sid, sizeof(sid), 0x53);
    fill(in, sizeof(in), 0x64);

    for (size_t pi = 0; pi < sizeof(policies)/sizeof(policies[0]); ++pi) {
        spiral_atcg_ctx_t ctx;
        spiral_status_t st = spiral_atcg_init_session(&ctx, master, nonce, sid, SPIRAL_ATCG_DEFAULT_ROUNDS, policies[pi]);
        if (st != SPIRAL_ATCG_OK) { printf("init failed policy=%s st=%d\n", spiral_atcg_policy_name(policies[pi]), st); return 1; }

        st = spiral_atcg_encrypt_blocks(&ctx, in, batch_ct, BLOCKS);
        if (st != SPIRAL_ATCG_OK) { printf("batch encrypt failed policy=%s st=%d\n", spiral_atcg_policy_name(policies[pi]), st); return 1; }
        st = spiral_atcg_decrypt_blocks(&ctx, batch_ct, batch_pt, BLOCKS);
        if (st != SPIRAL_ATCG_OK) { printf("batch decrypt failed policy=%s st=%d\n", spiral_atcg_policy_name(policies[pi]), st); return 1; }

        for (size_t b = 0; b < BLOCKS; ++b) {
            st = spiral_atcg_encrypt_block(&ctx, in + b * SPIRAL_ATCG_BLOCK_BYTES, single_ct + b * SPIRAL_ATCG_BLOCK_BYTES);
            if (st != SPIRAL_ATCG_OK) return 1;
            st = spiral_atcg_decrypt_block(&ctx, single_ct + b * SPIRAL_ATCG_BLOCK_BYTES, single_pt + b * SPIRAL_ATCG_BLOCK_BYTES);
            if (st != SPIRAL_ATCG_OK) return 1;
        }

        if (memcmp(batch_ct, single_ct, sizeof(batch_ct)) != 0) {
            printf("BATCH ENCRYPT MISMATCH policy=%s\n", spiral_atcg_policy_name(policies[pi])); return 1;
        }
        if (memcmp(batch_pt, in, sizeof(in)) != 0 || memcmp(single_pt, in, sizeof(in)) != 0) {
            printf("BATCH ROUNDTRIP MISMATCH policy=%s\n", spiral_atcg_policy_name(policies[pi])); return 1;
        }
        printf("BATCH API PASSED policy=%s\n", spiral_atcg_policy_name(policies[pi]));
    }
    printf("ALL BATCH API TESTS PASSED\n");
    return 0;
}
