/* HISTORICAL v2.2.4 reproducer: compile against the archived v2.2.4 source.
 * It is expected to fail against v2.2.5 and is not a current regression target.
 * Reproduce full-round structural weaknesses; success means flaws reproduced.
 * Deterministic input generation is for reproducibility, not key generation.
 * Uses only the public API; no access to secret round materials.
 */
#include "spiral_atcg.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>

static uint64_t rng = UINT64_C(0x2026110950495241);
static uint64_t next(void) {
    uint64_t z = (rng += UINT64_C(0x9e3779b97f4a7c15));
    z = (z ^ (z >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    z = (z ^ (z >> 27)) * UINT64_C(0x94d049bb133111eb);
    return z ^ (z >> 31);
}
static void fill(uint8_t *p, size_t n) {
    for (size_t i = 0; i < n; ++i) p[i] = (uint8_t)next();
}
static unsigned parity(const uint8_t p[40], uint8_t mask) {
    unsigned v = 0;
    for (unsigned i = 0; i < 40; ++i) {
        unsigned x = p[i] & mask;
        while (x) { v ^= x & 1u; x >>= 1; }
    }
    return v;
}
static void hex(const uint8_t *p) {
    for (unsigned i = 0; i < 40; ++i) printf("%02x", p[i]);
    putchar('\n');
}

int main(void) {
    const uint8_t masks[3] = {0x55, 0xaa, 0xff};
    unsigned long total_parity = 0, total_diff = 0, total_roundtrip = 0;
    for (int policy = 0; policy < 4; ++policy) {
        unsigned long pc = 0, dc = 0, rc = 0;
        for (unsigned rounds = 1; rounds <= SPIRAL_ATCG_MAX_ROUNDS; ++rounds) {
            for (unsigned session = 0; session < 4; ++session) {
                uint8_t key[32], nonce[16], sid[16];
                spiral_atcg_ctx_t ctx;
                fill(key, sizeof key); fill(nonce, sizeof nonce); fill(sid, sizeof sid);
                if (spiral_atcg_init_session(&ctx, key, nonce, sid, rounds,
                    (spiral_atcg_policy_t)policy) != SPIRAL_ATCG_OK) return 2;
                for (unsigned sample = 0; sample < 32; ++sample) {
                    uint8_t p[40], c[40], shifted_p[40], shifted_c[40], recovered[40];
                    fill(p, sizeof p);
                    if (spiral_atcg_encrypt_block(&ctx, p, c) != SPIRAL_ATCG_OK) return 2;
                    if (spiral_atcg_decrypt_block(&ctx, c, recovered) != SPIRAL_ATCG_OK) return 2;
                    if (memcmp(p, recovered, sizeof p)) return 3;
                    ++rc;
                    unsigned pe = parity(p, 0x55), po = parity(p, 0xaa);
                    unsigned ce = parity(c, 0x55), co = parity(c, 0xaa);
                    if (ce != ((rounds & 1u) ? po : pe) ||
                        co != ((rounds & 1u) ? pe : po)) {
                        fprintf(stderr, "Parity prediction failed: policy=%d rounds=%u\n", policy, rounds);
                        return 4;
                    }
                    ++pc;
                    for (unsigned m = 0; m < 3; ++m) {
                        uint8_t out_mask = masks[m];
                        if ((rounds & 1u) && m < 2) out_mask ^= 0xff;
                        for (unsigned b = 0; b < 40; ++b) shifted_p[b] = p[b] ^ masks[m];
                        if (spiral_atcg_encrypt_block(&ctx, shifted_p, shifted_c) != SPIRAL_ATCG_OK) return 2;
                        for (unsigned b = 0; b < 40; ++b) {
                            if ((uint8_t)(c[b] ^ shifted_c[b]) != out_mask) {
                                fprintf(stderr, "Differential prediction failed: policy=%d rounds=%u mask=%02x\n", policy, rounds, masks[m]);
                                return 5;
                            }
                        }
                        ++dc;
                        if (policy == 0 && rounds == 16 && session == 0 && sample == 0 && m == 0) {
                            printf("Example BASE, 16 rounds, delta=55 repeated 40 bytes\nP  = "); hex(p);
                            printf("C  = "); hex(c);
                            printf("P' = "); hex(shifted_p);
                            printf("C' = "); hex(shifted_c);
                            printf("parity_even(P,C)=%u,%u parity_odd(P,C)=%u,%u\n", pe, ce, po, co);
                        }
                    }
                }
            }
        }
        printf("%s rounds=1..32 sessions_per_round=4 samples_per_session=32 parity=%lu/%lu differential=%lu/%lu roundtrip=%lu/%lu\n",
               spiral_atcg_policy_name((spiral_atcg_policy_t)policy), pc, pc, dc, dc, rc, rc);
        total_parity += pc; total_diff += dc; total_roundtrip += rc;
    }
    printf("REPRODUCED: parity_pairs=%lu deterministic_differentials=%lu roundtrips=%lu\n",
           total_parity, total_diff, total_roundtrip);
    return 0;
}
