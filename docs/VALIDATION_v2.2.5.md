# v2.2.5 validation — 2026-11-09

Target: `2.2.5-feistel-research`. Platform: arm64 macOS, Apple Clang 17.0.0.
This is regression evidence and a limited structural analysis, not a security
certification or a quantum resource estimate.

## Executed checks

| Check | Observed result |
|---|---|
| Default `make test` | Passed |
| Absolute `BUILD` output directory | Passed after publication review |
| Complete suite, Clang `-O0`, warnings as errors | Passed |
| Complete suite, Clang `-O2`, warnings as errors | Passed |
| Complete suite, `-O1`, AddressSanitizer + UBSan, warnings as errors | Passed, no sanitizer diagnostics |
| Roundtrip + in-place checks, 4 policies, rounds 1–32 | 16,384 base samples passed |
| Dense affine parity regressions, rounds 16–32 | 8,704 samples; no exact/negated relation among tested masks within each session |
| Structured differences `55`, `AA`, `FF` repeated across the block | 26,112 difference samples; output difference varied within every tested session/mask |
| All-bit affine rank, default 16 rounds | 16 sessions × 768 pairs; rank 641/641 in every session |
| Generic vs unrolled 16-round paths | 128 sessions; encrypt and arbitrary-ciphertext decrypt matched |
| Batch vs single-block, in-place and empty batches | All policies, rounds 1–32 passed |
| Frozen KATs | 20/20 matched in all build settings; roundtrip and policy distinctness passed |
| Heavy probes | `make full` built; thorough avalanche and 72-configuration QRES executed |

The affine regression uses the three dense masks `55`, `AA`, `FF`, all nine
input/output pairings, and 32 plaintexts per session. It rejects equality or
inequality holding for all 32 samples. It is not a statistical bound on bias.
Differential tests reject any constant output difference, not only the previously
predicted difference. The regression's reduced-round checks cover correctness;
no security is claimed for API-supported reduced-round profiles.

The rank test considers rows `[P bits, E(P) bits, 1]` over GF(2). Full rank 641
provides witnesses excluding every nonzero *exact affine relation* for each of
these 16 sessions. It does not exclude biased linear approximations, nonlinear
relations, untested weak sessions, differential attacks or key recovery.

## Negative controls

The new tests were also compiled against the archived, unmodified v2.2.4 source
and matching header:

```text
FAIL affine parity: policy=BASE rounds=16 session=0 input=55 output=55 matches=32/32
Exit status: 3

AFFINE RANK FAIL policy=BASE session=0 rank=639/641
Exit status: 2
```

Thus the checks actually reject the old weakness. Changing one frozen expected
ciphertext byte in an isolated copy of the KAT verifier produced:

```text
FROZEN KAT MISMATCH: TV1 policy=BASE
Exit status: 1
```

The normal source/header were not changed by that negative control. All 20 new
ciphertexts differ from their corresponding v2.2.4 vectors. Frozen vectors were
generated from this reference implementation; they are regression fixtures, not
independent verification of the full design.

## Diffusion observations

Thorough default-16-round probe: 2,560 single-bit changes per policy.

| Policy | Mean changed bits / 320 | Min | Max | All 10 lanes active | Zero differences |
|---|---:|---:|---:|---:|---:|
| BASE | 160.25 | 127 | 193 | 100% | 0 |
| SAFE | 159.69 | 130 | 190 | 100% | 0 |
| AGGR | 159.96 | 129 | 193 | 100% | 0 |
| NULL | 160.20 | 130 | 191 | 100% | 0 |

The current [QRES data](QRES_v2.2.5.csv) covers four policies, three probe types,
six round counts and 16 tuples per configuration. Input probes at **2 rounds**
have mean HD 120.35–120.53 and 50% all-lane diffusion. At **3 rounds** the sampled
means are 159.78–159.96 and all-lane diffusion is 100%; sampled 4/6/8/12-round
input probes likewise have 100% all-lane diffusion. This behavior is consistent
with propagation through the new half-state Feistel update. It is not the old
2-round result and must not be presented as evidence of a quantified round margin.

## Reproduction

From the repository directory:

```sh
make test
make BUILD=build-o2 CC=clang CFLAGS='-O2 -std=c11 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Wpedantic -Werror -Iinclude' test
make BUILD=build-san CC=clang CFLAGS='-O1 -g -std=c11 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Wpedantic -Werror -fsanitize=address,undefined -fno-omit-frame-pointer -Iinclude' test
make full
make thorough
make qres
```

Normal `kat_generator` checks the frozen table. `kat_generator --generate`
deliberately bypasses that check for explicit future vector generation. Never
regenerate expected vectors during an ordinary regression run.

## Limits and evidence files

Logs and negative-control results are in [validation-v2.2.5/](validation-v2.2.5/).
CMake/CTest registration and CI/MinGW scripts were updated, but CMake was not
installed on this host and neither Windows/Linux nor hosted CI was executed.
The old exploratory security/quantum/advanced batteries were not rerun as a
security acceptance criterion; their historical labels are not security proofs.

The engineering KDF is retained, as are the four material policies and nonlinear
helpers. No independent review, constant-time audit, Q1/Q2 security proof,
quantum gate-cost estimate, authentication mode, or production approval is
provided. Passing these checks only supports the specific claims above.
