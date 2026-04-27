# Documentation index

Quick navigation to the most relevant documents for different audiences.

## Primary documents (read these first)

| File | Description |
|---|---|
| [`SPEC.md`](SPEC.md) | Full ATCG meta-material model specification (alphabet, grammar, helicity, transcription pipeline, four policies) |
| [`SUBSTRATE.md`](SUBSTRATE.md) | Substrate cipher specification (Spiral-ATCG/v2.2.4, 320-bit block, 16 rounds, four phases) |
| [`KAT.md`](KAT.md) | Known-answer test vectors: 5 scenarios × 4 policies = 20 entries |

## Audit reports

| File | Coverage |
|---|---|
| [`SECURITY_AUDIT.md`](SECURITY_AUDIT.md) | Security audit covering 22 probes (correctness, extended security, quantum-resistance indicators) |
| [`ADVANCED_AUDIT.md`](ADVANCED_AUDIT.md) | Advanced cryptanalysis (quantum walk indicators, boomerang distinguisher, algebraic attacks) — 10 probes + 2 corrections |
| [`QRES_summary.md`](QRES_summary.md) | Quantum-Resistance Evidence Suite summary, with paper-ready framing |

## Audit raw data

| File | Format |
|---|---|
| [`audit_sec_battery.csv`](audit_sec_battery.csv) | Extended security battery raw results |
| [`audit_q_battery.csv`](audit_q_battery.csv) | Quantum-resistance battery raw results |
| [`audit_adv_battery.csv`](audit_adv_battery.csv) | Advanced cryptanalytic battery raw results |
| [`QRES_data.csv`](QRES_data.csv) | QRES v2 dataset (4 policies × 3 probes × 6 round counts × 16 tuples = 72 rows) |
| [`QRES_3panel.svg`](QRES_3panel.svg) | QRES v2 convergence figure (3 panels: input / seed / nonce probes) |

## Changelogs (substrate evolution)

These document the design iteration history. Read in order if you want to understand how the substrate evolved.

| File | What changed |
|---|---|
| [`CHANGELOG_v2.1.1.md`](CHANGELOG_v2.1.1.md) | v2.1 → v2.1.1: fixed substrate diffusion bug (round-dependent lane shuffle) |
| [`CHANGELOG_v2.2.1.md`](CHANGELOG_v2.2.1.md) | v2.1.1 → v2.2.1: per-session round-material cache, inline helpers, no semantic change |
| [`CHANGELOG_v2.2.2.md`](CHANGELOG_v2.2.2.md) | v2.2.1 → v2.2.2: fixed-16-round scalar dispatch, batch encrypt/decrypt API |
| [`CHANGELOG_v2.2.4.md`](CHANGELOG_v2.2.4.md) | v2.2.2 → v2.2.4: load/store unrolling, spiral_mix_v2 unrolling |
| [`CHANGELOG_v2.2.4_security_battery.md`](CHANGELOG_v2.2.4_security_battery.md) | Addition of the comprehensive security audit batteries |

## Reading orders by audience

### First-time visitor
1. Top-level `README.md`
2. `SPEC.md` §1 through §7 (introduction + framework)
3. `SECURITY_AUDIT.md` executive summary

### Cryptographic reviewer
1. `SPEC.md` (full)
2. `SUBSTRATE.md`
3. `SECURITY_AUDIT.md`
4. `ADVANCED_AUDIT.md`
5. `QRES_summary.md`
6. Source code: `../include/spiral_atcg.h` and `../src/spiral_atcg.c`
7. Audit raw data CSVs for independent verification

### Implementer
1. `SPEC.md` (full)
2. `../include/spiral_atcg.h` (public API)
3. `../src/spiral_atcg.c` (reference implementation)
4. `KAT.md` (test vectors for verification)
5. `../tests/` (correctness test patterns)

### Procurement / compliance officer
1. Top-level `README.md` (especially "What this is NOT" section)
2. Top-level `SECURITY.md` (disclosure policy + scope disclaimer)
3. `QRES_summary.md` framing section

### Performance reviewer
1. `CHANGELOG_v2.2.1.md` through `CHANGELOG_v2.2.4.md` (optimization trajectory)
2. `../bench/benchmark.c` source
3. Run `make bench` and inspect output

## Sponsorship

This open research project is sponsored by:

**Digital Cryptogram (數位密碼科技股份有限公司)**  
[https://www.digitalcryptogram.com.tw/](https://www.digitalcryptogram.com.tw/)

For commercial enterprise cryptography products (Zentra KMS, Zentra CSP, PQC E2EGW, etc.), please visit the sponsor's website.
