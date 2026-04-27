# Spiral-ATCG v2.2.1 Zero-Risk C Cleanup

Version: v2.2.1-zero-risk-cleanup  
Base: v2.2-strength  
Scope: implementation-only performance cleanup

## Goal

This revision keeps the v2.2-strength algorithm, ATCG v2.0 transcription framework, 16-round profile, and KAT behavior unchanged.

The goal is to move material-only work out of the per-block hot path without changing the round equations.

## Changes

### 1. Per-round hot-path cache

`spiral_atcg_ctx_t` now contains:

```c
spiral_round_cache_t round_cache[SPIRAL_ATCG_MAX_ROUNDS];
```

The cache stores values fully determined by `(round_material[r], r)`:

```c
uint32_t k[5];
uint32_t s_xor[5];
uint8_t  alpha[5];
uint8_t  beta[5];
uint8_t  rot_a[5];
uint8_t  rot_b[5];
```

These are computed once in `spiral_atcg_init_session()`.

### 2. Cached values used by encrypt/decrypt

`spiral_round()` and `inv_spiral_round()` now consume:

```c
const spiral_round_material_t *m,
const spiral_round_cache_t *cache,
uint32_t r
```

The mathematical operations are unchanged. The same values that were previously computed inside each block are now read from the session cache.

### 3. Inline utility helpers

`rotl32`, `rotr32`, `bit_reverse32`, `mix32`, `load32_le`, and `store32_le` are marked with an internal `ATCG_ALWAYS_INLINE` macro where supported by the compiler.

### 4. Makefile target hygiene

`make all` now builds the normal edit/test targets only. Heavy probes are opt-in:

```bash
make full
make thorough
make qres
```

This avoids rebuilding long-running research probes during normal development cycles.

## Non-changes

No change was made to:

- ATCG alphabet / production rules
- ATCG_Project / Pair / TranscribeRound
- BASE / SAFE / AGGR / NULL policy semantics
- SpiralRound equations
- InvSpiralRound equations
- Round count
- KAT generation logic
- QRES methodology

## Validation

Reference `-O0` build:

```text
ROUNDTRIP PASSED policy=BASE / SAFE / AGGR / NULL
PAIR SYMMETRY PASSED policy=BASE / SAFE / AGGR / NULL
ALL AVALANCHE TESTS PASSED
ALL 20 KAT ENTRIES PASSED ROUNDTRIP
ALL 5 SCENARIOS PASSED POLICY-DISTINCTNESS CHECK
```

Thorough avalanche:

```text
[BASE]  avg_hd=159.94  all-10-lanes=100.00%  zero-diff=0
[SAFE]  avg_hd=159.81  all-10-lanes=100.00%  zero-diff=0
[AGGR]  avg_hd=159.78  all-10-lanes=100.00%  zero-diff=0
[NULL]  avg_hd=160.05  all-10-lanes=100.00%  zero-diff=0
```

## High-resolution benchmark, reference `-O0`

```text
Spiral-ATCG 2.2.1-zero-risk-cleanup high-resolution benchmark
rounds=16 block=40 data_blocks=5000 setup_iters=100 warmup_blocks=500

[BASE] encrypt 2.269 MiB/s, decrypt 2.280 MiB/s
[SAFE] encrypt 2.265 MiB/s, decrypt 2.256 MiB/s
[AGGR] encrypt 2.268 MiB/s, decrypt 2.232 MiB/s
[NULL] encrypt 2.245 MiB/s, decrypt 2.160 MiB/s
```

Compared with the previous v2.2-strength `-O0` reference benchmark, this is roughly a 25% to 65% improvement depending on policy and direction, while keeping the same safety framing.
