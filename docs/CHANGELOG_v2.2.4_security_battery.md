# Spiral-ATCG v2.2.4 Security Battery Add-on

This add-on imports the extended classical security battery and quantum-relevant classical probe battery into the v2.2.4 full-scalar-cleanup tree.

Added files:

- `tests/test_sec_battery.c`
- `tests/test_q_battery.c`
- `docs/spiral_atcg_v224_sec_battery.csv`
- `docs/spiral_atcg_v224_q_battery.csv`
- `docs/spiral_atcg_v224_security_report.md`

Build targets:

```bash
make sec-battery
make q-battery
make security-battery
```

Validation performed in the integration environment:

- `make all` passed.
- `make test` passed:
  - roundtrip
  - pair symmetry
  - single-bit avalanche
  - batch API
  - KAT roundtrip and policy-distinctness
- `test_sec_battery.c` and `test_q_battery.c` both compiled successfully.

The security and quantum battery programs are heavier empirical probes. The captured CSV outputs supplied with the add-on are bundled under `docs/`.

Caution: these probes are empirical classical health checks. They are useful for regression and anomaly detection, but they are not formal security proofs.
