#include "spiral_atcg.h"
#include <string.h>
#include <stdio.h>

#if defined(__GNUC__) || defined(__clang__)
#define ATCG_ALWAYS_INLINE static inline __attribute__((always_inline))
#else
#define ATCG_ALWAYS_INLINE static inline
#endif

#define ATCG_WORDS ((int)SPIRAL_ATCG_WORDS)

ATCG_ALWAYS_INLINE uint32_t rotl32(uint32_t x, int r) { r &= 31; return r ? ((x << r) | (x >> (32 - r))) : x; }
ATCG_ALWAYS_INLINE uint32_t rotr32(uint32_t x, int r) { r &= 31; return r ? ((x >> r) | (x << (32 - r))) : x; }

ATCG_ALWAYS_INLINE uint32_t bit_reverse32(uint32_t x) {
    x = ((x & 0x55555555u) << 1) | ((x >> 1) & 0x55555555u);
    x = ((x & 0x33333333u) << 2) | ((x >> 2) & 0x33333333u);
    x = ((x & 0x0F0F0F0Fu) << 4) | ((x >> 4) & 0x0F0F0F0Fu);
    x = ((x & 0x00FF00FFu) << 8) | ((x >> 8) & 0x00FF00FFu);
    return (x << 16) | (x >> 16);
}

ATCG_ALWAYS_INLINE uint32_t mix32(uint32_t x) {
    x ^= x >> 16; x *= 0x7FEB352Du;
    x ^= x >> 15; x *= 0x846CA68Bu;
    x ^= x >> 16; return x;
}

ATCG_ALWAYS_INLINE uint32_t load32_le(const uint8_t *p) {
    return ((uint32_t)p[0]) | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

ATCG_ALWAYS_INLINE void store32_le(uint8_t *p, uint32_t x) {
    p[0] = (uint8_t)x; p[1] = (uint8_t)(x >> 8); p[2] = (uint8_t)(x >> 16); p[3] = (uint8_t)(x >> 24);
}

static void load_state(uint32_t s[ATCG_WORDS], const uint8_t in[SPIRAL_ATCG_BLOCK_BYTES]) {
    for (int i = 0; i < ATCG_WORDS; ++i) s[i] = load32_le(in + 4 * i);
}

static void store_state(uint8_t out[SPIRAL_ATCG_BLOCK_BYTES], const uint32_t s[ATCG_WORDS]) {
    for (int i = 0; i < ATCG_WORDS; ++i) store32_le(out + 4 * i, s[i]);
}

static uint32_t domain_of_policy(spiral_atcg_policy_t p) {
    switch (p) {
        case SPIRAL_ATCG_POLICY_BASE: return 0x42415345u; /* BASE */
        case SPIRAL_ATCG_POLICY_SAFE: return 0x53414645u; /* SAFE */
        case SPIRAL_ATCG_POLICY_AGGR: return 0x41474752u; /* AGGR */
        case SPIRAL_ATCG_POLICY_NULL: return 0x4E554C4Cu; /* NULL */
        default: return 0u;
    }
}

static int valid_policy(spiral_atcg_policy_t p) { return p >= SPIRAL_ATCG_POLICY_BASE && p <= SPIRAL_ATCG_POLICY_NULL; }

const char *spiral_atcg_policy_name(spiral_atcg_policy_t policy) {
    switch (policy) {
        case SPIRAL_ATCG_POLICY_BASE: return "BASE";
        case SPIRAL_ATCG_POLICY_SAFE: return "SAFE";
        case SPIRAL_ATCG_POLICY_AGGR: return "AGGR";
        case SPIRAL_ATCG_POLICY_NULL: return "NULL";
        default: return "INVALID";
    }
}

/* Reference deterministic byte-domain KDF for engineering tests. Replace with the production MCMF/KDF if needed. */
static uint32_t kdf_word(const uint8_t *buf, size_t len, uint32_t domain, uint32_t r, uint32_t lane, uint32_t slot) {
    uint32_t h = 0x811C9DC5u ^ domain ^ (r * 0x9E3779B9u) ^ (lane * 0x85EBCA6Bu) ^ (slot * 0xC2B2AE35u);
    for (size_t i = 0; i < len; ++i) {
        h ^= (uint32_t)buf[i] + (uint32_t)i * 0x01000193u;
        h *= 0x01000193u;
        h = rotl32(h, 5) ^ (h >> 7);
    }
    h ^= bit_reverse32(domain + r + lane + slot);
    return mix32(h);
}

static void build_root_main(uint8_t root[64], const uint8_t master_seed[32]) {
    memcpy(root, master_seed, 32);
    memcpy(root + 32, "SPIRAL.ATCG.STRAND.MAIN.v2", 27);
    memset(root + 59, 0, 5);
}

static void build_root_var(uint8_t root[96], const uint8_t master_seed[32], const uint8_t nonce[16], const uint8_t session_id[16]) {
    memcpy(root, master_seed, 32);
    memcpy(root + 32, nonce, 16);
    memcpy(root + 48, session_id, 16);
    memcpy(root + 64, "SPIRAL.ATCG.STRAND.VAR.v2", 26);
    memset(root + 90, 0, 6);
}

static void expand_tokenset(spiral_atcg_tokenset_t *ts, const uint8_t *root, size_t root_len, uint32_t r, uint32_t strand_domain, spiral_atcg_policy_t policy) {
    const uint32_t pd = domain_of_policy(policy);
    for (uint32_t i = 0; i < SPIRAL_ATCG_WORDS; ++i) {
        uint32_t base = kdf_word(root, root_len, strand_domain ^ pd, r, i, 0);
        ts->A[i] = mix32(base ^ 0xA11C6A37u ^ rotl32(pd, (int)i));
        ts->T[i] = mix32(base ^ 0x7A7157EDu ^ bit_reverse32(ts->A[(i + SPIRAL_ATCG_WORDS - 1) % SPIRAL_ATCG_WORDS]));
        ts->C[i] = mix32(base ^ 0xC011CA7Eu ^ rotl32(ts->T[i], (int)((r + i) | 1u)));
        ts->G[i] = mix32(base ^ 0x6A7E5A1Eu ^ rotl32(ts->C[(i + 3) % SPIRAL_ATCG_WORDS], (int)((i * 3u + r + 1u) & 31u)));
        if (policy == SPIRAL_ATCG_POLICY_SAFE) {
            ts->T[i] ^= bit_reverse32(ts->A[i] ^ 0x54414950u);
            ts->G[i] ^= mix32(ts->C[i] + 0x43475052u + r + i);
        } else if (policy == SPIRAL_ATCG_POLICY_AGGR) {
            ts->A[i] = mix32(ts->A[i] ^ rotl32(ts->G[i], (int)(i + 5)));
            ts->T[i] = bit_reverse32(mix32(ts->T[i] + ts->C[(i + 5) % SPIRAL_ATCG_WORDS]));
            ts->C[i] ^= rotl32(mix32(ts->A[i] ^ ts->T[i]), (int)((r + i * 7u) & 31u));
            ts->G[i] += mix32(ts->C[i] ^ ts->T[(i + 1) % SPIRAL_ATCG_WORDS]);
        }
    }
}

spiral_status_t spiral_atcg_project(const uint8_t master_seed[32], const uint8_t nonce[16], const uint8_t session_id[16], uint32_t rounds, spiral_atcg_policy_t policy, spiral_atcg_strands_t *out) {
    if (!master_seed || !nonce || !session_id || !out) return SPIRAL_ATCG_ERR_NULL;
    if (rounds == 0 || rounds > SPIRAL_ATCG_MAX_ROUNDS) return SPIRAL_ATCG_ERR_ROUNDS;
    if (!valid_policy(policy)) return SPIRAL_ATCG_ERR_POLICY;

    memset(out, 0, sizeof(*out));
    out->rounds = rounds;
    out->policy = policy;

    uint8_t root_main[64];
    uint8_t root_var[96];
    build_root_main(root_main, master_seed);
    build_root_var(root_var, master_seed, nonce, session_id);

    for (uint32_t r = 0; r < rounds; ++r) {
        expand_tokenset(&out->main_strand[r], root_main, sizeof(root_main), r, 0x4D41494Eu, policy);
        expand_tokenset(&out->var_strand [r], root_var,  sizeof(root_var),  r, 0x56415220u, policy);
    }
    return SPIRAL_ATCG_OK;
}

spiral_status_t spiral_atcg_pair(const spiral_atcg_strands_t *strands, uint32_t r, spiral_atcg_pair_t *out_pair) {
    if (!strands || !out_pair) return SPIRAL_ATCG_ERR_NULL;
    if (strands->rounds == 0 || strands->rounds > SPIRAL_ATCG_MAX_ROUNDS || r >= strands->rounds) return SPIRAL_ATCG_ERR_ROUNDS;
    uint32_t j = strands->rounds - 1u - r;
    out_pair->x = &strands->main_strand[r];
    out_pair->y = &strands->var_strand[j];
    out_pair->round = r;
    out_pair->paired_round = j;
    return SPIRAL_ATCG_OK;
}

/* Pair-symmetric extraction: swap(x,y) gives the same extracted token. */
static uint32_t pair_sym(uint32_t a, uint32_t b, uint32_t tag, uint32_t i) {
    uint32_t x = a ^ b;
    uint32_t y = rotl32(a + b + tag, (int)((i * 7u + 3u) & 31u));
    uint32_t z = rotl32(a, (int)((i + 5u) & 31u)) ^ rotl32(b, (int)((i + 5u) & 31u));
    return mix32(x ^ y ^ z ^ bit_reverse32(tag + i));
}

static uint32_t compose_tc(uint32_t t, uint32_t c, uint32_t r, uint32_t i, spiral_atcg_policy_t policy) {
    uint32_t x = mix32(t ^ rotl32(c, (int)((r + i + 1u) & 31u)) ^ 0x54435F76u);
    if (policy == SPIRAL_ATCG_POLICY_SAFE) x ^= bit_reverse32(t + c + 0x5AFE0001u);
    if (policy == SPIRAL_ATCG_POLICY_AGGR) x = mix32(bit_reverse32(x) + rotl32(t ^ c, (int)((r * 3u + i * 5u) & 31u)));
    return x;
}

static uint32_t compose_ca(uint32_t c, uint32_t a, uint32_t r, uint32_t i, spiral_atcg_policy_t policy) {
    uint32_t x = mix32(c + rotl32(a ^ 0xCA5CADEu, (int)((r + i * 3u + 1u) & 31u)));
    if (policy == SPIRAL_ATCG_POLICY_SAFE) x += mix32(c ^ a ^ 0x5AFE0002u ^ i);
    if (policy == SPIRAL_ATCG_POLICY_AGGR) x ^= bit_reverse32(mix32(x ^ c ^ rotl32(a, (int)((i + 11u) & 31u))));
    return x;
}

static int complement_check(const spiral_atcg_pair_t *pair) {
    uint32_t acc = 0;
    for (uint32_t i = 0; i < SPIRAL_ATCG_WORDS; ++i) {
        uint32_t at = pair_sym(pair->x->A[i], pair->y->T[i], 0x41544F4Bu, i);
        uint32_t cg = pair_sym(pair->x->C[i], pair->y->G[i], 0x43474F4Bu, i);
        acc ^= (at ^ rotl32(cg, (int)i));
    }
    /* Reference SAFE compatibility: deterministic but deliberately non-fatal for most inputs. */
    return (mix32(acc) != 0u);
}

spiral_status_t spiral_atcg_transcribe_pair(const spiral_atcg_pair_t *pair, spiral_atcg_policy_t policy, spiral_round_material_t *out) {
    if (!pair || !out) return SPIRAL_ATCG_ERR_NULL;
    if (!valid_policy(policy)) return SPIRAL_ATCG_ERR_POLICY;
    if (policy == SPIRAL_ATCG_POLICY_SAFE && !complement_check(pair)) return SPIRAL_ATCG_ERR_COMPAT;

    for (uint32_t i = 0; i < SPIRAL_ATCG_WORDS; ++i) {
        uint32_t A = pair_sym(pair->x->A[i], pair->y->A[i], 0x414E4348u, i); /* Anchor */
        uint32_t T = pair_sym(pair->x->T[i], pair->y->T[i], 0x54574953u, i); /* Twist */
        uint32_t C = pair_sym(pair->x->C[i], pair->y->C[i], 0x434F5550u, i); /* Couple */
        uint32_t G = pair_sym(pair->x->G[i], pair->y->G[i], 0x47415445u, i); /* Gate */

        uint32_t TC = compose_tc(T, C, pair->round, i, policy);
        uint32_t CA = compose_ca(C, A, pair->round, i, policy);

        /* Production rules: LM := A, RC := T∘C, CD := C∘A, SK := G. */
        out->LM[i] = mix32(A ^ 0x4C4D5F76u ^ i);
        out->RC[i] = mix32(TC ^ 0x52435F76u ^ pair->round);
        out->CD[i] = mix32(CA ^ 0x43445F76u ^ (pair->paired_round << 1));
        out->SK[i] = mix32(G ^ 0x534B5F76u ^ rotl32(pair->round + i, (int)(i + 1)));

        if (policy == SPIRAL_ATCG_POLICY_NULL) {
            uint32_t collapsed = mix32(out->LM[i] ^ out->RC[i] ^ out->CD[i] ^ out->SK[i]);
            out->LM[i] = collapsed;
            out->RC[i] = 0;
            out->CD[i] = 0;
            out->SK[i] = 0;
        }
    }
    return SPIRAL_ATCG_OK;
}

spiral_status_t spiral_atcg_transcribe_round(const spiral_atcg_strands_t *strands, uint32_t r, spiral_round_material_t *out) {
    if (!strands || !out) return SPIRAL_ATCG_ERR_NULL;
    spiral_atcg_pair_t pair;
    spiral_status_t st = spiral_atcg_pair(strands, r, &pair);
    if (st != SPIRAL_ATCG_OK) return st;
    return spiral_atcg_transcribe_pair(&pair, strands->policy, out);
}

spiral_status_t spiral_atcg_precompute_materials(const spiral_atcg_strands_t *strands, spiral_round_material_t materials[SPIRAL_ATCG_MAX_ROUNDS]) {
    if (!strands || !materials) return SPIRAL_ATCG_ERR_NULL;
    if (strands->rounds == 0 || strands->rounds > SPIRAL_ATCG_MAX_ROUNDS) return SPIRAL_ATCG_ERR_ROUNDS;
    for (uint32_t r = 0; r < strands->rounds; ++r) {
        spiral_status_t st = spiral_atcg_transcribe_round(strands, r, &materials[r]);
        if (st != SPIRAL_ATCG_OK) return st;
    }
    return SPIRAL_ATCG_OK;
}

/* =====================
 * v2.2-strength substrate helpers
 *
 * Keeps ATCG v2 transcription unchanged, but uses a Spiral-DNA-320
 * v1-inspired reversible half-state substrate with round-dependent
 * dual-input expansion, F1/F2 stride permutations, SpiralMix_v2-style
 * butterfly diffusion, and output twist.
 * ===================== */

static void material_k(uint32_t k[5], const spiral_round_material_t *m, uint32_t r) {
    for (int i = 0; i < 5; ++i) {
        uint32_t x = m->SK[i] ^ rotl32(m->LM[(i + 5) % ATCG_WORDS], (int)((r + i + 1u) & 31u));
        x += m->RC[(i * 3 + (int)r) % ATCG_WORDS] ^ bit_reverse32(m->CD[(i + 7) % ATCG_WORDS]);
        x ^= mix32(m->SK[(i + 5) % ATCG_WORDS] + m->CD[(i + 2) % ATCG_WORDS] + 0x9E3779B9u + r + (uint32_t)i);
        k[i] = mix32(x);
    }
}

static void fill_round_cache(spiral_round_cache_t *cache, const spiral_round_material_t *m, uint32_t r) {
    material_k(cache->k, m, r);
    for (int i = 0; i < 5; ++i) {
        cache->s_xor[i] = m->LM[i] ^ rotl32(m->SK[i + 5], (int)((r + (uint32_t)i + 1u) & 31u));
        cache->alpha[i] = (uint8_t)((i + 1 + (int)r) % 5);
        cache->beta[i]  = (uint8_t)((i + 3 + (int)r) % 5);
        cache->rot_a[i] = (uint8_t)((((int)r * 3 + i * 5 + 1) & 31) | 1);
        cache->rot_b[i] = (uint8_t)((((int)r * 5 + i * 3 + 7) & 31) | 1);
    }
}

static void atcg_dual_input(uint32_t X[5], const uint32_t S[5], const spiral_round_material_t *m, uint32_t r) {
    uint32_t rs = 0x9E3779B9u ^ (uint32_t)(r * 0x10001u) ^ m->RC[r % ATCG_WORDS];
    for (int i = 0; i < 5; ++i) {
        uint32_t s0 = S[i];
        uint32_t s1 = S[(i + 1 + (int)r) % 5];
        uint32_t s2 = S[(i + 2) % 5];
        uint32_t s4 = S[(i + 4 + (int)r) % 5];
        uint32_t x = s0 ^ m->LM[(i + (int)r) % ATCG_WORDS];
        x ^= rotl32(s1 ^ m->SK[i], ((int)r + i * 5 + 1) | 1);
        x += rotl32((s2 ^ rs) + m->RC[(i + 3) % ATCG_WORDS], ((int)r + i * 7 + 3) | 1);
        x ^= mix32(s4 ^ rs ^ (uint32_t)i ^ m->CD[(i + 5) % ATCG_WORDS]);
        X[i] = x;
    }
}

static void atcg_F1(uint32_t out[5], const uint32_t in[5], const uint32_t k[5], const spiral_round_material_t *m, uint32_t r) {
    for (int i = 0; i < 5; ++i) {
        uint32_t x  = in[(i * 3 + (int)r) % 5];
        uint32_t x2 = in[(i + 2) % 5];
        uint32_t x4 = in[(i + 4) % 5];
        x += k[i] ^ rotl32(x2 ^ m->RC[(i + 1) % ATCG_WORDS], (((int)r + i) & 31) | 1);
        int rot = ((x ^ (uint32_t)r ^ x4 ^ m->CD[(i + 4) % ATCG_WORDS]) & 31u) | 1;
        x = rotl32(x, rot);
        x ^= (x * 0x7F4A7C15u);
        x += rotl32((x4 ^ x2) + m->LM[(i + 2) % ATCG_WORDS], ((i * 7 + (int)r) & 31) | 1);
        x = (x + rotl32(x, 9)) ^ rotl32(x, 3);
        x ^= mix32(x ^ k[(i + 1) % 5] ^ m->SK[(i + 6) % ATCG_WORDS]);
        out[i] = x;
    }
}

static void atcg_F2(uint32_t out[5], const uint32_t in[5], const uint32_t k[5], const spiral_round_material_t *m, uint32_t r) {
    for (int i = 0; i < 5; ++i) {
        uint32_t x  = in[(i * 3 + 2 * (int)r) % 5];
        uint32_t x1 = in[(i + 1) % 5];
        uint32_t x3 = in[(i + 3) % 5];
        x ^= k[i] + rotl32(x1 ^ m->LM[(i + 8) % ATCG_WORDS], (((int)r + i * 3) & 31) | 1);
        int rot = ((x + r * 13u + x3 + m->RC[(i + 5) % ATCG_WORDS]) & 31u) | 1;
        x = rotl32(x, rot);
        uint32_t y = x + rotl32(bit_reverse32(~(x ^ x1 ^ m->CD[(i + 1) % ATCG_WORDS])), 7);
        y ^= rotl32(x ^ x3 ^ m->SK[(i + 4) % ATCG_WORDS], 3);
        x = y;
        x ^= (x * 0x9E3779B1u);
        x = (x + rotl32(x, 11)) ^ rotl32(x, 17);
        x ^= (x >> 7);
        x += rotl32(x ^ x1 ^ m->RC[(i + 9) % ATCG_WORDS], 13);
        x ^= mix32(x ^ k[(i + 2) % 5] ^ m->LM[(i + 3) % ATCG_WORDS]);
        out[i] = x;
    }
}

static void atcg_spiral_mix_v2(uint32_t T[5], const spiral_round_material_t *m, uint32_t r) {
    uint32_t rs = 0x9E3779B9u ^ (uint32_t)(r * 0x10001u) ^ m->CD[(r + 3u) % ATCG_WORDS];
    T[0] ^= rotl32(rs ^ m->LM[0], 1);
    T[1] += rotl32(rs ^ m->LM[1], 5);
    T[2] ^= rotl32(rs ^ m->LM[2], 9);
    T[3] += rotl32(rs ^ m->LM[3], 13);
    T[4] ^= rotl32(rs ^ m->LM[4], 17);
    uint32_t p0 = T[0] ^ rotl32(T[1] ^ m->RC[0], (((int)r + 3) & 31) | 1);
    uint32_t p1 = T[1] + rotl32(T[2] ^ m->RC[1], (((int)r + 5) & 31) | 1);
    uint32_t p2 = T[2] ^ rotl32(T[3] ^ m->RC[2], (((int)r + 7) & 31) | 1);
    uint32_t p3 = T[3] + rotl32(T[4] ^ m->RC[3], (((int)r + 11) & 31) | 1);
    uint32_t p4 = T[4] ^ rotl32(T[0] ^ m->RC[4], (((int)r + 13) & 31) | 1);
    uint32_t a0 = p0 + rotl32((p3 ^ rs) + m->CD[0], (((int)r + 1) & 31) | 1);
    uint32_t a1 = p1 ^ rotl32((p4 + rs) ^ m->CD[1], (((int)r + 4) & 31) | 1);
    uint32_t a2 = p2 + rotl32((p0 ^ rs) + m->CD[2], (((int)r + 7) & 31) | 1);
    uint32_t a3 = p3 ^ rotl32((p1 + rs) ^ m->CD[3], (((int)r + 10) & 31) | 1);
    uint32_t a4 = p4 + rotl32((p2 ^ rs) + m->CD[4], (((int)r + 13) & 31) | 1);
    uint32_t b0 = a0 ^ mix32((a2 + rotl32(a4 ^ m->SK[0], (((int)r + 9) & 31) | 1)) ^ rs);
    uint32_t b1 = a1 + mix32((a3 ^ rotl32(a0 ^ m->SK[1], (((int)r + 12) & 31) | 1)) + rs);
    uint32_t b2 = a2 ^ mix32((a4 + rotl32(a1 ^ m->SK[2], (((int)r + 15) & 31) | 1)) ^ rs);
    uint32_t b3 = a3 + mix32((a0 ^ rotl32(a2 ^ m->SK[3], (((int)r + 18) & 31) | 1)) + rs);
    uint32_t b4 = a4 ^ mix32((a1 + rotl32(a3 ^ m->SK[4], (((int)r + 21) & 31) | 1)) ^ rs);
    uint32_t U[5];
    int sh = (int)(r % 5u);
    U[(0 + sh) % 5] = b0 ^ rotl32(b2, (((int)r + 3) & 31) | 1);
    U[(2 + sh) % 5] = b1 + rotl32(b3, (((int)r + 7) & 31) | 1);
    U[(4 + sh) % 5] = b2 ^ rotl32(b4, (((int)r + 11) & 31) | 1);
    U[(1 + sh) % 5] = b3 + rotl32(b0, (((int)r + 15) & 31) | 1);
    U[(3 + sh) % 5] = b4 ^ rotl32(b1, (((int)r + 19) & 31) | 1);
    T[0] = U[0] ^ mix32(U[3] + rs + m->LM[5]);
    T[1] = U[1] + mix32((U[4] ^ rs) + m->LM[6]);
    T[2] = U[2] ^ mix32(U[0] + rs + m->LM[7]);
    T[3] = U[3] + mix32((U[1] ^ rs) + m->LM[8]);
    T[4] = U[4] ^ mix32(U[2] + rs + m->LM[9]);
}

static inline int atcg_alpha(int i, uint32_t r) { return (i + 1 + (int)r) % 5; }
static inline int atcg_beta (int i, uint32_t r) { return (i + 3 + (int)r) % 5; }
static inline int atcg_rot_a(int i, uint32_t r) { return (((int)r * 3 + i * 5 + 1) & 31) | 1; }
static inline int atcg_rot_b(int i, uint32_t r) { return (((int)r * 5 + i * 3 + 7) & 31) | 1; }

spiral_status_t spiral_atcg_init_session(spiral_atcg_ctx_t *ctx, const uint8_t master_seed[32], const uint8_t nonce[16], const uint8_t session_id[16], uint32_t rounds, spiral_atcg_policy_t policy) {
    if (!ctx) return SPIRAL_ATCG_ERR_NULL;
    memset(ctx, 0, sizeof(*ctx));
    spiral_status_t st = spiral_atcg_project(master_seed, nonce, session_id, rounds, policy, &ctx->strands);
    if (st != SPIRAL_ATCG_OK) return st;
    st = spiral_atcg_precompute_materials(&ctx->strands, ctx->materials);
    if (st != SPIRAL_ATCG_OK) return st;
    ctx->rounds = rounds;
    for (uint32_t r = 0; r < rounds; ++r) {
        fill_round_cache(&ctx->round_cache[r], &ctx->materials[r], r);
    }
    ctx->policy = policy;
    return SPIRAL_ATCG_OK;
}

static void spiral_round(uint32_t s[ATCG_WORDS], const spiral_round_material_t *m, const spiral_round_cache_t *cache, uint32_t r) {
    uint32_t L[5], Rr[5], S[5], X[5], f1[5], f2[5], T[5];
    uint32_t YL[5], YR[5], NL[5], NR[5];
    for (int i = 0; i < 5; ++i) { L[i] = s[i]; Rr[i] = s[i + 5]; S[i] = L[i] ^ Rr[i]; }
    for (int i = 0; i < 5; ++i) S[i] ^= cache->s_xor[i];
    atcg_dual_input(X, S, m, r);
    atcg_F1(f1, X, cache->k, m, r);
    atcg_F2(f2, X, cache->k, m, r);
    for (int i = 0; i < 5; ++i) {
        T[i] = f1[i] ^ f2[i] ^ m->CD[(i + (int)r) % ATCG_WORDS];
        T[i] ^= rotl32(T[i], ((T[i] ^ r ^ m->RC[i]) & 31u) | 1u);
        T[i] += rotl32(T[i] ^ m->SK[(i + 2) % ATCG_WORDS], (((int)r + i * 7) & 31) | 1);
    }
    atcg_spiral_mix_v2(T, m, r);
    for (int i = 0; i < 5; ++i) { YL[i] = L[i] ^ T[i]; YR[i] = Rr[i] ^ T[i]; }
    for (int i = 0; i < 5; ++i) {
        NL[i] = rotl32(YR[cache->alpha[i]], cache->rot_a[i]);
        NR[i] = rotr32(YL[cache->beta[i]], cache->rot_b[i]);
    }
    for (int i = 0; i < 5; ++i) { s[i] = NL[i]; s[i + 5] = NR[i]; }
}

static void inv_spiral_round(uint32_t s[ATCG_WORDS], const spiral_round_material_t *m, const spiral_round_cache_t *cache, uint32_t r) {
    uint32_t L[5], Rr[5], YL[5], YR[5], S[5], X[5], f1[5], f2[5], T[5];
    for (int i = 0; i < 5; ++i) { L[i] = s[i]; Rr[i] = s[i + 5]; }
    for (int i = 0; i < 5; ++i) {
        uint8_t ja = cache->alpha[i];
        uint8_t jb = cache->beta[i];
        YR[ja] = rotr32(L[i], cache->rot_a[i]);
        YL[jb] = rotl32(Rr[i], cache->rot_b[i]);
    }
    for (int i = 0; i < 5; ++i) {
        S[i] = YL[i] ^ YR[i];
        S[i] ^= cache->s_xor[i];
    }
    atcg_dual_input(X, S, m, r);
    atcg_F1(f1, X, cache->k, m, r);
    atcg_F2(f2, X, cache->k, m, r);
    for (int i = 0; i < 5; ++i) {
        T[i] = f1[i] ^ f2[i] ^ m->CD[(i + (int)r) % ATCG_WORDS];
        T[i] ^= rotl32(T[i], ((T[i] ^ r ^ m->RC[i]) & 31u) | 1u);
        T[i] += rotl32(T[i] ^ m->SK[(i + 2) % ATCG_WORDS], (((int)r + i * 7) & 31) | 1);
    }
    atcg_spiral_mix_v2(T, m, r);
    for (int i = 0; i < 5; ++i) { s[i] = YL[i] ^ T[i]; s[i + 5] = YR[i] ^ T[i]; }
}


/* Fixed 16-round fast scalar dispatch.
 * This is bit-exact: it removes the generic round loop for the default
 * 16-round profile and keeps the v2.2-strength round function unchanged.
 */
#define ATCG_ROUND_ENC(S, R) spiral_round((S), &ctx->materials[(R)], &ctx->round_cache[(R)], (R))
#define ATCG_ROUND_DEC(S, R) inv_spiral_round((S), &ctx->materials[(R)], &ctx->round_cache[(R)], (R))

static void encrypt_block16_fast_scalar(const spiral_atcg_ctx_t *ctx, const uint8_t *plaintext, uint8_t *ciphertext) {
    uint32_t s[ATCG_WORDS];
    load_state(s, plaintext);
    ATCG_ROUND_ENC(s, 0);  ATCG_ROUND_ENC(s, 1);  ATCG_ROUND_ENC(s, 2);  ATCG_ROUND_ENC(s, 3);
    ATCG_ROUND_ENC(s, 4);  ATCG_ROUND_ENC(s, 5);  ATCG_ROUND_ENC(s, 6);  ATCG_ROUND_ENC(s, 7);
    ATCG_ROUND_ENC(s, 8);  ATCG_ROUND_ENC(s, 9);  ATCG_ROUND_ENC(s, 10); ATCG_ROUND_ENC(s, 11);
    ATCG_ROUND_ENC(s, 12); ATCG_ROUND_ENC(s, 13); ATCG_ROUND_ENC(s, 14); ATCG_ROUND_ENC(s, 15);
    store_state(ciphertext, s);
}

static void decrypt_block16_fast_scalar(const spiral_atcg_ctx_t *ctx, const uint8_t *ciphertext, uint8_t *plaintext) {
    uint32_t s[ATCG_WORDS];
    load_state(s, ciphertext);
    ATCG_ROUND_DEC(s, 15); ATCG_ROUND_DEC(s, 14); ATCG_ROUND_DEC(s, 13); ATCG_ROUND_DEC(s, 12);
    ATCG_ROUND_DEC(s, 11); ATCG_ROUND_DEC(s, 10); ATCG_ROUND_DEC(s, 9);  ATCG_ROUND_DEC(s, 8);
    ATCG_ROUND_DEC(s, 7);  ATCG_ROUND_DEC(s, 6);  ATCG_ROUND_DEC(s, 5);  ATCG_ROUND_DEC(s, 4);
    ATCG_ROUND_DEC(s, 3);  ATCG_ROUND_DEC(s, 2);  ATCG_ROUND_DEC(s, 1);  ATCG_ROUND_DEC(s, 0);
    store_state(plaintext, s);
}

#undef ATCG_ROUND_ENC
#undef ATCG_ROUND_DEC

static void encrypt_block_generic_scalar(const spiral_atcg_ctx_t *ctx, const uint8_t *plaintext, uint8_t *ciphertext) {
    uint32_t s[ATCG_WORDS];
    load_state(s, plaintext);
    for (uint32_t r = 0; r < ctx->rounds; ++r) {
        spiral_round(s, &ctx->materials[r], &ctx->round_cache[r], r);
    }
    store_state(ciphertext, s);
}

static void decrypt_block_generic_scalar(const spiral_atcg_ctx_t *ctx, const uint8_t *ciphertext, uint8_t *plaintext) {
    uint32_t s[ATCG_WORDS];
    load_state(s, ciphertext);
    for (uint32_t rr = ctx->rounds; rr > 0; --rr) {
        uint32_t r = rr - 1u;
        inv_spiral_round(s, &ctx->materials[r], &ctx->round_cache[r], r);
    }
    store_state(plaintext, s);
}

spiral_status_t spiral_atcg_encrypt_block(const spiral_atcg_ctx_t *ctx, const uint8_t plaintext[40], uint8_t ciphertext[40]) {
    if (!ctx || !plaintext || !ciphertext) return SPIRAL_ATCG_ERR_NULL;
    if (ctx->rounds == 0 || ctx->rounds > SPIRAL_ATCG_MAX_ROUNDS) return SPIRAL_ATCG_ERR_ROUNDS;
    if (ctx->rounds == SPIRAL_ATCG_DEFAULT_ROUNDS) encrypt_block16_fast_scalar(ctx, plaintext, ciphertext);
    else encrypt_block_generic_scalar(ctx, plaintext, ciphertext);
    return SPIRAL_ATCG_OK;
}

spiral_status_t spiral_atcg_decrypt_block(const spiral_atcg_ctx_t *ctx, const uint8_t ciphertext[40], uint8_t plaintext[40]) {
    if (!ctx || !ciphertext || !plaintext) return SPIRAL_ATCG_ERR_NULL;
    if (ctx->rounds == 0 || ctx->rounds > SPIRAL_ATCG_MAX_ROUNDS) return SPIRAL_ATCG_ERR_ROUNDS;
    if (ctx->rounds == SPIRAL_ATCG_DEFAULT_ROUNDS) decrypt_block16_fast_scalar(ctx, ciphertext, plaintext);
    else decrypt_block_generic_scalar(ctx, ciphertext, plaintext);
    return SPIRAL_ATCG_OK;
}

spiral_status_t spiral_atcg_encrypt_blocks(const spiral_atcg_ctx_t *ctx, const uint8_t *plaintext_blocks, uint8_t *ciphertext_blocks, size_t blocks) {
    if (!ctx || (!plaintext_blocks && blocks) || (!ciphertext_blocks && blocks)) return SPIRAL_ATCG_ERR_NULL;
    if (ctx->rounds == 0 || ctx->rounds > SPIRAL_ATCG_MAX_ROUNDS) return SPIRAL_ATCG_ERR_ROUNDS;
    for (size_t b = 0; b < blocks; ++b) {
        const uint8_t *in = plaintext_blocks + b * SPIRAL_ATCG_BLOCK_BYTES;
        uint8_t *out = ciphertext_blocks + b * SPIRAL_ATCG_BLOCK_BYTES;
        if (ctx->rounds == SPIRAL_ATCG_DEFAULT_ROUNDS) encrypt_block16_fast_scalar(ctx, in, out);
        else encrypt_block_generic_scalar(ctx, in, out);
    }
    return SPIRAL_ATCG_OK;
}

spiral_status_t spiral_atcg_decrypt_blocks(const spiral_atcg_ctx_t *ctx, const uint8_t *ciphertext_blocks, uint8_t *plaintext_blocks, size_t blocks) {
    if (!ctx || (!ciphertext_blocks && blocks) || (!plaintext_blocks && blocks)) return SPIRAL_ATCG_ERR_NULL;
    if (ctx->rounds == 0 || ctx->rounds > SPIRAL_ATCG_MAX_ROUNDS) return SPIRAL_ATCG_ERR_ROUNDS;
    for (size_t b = 0; b < blocks; ++b) {
        const uint8_t *in = ciphertext_blocks + b * SPIRAL_ATCG_BLOCK_BYTES;
        uint8_t *out = plaintext_blocks + b * SPIRAL_ATCG_BLOCK_BYTES;
        if (ctx->rounds == SPIRAL_ATCG_DEFAULT_ROUNDS) decrypt_block16_fast_scalar(ctx, in, out);
        else decrypt_block_generic_scalar(ctx, in, out);
    }
    return SPIRAL_ATCG_OK;
}
