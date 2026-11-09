# Documentation index

## Current v2.2.5 research version

| Document | Scope |
|---|---|
| [CHANGELOG_v2.2.5.md](CHANGELOG_v2.2.5.md) | Security repair, compatibility and remaining work |
| [VALIDATION_v2.2.5.md](VALIDATION_v2.2.5.md) | Executed checks and their limits |
| [SUBSTRATE.md](SUBSTRATE.md) | Current balanced Feistel transition and inverse |
| [SPEC.md](SPEC.md) | ATCG framework; §7.3 corrected; §18 current version |
| [KAT.md](KAT.md) | 20 current vectors, also frozen in the test header |
| [QRES_v2.2.5.csv](QRES_v2.2.5.csv) | Current 72-row diffusion probe data |

## Historical records — not current security evidence

| Document | Scope |
|---|---|
| [REASSESSMENT_v2.2.4.zh-TW.md](REASSESSMENT_v2.2.4.zh-TW.md) | Full-round invariants found in v2.2.4 |
| [SUBSTRATE_v2.2.4.md](SUBSTRATE_v2.2.4.md) | Previous round structure |
| [KAT_v2.2.4.md](KAT_v2.2.4.md) | Previous ciphertext vectors |
| [SECURITY_AUDIT.md](SECURITY_AUDIT.md) | Earlier probes that missed those invariants |
| [ADVANCED_AUDIT.md](ADVANCED_AUDIT.md) | Earlier exploratory probes |
| [QRES_summary.md](QRES_summary.md) | Earlier diffusion summary |

`QRES_data.csv`, `QRES_3panel.svg`, and `audit_*_battery.csv` retain previous data.
Earlier `CHANGELOG_v2.*.md` files preserve history; their measurements and security
wording must not be applied to v2.2.5. `make test` runs the current regression
suite. `make full` only builds heavy probes; `make thorough` and `make qres` run
the diffusion probes.
