# Spiral-ATCG External Review Guide

Spiral-ATCG is an experimental research artifact for grammar-driven round-material generation in word-oriented symmetric constructions.

It is not proposed as a standard-ready cipher, an AES replacement, or a production cryptographic primitive.

This document explains what kind of external review would be most useful, what is not claimed, and how to reproduce the artifact.

---

## What to review

We are especially interested in technical analysis of the following:

- reduced-round differential distinguishers
- reduced-round linear distinguishers
- rotational or ARX-style structural relations
- invariant-subspace or slide-like structures
- related-key behaviour
- policy aliasing between `BASE` / `SAFE` / `AGGR` / `NULL`
- whether the ATCG transcription layer reduces to a known key-schedule or whitening pattern
- structural relations between the named transcription policies
- implementation-level observations (timing, sanitizer findings, undefined behaviour)

Negative results — clear statements that a particular distinguisher does *not* exist within a given model — are equally welcome.

---

## What is not claimed

To make the scope of this artifact unambiguous:

- **No security proof.** No IND-CPA, IND-CCA, AEAD, PRP, QROM, or post-quantum security theorem is claimed.
- **No certification.** No FIPS 140-3, PCI DSS, Common Criteria, or NIST validation is claimed.
- **No recommendation for production deployment.** Spiral-ATCG is a research artifact for analysis and experimentation, not a default deployment choice.
- **No claim that the included probes prove security.** The bundled tests are regression and structural-health checks; they document the behaviour of the reference implementation under the included probe suite, and they do not replace independent cryptanalysis.
- **No claim of post-quantum security in the technical sense.** Symmetric constructions are normally discussed through key size, generic search cost, and the Grover bound; this artifact makes no quantitative quantum-security claim.

---

## Reproduction

### Basic build

```bash
git clone https://github.com/digital-cryptogram/spiral-atcg.git
cd spiral-atcg
make all          # build library + standard tests + KAT generator + benchmark
make test         # roundtrip + pair-symmetry + avalanche + batch API tests
make full         # heavier structural probes
make bench        # high-resolution benchmark
```

### Test vectors and probe data

- [`docs/KAT.md`](docs/KAT.md) — 5 scenarios × 4 policies = 20 deterministic KAT entries
- [`docs/QRES_data.csv`](docs/QRES_data.csv) — 72-row deterministic structural-probe dataset

### Probe methodology

- [`docs/SECURITY_AUDIT.md`](docs/SECURITY_AUDIT.md) — extended structural probe battery
- [`docs/ADVANCED_AUDIT.md`](docs/ADVANCED_AUDIT.md) — advanced exploratory probes; also documents one corrected probe (cube-position collision false positive) preserved for transparency
- [`docs/QRES_summary.md`](docs/QRES_summary.md) — structural-probe summary

### Specification

- [`docs/SPEC.md`](docs/SPEC.md) — the ATCG material model: alphabet, complement rule, antiparallel helicity, three-step transcription pipeline, and named transcription policies

---

## How to report feedback

Please open a GitHub issue with as many of the following as apply:

- **Affected policy** (`BASE`, `SAFE`, `AGGR`, `NULL`, or all)
- **Affected round count** (full 16, or a specific reduced-round value)
- **Input difference / mask / condition** (the structural object you used)
- **Reproduction command** (the exact command and any seed value)
- **Observed distinguisher or failure** (what you saw)
- **Expected implication** (what this implies about the construction)

For sensitive findings (e.g.\ a practical break), please prefer the channel described in [`SECURITY.md`](SECURITY.md) over a public issue.

---

## Scope reminder

This artifact is offered as an experimental cryptographic substrate that can be analysed reproducibly. The value of an external review is in technical findings — distinguishers, reductions, structural relations, implementation observations — not in opinions about whether the framework should exist.

Negative findings, partial findings, and corrections to the included methodology are all useful contributions and will be acknowledged.
