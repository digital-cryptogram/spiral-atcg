# Spiral-ATCG v2.2-strength QRES summary

## TL;DR

v2.2-strength resolves the Phase B framing problem by upgrading the substrate round function instead of trying to defend slower diffusion. The ATCG v2.0 transcription model is unchanged, but the internal `SpiralRound` now uses a v1-inspired round-dependent diffusion structure.

The important result is that the input probe no longer waits until round 6. In the generated QRES run, input diffusion is already ideal-class by round 2 across all four policies.

## Generated artifact

```text
docs/spiral_atcg_qres_v2_2_strength_comparison.csv
```

The CSV contains 73 lines: one header plus 72 data rows for:

```text
4 policies × 3 probes × 6 round counts
```

## Input probe: substrate diffusion

Input probe measures the cipher substrate. It is the most important reduced-round diffusion check.

Averaged across BASE / SAFE / AGGR / NULL:

| rounds | avg HD mean | full-diffusion % | zero-diff |
|---:|---:|---:|---:|
| 2 | ~159.89 | 100% | 0 |
| 3 | ~159.97 | 100% | 0 |
| 4 | ~160.00 | 100% | 0 |
| 6 | ~160.07 | 100% | 0 |
| 8 | ~159.95 | 100% | 0 |
| 12 | ~159.99 | 100% | 0 |

This is a material improvement over v2.1.1, where the input probe reached ideal behavior only around round 6.

## Seed and nonce probes: transcription/KDF diffusion

Seed and nonce probes measure the upstream ATCG transcription path, not only the cipher substrate. They remain useful health checks, but they should be framed separately from input diffusion.

In v2.2-strength, seed and nonce probes also remain near ideal HD across all measured round counts, with zero-diff total equal to zero.

## Four-policy interpretation

The policy comparison should be split into two claims:

1. **NULL vs {BASE, SAFE, AGGR}**: close differential behavior is positive. It means the substrate diffusion does not depend on a policy-specific crutch.
2. **BASE vs SAFE vs AGGR**: close QRES curves do not rank the three research policies. The BASE / SAFE / AGGR ordering must be argued structurally through the transcription rules and compatibility model, not from QRES alone.

## Honest scope

QRES is not a security proof. It is a reduced-round empirical health check for diffusion, zero-difference behavior, and policy-axis sanity. The value of v2.2-strength is that it removes the obvious reduced-round weakness from v2.1.1 before Phase C paper drafting.
