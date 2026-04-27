#ifndef SPIRAL_ATCG_H
#define SPIRAL_ATCG_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SPIRAL_ATCG_BLOCK_BYTES     40u
#define SPIRAL_ATCG_MASTER_BYTES    32u
#define SPIRAL_ATCG_NONCE_BYTES     16u
#define SPIRAL_ATCG_SESSION_BYTES   16u
#define SPIRAL_ATCG_WORDS           10u
#define SPIRAL_ATCG_MAX_ROUNDS      32u
#define SPIRAL_ATCG_DEFAULT_ROUNDS  16u
#define SPIRAL_ATCG_VERSION_STRING  "2.2.4-full-scalar-cleanup"

typedef enum {
    SPIRAL_ATCG_OK = 0,
    SPIRAL_ATCG_ERR_NULL = -1,
    SPIRAL_ATCG_ERR_ROUNDS = -2,
    SPIRAL_ATCG_ERR_POLICY = -3,
    SPIRAL_ATCG_ERR_COMPAT = -4
} spiral_status_t;

typedef enum {
    SPIRAL_ATCG_POLICY_BASE = 0,
    SPIRAL_ATCG_POLICY_SAFE = 1,
    SPIRAL_ATCG_POLICY_AGGR = 2,
    SPIRAL_ATCG_POLICY_NULL = 3
} spiral_atcg_policy_t;

typedef struct {
    uint32_t A[SPIRAL_ATCG_WORDS];
    uint32_t T[SPIRAL_ATCG_WORDS];
    uint32_t C[SPIRAL_ATCG_WORDS];
    uint32_t G[SPIRAL_ATCG_WORDS];
} spiral_atcg_tokenset_t;

typedef struct {
    spiral_atcg_tokenset_t main_strand[SPIRAL_ATCG_MAX_ROUNDS];
    spiral_atcg_tokenset_t var_strand [SPIRAL_ATCG_MAX_ROUNDS];
    uint32_t rounds;
    spiral_atcg_policy_t policy;
} spiral_atcg_strands_t;

typedef struct {
    const spiral_atcg_tokenset_t *x;
    const spiral_atcg_tokenset_t *y;
    uint32_t round;
    uint32_t paired_round;
} spiral_atcg_pair_t;

typedef struct {
    uint32_t LM[SPIRAL_ATCG_WORDS];
    uint32_t RC[SPIRAL_ATCG_WORDS];
    uint32_t CD[SPIRAL_ATCG_WORDS];
    uint32_t SK[SPIRAL_ATCG_WORDS];
} spiral_round_material_t;

/* Derived per-round constants for the v2.2-strength substrate hot path.
 * They are fully determined by spiral_round_material_t and the round index,
 * so caching them in init_session() is bit-exact and keeps KAT unchanged. */
typedef struct {
    uint32_t k[5];
    uint32_t s_xor[5];
    uint8_t alpha[5];
    uint8_t beta[5];
    uint8_t rot_a[5];
    uint8_t rot_b[5];
} spiral_round_cache_t;

typedef struct {
    spiral_atcg_strands_t strands;
    spiral_round_material_t materials[SPIRAL_ATCG_MAX_ROUNDS];
    spiral_round_cache_t round_cache[SPIRAL_ATCG_MAX_ROUNDS];
    uint32_t rounds;
    spiral_atcg_policy_t policy;
} spiral_atcg_ctx_t;

const char *spiral_atcg_policy_name(spiral_atcg_policy_t policy);

spiral_status_t spiral_atcg_project(
    const uint8_t master_seed[SPIRAL_ATCG_MASTER_BYTES],
    const uint8_t nonce[SPIRAL_ATCG_NONCE_BYTES],
    const uint8_t session_id[SPIRAL_ATCG_SESSION_BYTES],
    uint32_t rounds,
    spiral_atcg_policy_t policy,
    spiral_atcg_strands_t *out
);

spiral_status_t spiral_atcg_pair(
    const spiral_atcg_strands_t *strands,
    uint32_t r,
    spiral_atcg_pair_t *out_pair
);

spiral_status_t spiral_atcg_transcribe_pair(
    const spiral_atcg_pair_t *pair,
    spiral_atcg_policy_t policy,
    spiral_round_material_t *out
);

spiral_status_t spiral_atcg_transcribe_round(
    const spiral_atcg_strands_t *strands,
    uint32_t r,
    spiral_round_material_t *out
);

spiral_status_t spiral_atcg_precompute_materials(
    const spiral_atcg_strands_t *strands,
    spiral_round_material_t materials[SPIRAL_ATCG_MAX_ROUNDS]
);

spiral_status_t spiral_atcg_init_session(
    spiral_atcg_ctx_t *ctx,
    const uint8_t master_seed[SPIRAL_ATCG_MASTER_BYTES],
    const uint8_t nonce[SPIRAL_ATCG_NONCE_BYTES],
    const uint8_t session_id[SPIRAL_ATCG_SESSION_BYTES],
    uint32_t rounds,
    spiral_atcg_policy_t policy
);

spiral_status_t spiral_atcg_encrypt_block(
    const spiral_atcg_ctx_t *ctx,
    const uint8_t plaintext[SPIRAL_ATCG_BLOCK_BYTES],
    uint8_t ciphertext[SPIRAL_ATCG_BLOCK_BYTES]
);

spiral_status_t spiral_atcg_decrypt_block(
    const spiral_atcg_ctx_t *ctx,
    const uint8_t ciphertext[SPIRAL_ATCG_BLOCK_BYTES],
    uint8_t plaintext[SPIRAL_ATCG_BLOCK_BYTES]
);

/* Level-2 fast scalar batch API.
 * Bit-exact equivalent to calling encrypt/decrypt_block repeatedly.
 * Reduces per-block API overhead and enables fixed-16-round dispatch. */
spiral_status_t spiral_atcg_encrypt_blocks(
    const spiral_atcg_ctx_t *ctx,
    const uint8_t *plaintext_blocks,
    uint8_t *ciphertext_blocks,
    size_t blocks
);

spiral_status_t spiral_atcg_decrypt_blocks(
    const spiral_atcg_ctx_t *ctx,
    const uint8_t *ciphertext_blocks,
    uint8_t *plaintext_blocks,
    size_t blocks
);

#ifdef __cplusplus
}
#endif

#endif
