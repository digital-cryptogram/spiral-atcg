# Spiral-ATCG

[![CI](https://github.com/digital-cryptogram/spiral-atcg/actions/workflows/ci.yml/badge.svg)](https://github.com/digital-cryptogram/spiral-atcg/actions/workflows/ci.yml)
[![License: Apache 2.0](https://img.shields.io/badge/License-Apache_2.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![Spec](https://img.shields.io/badge/spec-v2.0-blue)](docs/SPEC.md)
[![Reference impl](https://img.shields.io/badge/reference-v2.2.4-green)](src/spiral_atcg.c)

> An open experimental research framework for grammar-driven round-material generation in word-oriented symmetric constructions.
>
> This repository is a **public review artifact**. It is not proposed as a standard-ready cipher, an AES replacement, or a production cryptographic primitive.
>
> Open cryptographic research from **[Digital Cryptogram (數位密碼科技)](https://www.digitalcryptogram.com.tw/)**.

---

[**English**](#english) · [**中文**](#中文)

---

## English

### What this is

Spiral-ATCG is an experimental research framework for studying how round material in word-oriented symmetric constructions can be generated, separated, and compared through a small grammar of production rules.

The framework is named after four material types:

- `A` — anchor
- `T` — twist
- `C` — couple
- `G` — gate

The analogy to nucleotide pairing in DNA is intentional as terminology, but it is informal and should not be read as a biological security argument.

Concretely, the project provides:

- A **specification** ([`docs/SPEC.md`](docs/SPEC.md)) of the ATCG material model, including alphabet, complement rule, antiparallel helicity, a three-step transcription pipeline, and named transcription policies (`BASE`, `SAFE`, `AGGR`, `NULL`).
- A **reference substrate**, Spiral-ATCG/v2.2.4, instantiating the framework as a 320-bit word-oriented experimental construction with 16 rounds and selectable transcription policies.
- A **portable C reference implementation** ([`src/spiral_atcg.c`](src/spiral_atcg.c)) under 700 lines, building on Linux / macOS / Windows (MinGW).
- **Regression and structural-analysis probes** covering correctness, diffusion sanity, policy behaviour, reduced-round behaviour, and additional structural checks.
- **Reproducible test vectors** ([`docs/KAT.md`](docs/KAT.md), 5 scenarios × 4 policies = 20 entries) and a deterministic structural-probe dataset ([`docs/QRES_data.csv`](docs/QRES_data.csv), 72 rows).
- A **benchmark harness** with high-resolution timing, multiple optimization levels, and per-policy reporting.

### What this is *not*

Honest scope so this work is judged correctly:

- **Not a finalized standard and not certified.** No FIPS 140-3, PCI DSS, Common Criteria, or NIST validation is claimed.
- **Not a security proof.** No IND-CPA, IND-CCA, AEAD, PRP, QROM, or post-quantum security theorem is claimed.
- **Not a replacement for AES or any NIST-approved primitive.** Spiral-ATCG is a research artifact for analysis and experimentation, not a default deployment choice.
- **Not a post-quantum cipher in the technical sense.** Post-quantum security normally refers to public-key constructions; symmetric constructions are usually discussed through key size, generic search cost, and the Grover bound.
- **Not a claim that the tests prove security.** The included tests are regression and structural-health checks. They are useful for reproducibility and for identifying obvious failures, but they do not replace independent cryptanalysis.

### Why publish this

Three reasons:

1. **Open review requires reproducible artifacts.** Specifications, reference code, test vectors, and probe data are public so that others can inspect, reproduce, and challenge the construction.
2. **The framework abstraction is the research question.** Spiral-ATCG separates material alphabet, production grammar, transcription, policies, round material, and substrate transition. This gives a vocabulary for studying policy-ablatable round-material generation rather than treating all material generation as an opaque key-schedule detail.
3. **The project is intended to invite external analysis.** We are especially interested in reduced-round, differential, linear, rotational, invariant-subspace, related-key, policy-aliasing, and implementation-level analysis.

### Questions for reviewers

We welcome technical feedback on questions such as:

- Is the separation between round-material generation and substrate transition a useful design abstraction?
- Are the policy definitions meaningfully distinct, or are some policies constructionally aliased?
- Are there reduced-round differential, linear, rotational, slide-like, invariant-subspace, or related-key distinguishers?
- Does the ATCG transcription layer reduce to a known key-schedule or whitening pattern?
- Which additional analyses would make this artifact more useful to the cryptology community?

### Quick start

```bash
git clone https://github.com/digital-cryptogram/spiral-atcg.git
cd spiral-atcg
make all          # build library + standard tests + KAT generator + benchmark
make test         # run roundtrip + pair-symmetry + avalanche + batch API tests
make full         # build + run heavier probes
make bench        # run the high-resolution benchmark
```

Expected output for `make test`:

```
ROUNDTRIP PASSED policy=BASE / SAFE / AGGR / NULL
PAIR SYMMETRY PASSED policy=BASE / SAFE / AGGR / NULL
ALL AVALANCHE TESTS PASSED
BATCH API PASSED policy=BASE / SAFE / AGGR / NULL
ALL 20 KAT ENTRIES PASSED ROUNDTRIP
```

### Probe summary at a glance

| Battery | Probes | Scope | Result |
|---|---:|---|---|
| Bundled correctness | 8 | API, roundtrip, pair symmetry, KAT consistency | Pass within test suite |
| Extended structural probes | 8 | differential, linear, key avalanche, bit balance, related-key, slide, fixed-point, reduced-round checks | No anomaly observed within tested scope |
| Additional structural probes | 6 | period, hidden-shift-style, search-margin, cross-round period, q-distinguishing, material diversity checks | No anomaly observed within tested scope |
| Advanced exploratory probes | 10 + 2 corrections | quantum-walk-style, boomerang-style, algebraic-style exploratory checks | No anomaly observed within tested scope |
| **Total** | **34 probes / 12 families** | Regression and structural-health testing | No structural anomaly observed within tested scope |

These results are **not security proofs**. They document the behaviour of the reference implementation under the included probe suite.

The reports also document one case where an initial probe design produced a false-positive signal: cube-position collisions caused trivial cube collapse. The probe was corrected, and both versions were preserved for transparency. See [`docs/ADVANCED_AUDIT.md`](docs/ADVANCED_AUDIT.md) for the methodology note.

### Repository layout

```
spiral-atcg/
├── src/            Reference C implementation
├── include/        Public C API
├── tests/          Correctness tests + probe batteries
├── bench/          High-resolution benchmark
├── docs/           Specification, reports, test vectors, changelogs
├── Makefile        GNU Make build
├── CMakeLists.txt  CMake build
└── .github/        CI workflow
```

### Documentation roadmap

For different readers:

- **First-time visitor:** read this README, then [`docs/SPEC.md`](docs/SPEC.md) §1–§7.
- **Cryptology reviewer:** [`docs/SPEC.md`](docs/SPEC.md) → [`docs/SECURITY_AUDIT.md`](docs/SECURITY_AUDIT.md) → [`docs/ADVANCED_AUDIT.md`](docs/ADVANCED_AUDIT.md) → [`docs/QRES_summary.md`](docs/QRES_summary.md).
- **Implementer:** [`docs/SPEC.md`](docs/SPEC.md) → [`include/spiral_atcg.h`](include/spiral_atcg.h) → [`src/spiral_atcg.c`](src/spiral_atcg.c) → [`docs/KAT.md`](docs/KAT.md).
- **Performance reviewer:** build with `make bench` and read [`docs/CHANGELOG_v2.2.4.md`](docs/CHANGELOG_v2.2.4.md).

### Historical documentation note

Some files under `docs/` are preserved as engineering history, release notes, or exploratory audit records. If there is any difference in claim wording between those documents and this README, the most conservative interpretation should be used.

In particular, terms such as `AEAD-style`, `schedule-hidden`, `replay protection`, `quantum-resistance indicators`, `security audit`, and `HARDENED` should be read as implementation labels or engineering descriptions, not as formal security claims.

### License

Apache License 2.0. See [`LICENSE`](LICENSE).

The Apache 2.0 license includes a patent grant clause, which is appropriate for an open research artifact intended for public review.

### Citing this work

```bibtex
@misc{spiral_atcg_2026,
  title        = {Spiral-ATCG: A Reproducible Experimental Framework for Grammar-Driven Round-Material Generation},
  author       = {Digital Cryptogram},
  year         = {2026},
  howpublished = {\url{https://github.com/digital-cryptogram/spiral-atcg}}
}
```

### Contact

- **Sponsoring organization:** [Digital Cryptogram (數位密碼科技股份有限公司)](https://www.digitalcryptogram.com.tw/)
- **Address:** 114 台北市內湖區瑞光路 578 號 8 樓
- **Phone:** 02-2657-5188
- **Email:** casey@ruitingtech.com.tw
- **Bug reports / security disclosures:** see [`SECURITY.md`](SECURITY.md)
- **Contributions:** see [`CONTRIBUTING.md`](CONTRIBUTING.md)

---

## 中文

### 這是什麼

Spiral-ATCG 是一個實驗性研究框架，用來研究 word-oriented 對稱式構造中的 round material 如何透過小型 production-rule grammar 產生、分離與比較。

框架名稱來自四種材料型別：

- `A` — anchor
- `T` — twist
- `C` — couple
- `G` — gate

DNA 鹼基配對的類比是有意採用的術語，但它是非正式 metaphor，不應被解讀為生物學式安全性論證。

具體而言，本專案提供：

- ATCG material model 的**規格文件** ([`docs/SPEC.md`](docs/SPEC.md))，包含 alphabet、complement rule、antiparallel helicity、三段 transcription pipeline，以及命名 transcription policies (`BASE`、`SAFE`、`AGGR`、`NULL`)。
- **參考 substrate** Spiral-ATCG/v2.2.4，將框架實例化為 320-bit word-oriented experimental construction，包含 16 rounds 與可選 transcription policies。
- 一份不到 700 行的**可攜式 C 參考實作** ([`src/spiral_atcg.c`](src/spiral_atcg.c))，可在 Linux / macOS / Windows (MinGW) 上編譯。
- **Regression 與 structural-analysis probes**，涵蓋 correctness、diffusion sanity、policy behaviour、reduced-round behaviour 與其他結構性檢查。
- **可重現測試向量** ([`docs/KAT.md`](docs/KAT.md)，5 scenarios × 4 policies = 20 entries) 與確定性 structural-probe dataset ([`docs/QRES_data.csv`](docs/QRES_data.csv)，72 rows)。
- **效能基準測試框架**，使用高解析度 timer、多種 optimization level 與 per-policy reporting。

### 這不是什麼

為了讓使用者正確判斷本專案定位，明確列出 scope：

- **不是已完成標準，也未取得認證。** 不宣稱 FIPS 140-3、PCI DSS、Common Criteria 或 NIST validation。
- **不是 security proof。** 不宣稱 IND-CPA、IND-CCA、AEAD、PRP、QROM 或 post-quantum security theorem。
- **不是 AES 或任何 NIST-approved primitive 的取代品。** Spiral-ATCG 是分析與實驗用研究 artifact，不是預設部署選項。
- **嚴格意義上不是後量子 cipher。** 後量子安全通常指公開金鑰構造；對稱式構造通常透過 key size、generic search cost 與 Grover bound 討論。
- **不宣稱測試結果證明安全。** 內含測試是 regression 與 structural-health checks，可用於重現與發現明顯失敗，但不能取代獨立密碼分析。

### 為什麼開源

三個原因：

1. **公開審查需要可重現 artifact。** Spec、reference code、test vectors 與 probe data 全部公開，讓外部研究者可以檢查、重現與挑戰此構造。
2. **框架抽象本身是研究問題。** Spiral-ATCG 將 material alphabet、production grammar、transcription、policies、round material 與 substrate transition 分離，提供一套研究 policy-ablatable round-material generation 的語彙，而不是把所有 material generation 都視為不透明 key schedule 細節。
3. **本專案目標是邀請外部分析。** 特別希望獲得 reduced-round、differential、linear、rotational、invariant-subspace、related-key、policy-aliasing 與 implementation-level analysis。

### 給審查者的問題

歡迎針對以下問題提供技術回饋：

- round-material generation 與 substrate transition 的分離，是否是有用的設計抽象？
- policy definitions 是否具有實質差異，或某些 policies 是否 constructionally aliased？
- 是否存在 reduced-round differential、linear、rotational、slide-like、invariant-subspace 或 related-key distinguishers？
- ATCG transcription layer 是否可化約為已知 key schedule 或 whitening pattern？
- 還需要哪些分析，才能讓這個 artifact 對 cryptology community 更有用？

### 快速開始

```bash
git clone https://github.com/digital-cryptogram/spiral-atcg.git
cd spiral-atcg
make all          # 編譯 library + 標準測試 + KAT 產生器 + benchmark
make test         # 跑 roundtrip + pair-symmetry + avalanche + batch API 測試
make full         # 編譯並執行較重的 probes
make bench        # 跑高解析度 benchmark
```

`make test` 預期輸出：

```
ROUNDTRIP PASSED policy=BASE / SAFE / AGGR / NULL
PAIR SYMMETRY PASSED policy=BASE / SAFE / AGGR / NULL
ALL AVALANCHE TESTS PASSED
BATCH API PASSED policy=BASE / SAFE / AGGR / NULL
ALL 20 KAT ENTRIES PASSED ROUNDTRIP
```

### Probe 結果一覽

| Battery | Probe 數 | Scope | 結果 |
|---|---:|---|---|
| 內建 correctness 測試 | 8 | API、roundtrip、pair symmetry、KAT consistency | 測試套件內通過 |
| 擴展結構性 probes | 8 | differential、linear、key avalanche、bit balance、related-key、slide、fixed-point、reduced-round checks | 測試範圍內未觀察到 anomaly |
| 額外結構性 probes | 6 | period、hidden-shift-style、search-margin、cross-round period、q-distinguishing、material diversity checks | 測試範圍內未觀察到 anomaly |
| 進階探索性 probes | 10 + 2 修正 | quantum-walk-style、boomerang-style、algebraic-style exploratory checks | 測試範圍內未觀察到 anomaly |
| **合計** | **34 probes / 12 families** | Regression and structural-health testing | 測試範圍內未觀察到結構性異常 |

這些結果**不是安全性證明**，只是記錄 reference implementation 在內含 probe suite 下的行為。

稽核報告中也記錄一次初始 probe 設計造成的 false-positive 信號：cube-position collisions 導致 trivial cube collapse。該 probe 已被修正，兩個版本都保留作為透明紀錄。Methodology note 見 [`docs/ADVANCED_AUDIT.md`](docs/ADVANCED_AUDIT.md)。

### 檔案結構

```
spiral-atcg/
├── src/            參考 C 實作
├── include/        公開 C API
├── tests/          Correctness 測試 + probe batteries
├── bench/          高解析度 benchmark
├── docs/           Spec、reports、test vectors、changelogs
├── Makefile        GNU Make build
├── CMakeLists.txt  CMake build
└── .github/        CI workflow
```

### 文件閱讀建議

依照不同讀者：

- **第一次來訪：** 讀本 README，再看 [`docs/SPEC.md`](docs/SPEC.md) §1–§7。
- **Cryptology reviewer：** [`docs/SPEC.md`](docs/SPEC.md) → [`docs/SECURITY_AUDIT.md`](docs/SECURITY_AUDIT.md) → [`docs/ADVANCED_AUDIT.md`](docs/ADVANCED_AUDIT.md) → [`docs/QRES_summary.md`](docs/QRES_summary.md)。
- **實作者：** [`docs/SPEC.md`](docs/SPEC.md) → [`include/spiral_atcg.h`](include/spiral_atcg.h) → [`src/spiral_atcg.c`](src/spiral_atcg.c) → [`docs/KAT.md`](docs/KAT.md)。
- **效能審查者：** 用 `make bench` 編譯後執行，並參閱 [`docs/CHANGELOG_v2.2.4.md`](docs/CHANGELOG_v2.2.4.md)。

### 歷史文件說明

`docs/` 下部分文件保留為工程歷史、release notes 或探索性稽核紀錄。如果這些文件與本 README 在 claim 語氣上有差異，應採用較保守的解讀。

特別是 `AEAD-style`、`schedule-hidden`、`replay protection`、`quantum-resistance indicators`、`security audit` 與 `HARDENED` 等詞，應解讀為 implementation labels 或工程描述，**不是 formal security claims**。

### 授權

Apache License 2.0，詳見 [`LICENSE`](LICENSE)。

Apache 2.0 包含 patent grant clause，適合作為公開審查用研究 artifact 的授權。

### 引用方式

```bibtex
@misc{spiral_atcg_2026,
  title        = {Spiral-ATCG: A Reproducible Experimental Framework for Grammar-Driven Round-Material Generation},
  author       = {Digital Cryptogram},
  year         = {2026},
  howpublished = {\url{https://github.com/digital-cryptogram/spiral-atcg}}
}
```

### 聯絡資訊

- **贊助組織：** [數位密碼科技股份有限公司 (Digital Cryptogram)](https://www.digitalcryptogram.com.tw/)
- **地址：** 114 台北市內湖區瑞光路 578 號 8 樓
- **電話：** 02-2657-5188
- **Email：** casey@ruitingtech.com.tw
- **Bug 回報 / 資安揭露：** 請見 [`SECURITY.md`](SECURITY.md)
- **貢獻方式：** 請見 [`CONTRIBUTING.md`](CONTRIBUTING.md)
