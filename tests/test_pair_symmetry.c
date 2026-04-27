#include "spiral_atcg.h"
#include <stdio.h>
#include <string.h>

static void fill(uint8_t *p, size_t n, uint8_t seed) {
    for (size_t i = 0; i < n; ++i) p[i] = (uint8_t)(seed + i * 29u + (i >> 2));
}

static int material_equal(const spiral_round_material_t *a, const spiral_round_material_t *b) {
    return memcmp(a, b, sizeof(*a)) == 0;
}

int main(void) {
    spiral_atcg_policy_t policies[] = { SPIRAL_ATCG_POLICY_BASE, SPIRAL_ATCG_POLICY_SAFE, SPIRAL_ATCG_POLICY_AGGR, SPIRAL_ATCG_POLICY_NULL };
    uint8_t master[32], nonce[16], sid[16];
    fill(master, sizeof(master), 0x21);
    fill(nonce, sizeof(nonce), 0x43);
    fill(sid, sizeof(sid), 0x65);

    for (size_t pi = 0; pi < sizeof(policies)/sizeof(policies[0]); ++pi) {
        spiral_atcg_strands_t strands;
        spiral_status_t st = spiral_atcg_project(master, nonce, sid, SPIRAL_ATCG_DEFAULT_ROUNDS, policies[pi], &strands);
        if (st != SPIRAL_ATCG_OK) { printf("project failed st=%d\n", st); return 1; }
        for (uint32_t r = 0; r < strands.rounds; ++r) {
            spiral_atcg_pair_t p1, p2;
            spiral_round_material_t m1, m2;
            st = spiral_atcg_pair(&strands, r, &p1);
            if (st != SPIRAL_ATCG_OK) return 1;
            p2.x = p1.y;
            p2.y = p1.x;
            p2.round = p1.round;
            p2.paired_round = p1.paired_round;
            st = spiral_atcg_transcribe_pair(&p1, policies[pi], &m1);
            if (st != SPIRAL_ATCG_OK) { printf("transcribe p1 failed policy=%s st=%d\n", spiral_atcg_policy_name(policies[pi]), st); return 1; }
            st = spiral_atcg_transcribe_pair(&p2, policies[pi], &m2);
            if (st != SPIRAL_ATCG_OK) { printf("transcribe p2 failed policy=%s st=%d\n", spiral_atcg_policy_name(policies[pi]), st); return 1; }
            if (!material_equal(&m1, &m2)) {
                printf("PAIR SYMMETRY FAILED policy=%s r=%u\n", spiral_atcg_policy_name(policies[pi]), r);
                return 1;
            }
        }
        printf("PAIR SYMMETRY PASSED policy=%s\n", spiral_atcg_policy_name(policies[pi]));
    }
    printf("ALL PAIR SYMMETRY TESTS PASSED\n");
    return 0;
}
