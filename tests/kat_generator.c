/*
 * Spiral-ATCG v2.1 KAT (Known-Answer Test) generator.
 *
 * Generates 5 test scenarios x 4 policies = 20 KAT entries.
 * Each entry: (master_seed, nonce, session_id, plaintext) -> ciphertext.
 * Also verifies roundtrip on every entry.
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "spiral_atcg.h"

static void hex_dump(const char *label, const uint8_t *p, size_t n) {
    printf("%s = ", label);
    for (size_t i = 0; i < n; ++i) printf("%02x", p[i]);
    printf("\n");
}

typedef struct {
    const char *name;
    const char *desc;
    uint8_t master[32];
    uint8_t nonce[16];
    uint8_t sid[16];
    uint8_t plaintext[40];
} kat_scenario_t;

static void scenario_all_zero(kat_scenario_t *s) {
    s->name = "TV1";
    s->desc = "all-zero master/nonce/sid/plaintext";
    memset(s->master, 0, 32);
    memset(s->nonce,  0, 16);
    memset(s->sid,    0, 16);
    memset(s->plaintext, 0, 40);
}

static void scenario_sequential(kat_scenario_t *s) {
    s->name = "TV2";
    s->desc = "sequential bytes 0x00..0x67 across all inputs";
    for (int i = 0; i < 32; i++) s->master[i] = (uint8_t)(0x00 + i);
    for (int i = 0; i < 16; i++) s->nonce[i]  = (uint8_t)(0x20 + i);
    for (int i = 0; i < 16; i++) s->sid[i]    = (uint8_t)(0x30 + i);
    for (int i = 0; i < 40; i++) s->plaintext[i] = (uint8_t)(0x40 + i);
}

static void scenario_single_bit_pt(kat_scenario_t *s) {
    s->name = "TV3";
    s->desc = "all-zero session inputs, single-bit plaintext (bit 0 of byte 0)";
    memset(s->master, 0, 32);
    memset(s->nonce,  0, 16);
    memset(s->sid,    0, 16);
    memset(s->plaintext, 0, 40);
    s->plaintext[0] = 0x01;
}

static void scenario_random_like(kat_scenario_t *s) {
    s->name = "TV4";
    s->desc = "deterministic SplitMix64-derived (master, nonce, sid, plaintext)";
    /* Inline SplitMix64 to avoid external deps. */
    uint64_t st = 0x123456789ABCDEF0ULL;
    uint8_t *bufs[4]  = {s->master, s->nonce, s->sid, s->plaintext};
    size_t  sizes[4]  = {32, 16, 16, 40};
    for (int b = 0; b < 4; b++) {
        for (size_t i = 0; i < sizes[b]; i++) {
            st += 0x9E3779B97F4A7C15ULL;
            uint64_t z = st;
            z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
            z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
            z = z ^ (z >> 31);
            bufs[b][i] = (uint8_t)(z & 0xFF);
        }
    }
}

static void scenario_all_0xff(kat_scenario_t *s) {
    s->name = "TV5";
    s->desc = "all-0xff master/nonce/sid/plaintext";
    memset(s->master, 0xFF, 32);
    memset(s->nonce,  0xFF, 16);
    memset(s->sid,    0xFF, 16);
    memset(s->plaintext, 0xFF, 40);
}

static int run_kat_entry(const kat_scenario_t *s, spiral_atcg_policy_t policy) {
    spiral_atcg_ctx_t ctx;
    uint8_t ciphertext[40];
    uint8_t recovered[40];

    spiral_status_t st = spiral_atcg_init_session(&ctx, s->master, s->nonce, s->sid,
                                                   SPIRAL_ATCG_DEFAULT_ROUNDS, policy);
    if (st != SPIRAL_ATCG_OK) {
        fprintf(stderr, "init_session failed: TV=%s policy=%s status=%d\n",
                s->name, spiral_atcg_policy_name(policy), st);
        return 1;
    }

    st = spiral_atcg_encrypt_block(&ctx, s->plaintext, ciphertext);
    if (st != SPIRAL_ATCG_OK) {
        fprintf(stderr, "encrypt failed: TV=%s policy=%s status=%d\n",
                s->name, spiral_atcg_policy_name(policy), st);
        return 1;
    }

    st = spiral_atcg_decrypt_block(&ctx, ciphertext, recovered);
    if (st != SPIRAL_ATCG_OK) {
        fprintf(stderr, "decrypt failed: TV=%s policy=%s status=%d\n",
                s->name, spiral_atcg_policy_name(policy), st);
        return 1;
    }

    int rt_ok = (memcmp(s->plaintext, recovered, 40) == 0);

    printf("\n--- %s policy=%s rounds=%u rc=%s roundtrip=%s ---\n",
           s->name, spiral_atcg_policy_name(policy),
           SPIRAL_ATCG_DEFAULT_ROUNDS, "BitRev(~x)",
           rt_ok ? "PASS" : "FAIL");
    hex_dump("master    ", s->master,    32);
    hex_dump("nonce     ", s->nonce,     16);
    hex_dump("sid       ", s->sid,       16);
    hex_dump("plaintext ", s->plaintext, 40);
    hex_dump("ciphertext", ciphertext,   40);

    return rt_ok ? 0 : 1;
}

int main(void) {
    printf("Spiral-ATCG v%s KAT generation\n", SPIRAL_ATCG_VERSION_STRING);
    printf("rounds=%u block=%u master=%u nonce=%u session=%u rc=%s\n",
           SPIRAL_ATCG_DEFAULT_ROUNDS,
           SPIRAL_ATCG_BLOCK_BYTES,
           SPIRAL_ATCG_MASTER_BYTES,
           SPIRAL_ATCG_NONCE_BYTES,
           SPIRAL_ATCG_SESSION_BYTES,
           "BitRev(~x)");

    kat_scenario_t scenarios[5];
    scenario_all_zero       (&scenarios[0]);
    scenario_sequential     (&scenarios[1]);
    scenario_single_bit_pt  (&scenarios[2]);
    scenario_random_like    (&scenarios[3]);
    scenario_all_0xff       (&scenarios[4]);

    spiral_atcg_policy_t policies[4] = {
        SPIRAL_ATCG_POLICY_BASE,
        SPIRAL_ATCG_POLICY_SAFE,
        SPIRAL_ATCG_POLICY_AGGR,
        SPIRAL_ATCG_POLICY_NULL
    };

    int failures = 0;
    int policy_collision_failures = 0;

    for (size_t i = 0; i < 5; i++) {
        printf("\n=== Scenario %s: %s ===\n", scenarios[i].name, scenarios[i].desc);

        uint8_t cts[4][SPIRAL_ATCG_BLOCK_BYTES];
        for (int p = 0; p < 4; p++) {
            failures += run_kat_entry(&scenarios[i], policies[p]);

            spiral_atcg_ctx_t ctx;
            if (spiral_atcg_init_session(&ctx, scenarios[i].master, scenarios[i].nonce, scenarios[i].sid,
                                         SPIRAL_ATCG_DEFAULT_ROUNDS, policies[p]) == SPIRAL_ATCG_OK) {
                spiral_atcg_encrypt_block(&ctx, scenarios[i].plaintext, cts[p]);
            } else {
                memset(cts[p], 0, sizeof(cts[p]));
            }
        }

        int scenario_collision = 0;
        for (int a = 0; a < 4; a++) {
            for (int b = a + 1; b < 4; b++) {
                if (memcmp(cts[a], cts[b], SPIRAL_ATCG_BLOCK_BYTES) == 0) {
                    fprintf(stderr, "POLICY COLLISION: %s %s == %s\n",
                            scenarios[i].name,
                            spiral_atcg_policy_name(policies[a]),
                            spiral_atcg_policy_name(policies[b]));
                    policy_collision_failures++;
                    scenario_collision = 1;
                }
            }
        }
        printf("Policy-axis check: %s; all 4 policy ciphertexts are %s for %s\n",
               scenario_collision ? "FAIL" : "PASS",
               scenario_collision ? "not distinct" : "distinct",
               scenarios[i].name);
    }

    printf("\n========================================\n");
    if (failures == 0 && policy_collision_failures == 0) {
        printf("ALL 20 KAT ENTRIES PASSED ROUNDTRIP\n");
        printf("ALL 5 SCENARIOS PASSED POLICY-DISTINCTNESS CHECK\n");
    } else {
        printf("ROUNDTRIP FAILURES: %d / 20\n", failures);
        printf("POLICY COLLISION FAILURES: %d\n", policy_collision_failures);
    }
    return failures + policy_collision_failures;
}
