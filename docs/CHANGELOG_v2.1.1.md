# Spiral-ATCG v2.1.1-fast Diffusion Fix

## Status

This is a substrate-only engineering fix. It does not change the Spiral-ATCG v2.0 framework, alphabet, production rules, helicity rule, transcription pipeline, or policy semantics.

## Problem in v2.1-fast

The v2.1-fast substrate round used a fixed Couple pairing:

```c
(0,9), (1,8), (2,7), (3,6), (4,5)
```

Because the other phases were per-lane or material-only, the state formed five closed two-lane components. A single-bit plaintext change in lane 0 could not propagate to lanes 1..8 after 16 rounds.

## Fix

v2.1.1 adds a round-dependent bijective lane shuffle between Phase 1 and the Couple phase:

```c
tmp[i] = s[(i * 3 + r * 7) % 10];
```

The inverse round applies the inverse permutation after inverse Couple and before inverse Phase 1.

## Why this is minimal

- The ATCG transcription layer is unchanged.
- Precomputed `LM / RC / CD / SK` material cache is unchanged.
- `SpiralRound` / `InvSpiralRound` remain reversible.
- Roundtrip and pair-symmetry tests remain valid.
- A new avalanche test verifies that a 1-bit plaintext change activates all 10 lanes.

## Required tests

```bash
make test
```

Expected core result:

```text
ALL V2 ROUNDTRIPS PASSED
ALL PAIR SYMMETRY TESTS PASSED
ALL AVALANCHE TESTS PASSED
```
