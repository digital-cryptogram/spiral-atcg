#include "spiral_atcg.h"
#include <stdio.h>
#include <string.h>

static void fill(uint8_t *p, size_t n, uint8_t seed) {
    for (size_t i = 0; i < n; ++i) p[i] = (uint8_t)(seed + i * 17u + (i >> 1));
}

int main(void) {
    spiral_atcg_policy_t policies[] = {
        SPIRAL_ATCG_POLICY_BASE,
        SPIRAL_ATCG_POLICY_SAFE,
        SPIRAL_ATCG_POLICY_AGGR,
        SPIRAL_ATCG_POLICY_NULL
    };

    uint8_t master[32], nonce[16], sid[16], pt[40], ct[40], dt[40];
    fill(master, sizeof(master), 0x10);
    fill(nonce, sizeof(nonce), 0x30);
    fill(sid, sizeof(sid), 0x50);

    for (size_t p = 0; p < sizeof(policies)/sizeof(policies[0]); ++p) {
        for (int tv = 0; tv < 128; ++tv) {
            fill(pt, sizeof(pt), (uint8_t)(0x70 + tv));
            master[0] = (uint8_t)(0x10 + tv);
            nonce[3] = (uint8_t)(0x40 + tv * 3);
            sid[7] = (uint8_t)(0x60 ^ tv);

            spiral_atcg_ctx_t ctx;
            spiral_status_t st = spiral_atcg_init_session(&ctx, master, nonce, sid, SPIRAL_ATCG_DEFAULT_ROUNDS, policies[p]);
            if (st != SPIRAL_ATCG_OK) {
                printf("init failed policy=%s st=%d\n", spiral_atcg_policy_name(policies[p]), st);
                return 1;
            }
            st = spiral_atcg_encrypt_block(&ctx, pt, ct);
            if (st != SPIRAL_ATCG_OK) { printf("encrypt failed st=%d\n", st); return 1; }
            st = spiral_atcg_decrypt_block(&ctx, ct, dt);
            if (st != SPIRAL_ATCG_OK) { printf("decrypt failed st=%d\n", st); return 1; }
            if (memcmp(pt, dt, sizeof(pt)) != 0) {
                printf("ROUNDTRIP FAILED policy=%s tv=%d\n", spiral_atcg_policy_name(policies[p]), tv);
                return 1;
            }
        }
        printf("ROUNDTRIP PASSED policy=%s\n", spiral_atcg_policy_name(policies[p]));
    }
    printf("ALL V2 ROUNDTRIPS PASSED\n");
    return 0;
}
