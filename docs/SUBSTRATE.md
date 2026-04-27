# Spiral-ATCG v2.2-strength substrate upgrade

## Purpose

v2.2-strength keeps the ATCG v2.0 framework unchanged and replaces only the substrate round function.

The v2.1.1 core fixed the original fixed-pair diffusion bug by adding a round-dependent lane shuffle. However, reduced-round QRES still showed that the input probe reached ideal HD and all-lane diffusion around round 6. That was materially slower than the older Spiral-DNA-320 v1 substrate.

v2.2-strength therefore follows direction B from the Phase B framing review:

```text
ATCG framework stays.
Substrate round is upgraded toward the v1 Spiral-DNA-320 diffusion structure.
```

## What changed

The public ATCG functions are unchanged:

```c
spiral_atcg_project(...);
spiral_atcg_pair(...);
spiral_atcg_transcribe_round(...);
spiral_atcg_precompute_materials(...);
spiral_atcg_init_session(...);
spiral_atcg_encrypt_block(...);
spiral_atcg_decrypt_block(...);
```

The changed part is the internal `spiral_round()` / `inv_spiral_round()` implementation.

v2.2-strength adds a v1-inspired reversible half-state substrate:

1. Split state into `L[5]` and `R[5]`.
2. Compute `S = L ^ R`.
3. Inject ATCG material into the S path.
4. Run round-dependent `dual_input()`.
5. Run `F1` and `F2` with stride-3 / round-dependent lane selection.
6. Mix `T = F1 ^ F2` through SpiralMix_v2-style butterfly.
7. Apply round-dependent output twist using `alpha(i,r)` and `beta(i,r)`.
8. Decrypt reverses the output twist, recomputes `T`, and unmasks `YL/YR`.

## Why it stays invertible

The round remains reversible because `T` is a deterministic function of:

```text
S = L ^ R
round material LM/RC/CD/SK
round index r
```

During decrypt, after inverse output twist we recover:

```text
YL = old_L ^ T
YR = old_R ^ T
YL ^ YR = old_L ^ old_R = S
```

So decrypt can recompute the same `T` and recover:

```text
old_L = YL ^ T
old_R = YR ^ T
```

## Reduced-round QRES improvement

Generated v2.2-strength QRES CSV:

```text
docs/spiral_atcg_qres_v2_2_strength_comparison.csv
```

Input probe, averaged across BASE / SAFE / AGGR / NULL:

| rounds | avg HD mean | full-diffusion % |
|---:|---:|---:|
| 2 | ~159.89 | 100% |
| 3 | ~159.97 | 100% |
| 4 | ~160.00 | 100% |
| 6 | ~160.07 | 100% |
| 8 | ~159.95 | 100% |
| 12 | ~159.99 | 100% |

This restores reduced-round diffusion to the same class as the original v1 substrate. The 16-round profile is still retained as the default engineering safety budget.

## Framing

v2.2-strength should be framed as:

> Spiral-ATCG is the meta-material / transcription framework. The substrate should not be intentionally weaker than the earlier Spiral-DNA-320 core. v2.2 therefore restores the stronger v1-style round-dependent diffusion structure while preserving the ATCG v2 production pipeline.

This avoids the weaker v2.1.1 claim that slower round-6 diffusion was merely a design trade-off.
