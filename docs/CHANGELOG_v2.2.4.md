# Spiral-ATCG v2.2.4 Full Scalar Cleanup

Status: implementation-only optimization.

This revision keeps the v2.2-strength cipher semantics unchanged and preserves all known-answer ciphertexts from `docs/spiral_atcg_v22_kat_v1.md`.

## Changes

- Unrolled `load_state()` and `store_state()` to remove the per-block 10-word loop.
- Removed the remaining `U[5]` temporary array from `atcg_spiral_mix_v2()` and replaced it with scalar `u0..u4` mapping selected by `r % 5`.
- Kept hot-path helpers compiler-friendly; only low-level rotate/load/store helpers remain inline to avoid GCC compile-time blowups on large round bodies.
- Kept the generic variable-round fallback.

## Non-changes

- ATCG Project / Pair / Transcribe pipeline is unchanged.
- Production rules `LM := A`, `RC := T ∘ C`, `CD := C ∘ A`, `SK := G` are unchanged.
- v2.2-strength substrate math is unchanged.
- 16-round profile and all policy definitions are unchanged.

## Validation

Required checks passed:

```text
ROUNDTRIP PASSED policy=BASE / SAFE / AGGR / NULL
PAIR SYMMETRY PASSED policy=BASE / SAFE / AGGR / NULL
ALL AVALANCHE TESTS PASSED
BATCH API PASSED policy=BASE / SAFE / AGGR / NULL
ALL 20 KAT ENTRIES PASSED ROUNDTRIP
ALL 5 SCENARIOS PASSED POLICY-DISTINCTNESS CHECK
KAT ciphertext diff against docs/spiral_atcg_v22_kat_v1.md: no differences
```

Thorough avalanche remains statistically unchanged:

```text
[BASE]  avg_hd=159.94  all-10-lanes=100.00%  zero-diff=0
[SAFE]  avg_hd=159.81  all-10-lanes=100.00%  zero-diff=0
[AGGR]  avg_hd=159.78  all-10-lanes=100.00%  zero-diff=0
[NULL]  avg_hd=160.05  all-10-lanes=100.00%  zero-diff=0
```

## Benchmark note

On the sandbox reference environment, `-O1` fast-bench remains around 6.3-6.8 MiB/s. The main value of this revision is preparing the round body for the next step: a separate `spiral_round_fast.c` scalar core with full `s0..s9` state expansion.
