#include "spiral_atcg.h"
#include <stdio.h>

static void fill(uint8_t *p, size_t n, uint8_t seed) {
    for (size_t i = 0; i < n; ++i) p[i] = (uint8_t)(seed + i * 13u);
}

int main(void) {
    uint8_t master[32], nonce[16], sid[16];
    fill(master, sizeof(master), 0x11);
    fill(nonce, sizeof(nonce), 0x22);
    fill(sid, sizeof(sid), 0x33);

    spiral_atcg_strands_t strands;
    spiral_status_t st = spiral_atcg_project(master, nonce, sid, SPIRAL_ATCG_DEFAULT_ROUNDS, SPIRAL_ATCG_POLICY_BASE, &strands);
    if (st != SPIRAL_ATCG_OK) return 1;

    printf("Spiral-ATCG 2.1-fast material trace policy=%s rounds=%u\n", spiral_atcg_policy_name(strands.policy), strands.rounds);
    for (uint32_t r = 0; r < strands.rounds; ++r) {
        spiral_atcg_pair_t pair;
        spiral_round_material_t m;
        spiral_atcg_pair(&strands, r, &pair);
        spiral_atcg_transcribe_round(&strands, r, &m);
        printf("r=%02u pair=%02u LM0=%08x RC0=%08x CD0=%08x SK0=%08x\n",
               r, pair.paired_round, m.LM[0], m.RC[0], m.CD[0], m.SK[0]);
    }
    return 0;
}
