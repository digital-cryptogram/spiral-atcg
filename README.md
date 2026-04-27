# Spiral-ATCG

[![CI](https://github.com/digital-cryptogram/spiral-atcg/actions/workflows/ci.yml/badge.svg)](https://github.com/digital-cryptogram/spiral-atcg/actions/workflows/ci.yml)
[![License: Apache 2.0](https://img.shields.io/badge/License-Apache_2.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![Spec](https://img.shields.io/badge/spec-v2.0-blue)](docs/SPEC.md)
[![Reference impl](https://img.shields.io/badge/reference-v2.2.4-green)](src/spiral_atcg.c)

> An open meta-material framework for symmetric cipher design, with a fully specified 320-bit reference cipher, a comprehensive security audit, and a reproducible benchmark harness.
>
> Open cryptographic research from **[Digital Cryptogram (數位密碼科技)](https://www.digitalcryptogram.com.tw/)**.

---

[**English**](#english) · [**中文**](#中文)

---

## English

### What this is

Spiral-ATCG is a research framework for designing symmetric block ciphers as compositions of typed material families derived from session inputs through a small grammar of production rules. The framework is named after the four material types it uses (`A` anchor, `T` twist, `C` couple, `G` gate); the analogy to nucleotide pairing in DNA is intentional but informal.

Concretely the project provides:

- A **specification** (`docs/SPEC.md`) of the ATCG meta-material model, including alphabet, complement rule, antiparallel helicity, three-step transcription pipeline, and four named transcription policies (`BASE`, `SAFE`, `AGGR`, `NULL`).
- A **reference cipher**, Spiral-ATCG/v2.2.4, instantiating the framework as a 320-bit block cipher with 16 rounds and four selectable transcription policies.
- A **portable C reference implementation** (`src/spiral_atcg.c`) under 700 lines, building cleanly on Linux / macOS / Windows (MinGW).
- A **comprehensive security audit** spanning 34 probes across 4 batteries (correctness, extended security, quantum-resistance indicators, advanced cryptanalysis).
- **Reproducible test vectors** (`docs/KAT.md`, 5 scenarios × 4 policies = 20 entries) and a deterministic QRES dataset (`docs/QRES_data.csv`, 72 rows).
- A **benchmark harness** with high-resolution timing, multiple optimization levels, and per-policy reporting.

### What this is *not*

Honest scope so this is judged correctly:

- **Not a finalized standard, not certified.** No FIPS 140-3, no PCI DSS, no Common Criteria certification. Production cryptography in regulated environments must use NIST-approved algorithms.
- **Not a security proof.** No IND-CPA / IND-CCA / QROM proof, in line with how almost all standalone block ciphers are positioned (including AES, ChaCha20, SPECK).
- **Not a replacement for AES.** Spiral-ATCG is a research framework. It is suitable for analysis, education, and as a complement to standard cryptography in `Crypto Agility` settings — not as a default cipher choice.
- **Not a post-quantum cipher** in the technical sense (post-quantum refers to public-key constructions; symmetric ciphers obtain quantum resistance through key size and the Grover bound).

### Why publish this

Three reasons:

1. **Open research benefits from external eyes.** Specifications, reference code, and audit data being public is the only honest path for a cryptographic project.
2. **The framework abstraction is the contribution.** Separating cipher design into typed alphabet (Layer 1) → grammar (Layer 2) → round materials (Layer 3) → substrate (Layer 4) gives a cleaner vocabulary for analysis than the conventional "key schedule + round function" split.
3. **Credibility through transparency.** Digital Cryptogram builds enterprise PQC and KMS products; this open project demonstrates the cryptographic depth behind those products.

### Quick start

```bash
git clone https://github.com/digital-cryptogram/spiral-atcg.git
cd spiral-atcg
make all          # build library + standard tests + KAT generator + benchmark
make test         # run roundtrip + pair-symmetry + avalanche + batch API tests
make full         # build + run heavier probes (thorough avalanche, material trace, QRES)
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

### Audit summary at a glance

| Battery | Probes | Result |
|---|---|---|
| Bundled correctness | 8 | All pass |
| Extended security | 8 (differential, linear, key avalanche, bit balance, related-key, slide, fixed-point, reduced-round) | No anomaly |
| Quantum-resistance indicators | 6 (Simon period, hidden shift, Grover margin, cross-round period, q-distinguishing, material diversity) | No anomaly |
| Advanced cryptanalysis | 10 + 2 corrections (quantum walk, boomerang, algebraic) | No anomaly |
| **Total** | **34 probes / 12 attack families** | **No structural anomaly within tested scope** |

The audit reports document one case where an initial probe design produced a false-positive signal (cube-position collisions causing trivial cube collapse); we corrected the probe and republished both versions. See `docs/ADVANCED_AUDIT.md` for the methodology note.

### Repository layout

```
spiral-atcg/
├── src/            Reference C implementation
├── include/        Public C API
├── tests/          Correctness tests + audit batteries
├── bench/          High-resolution benchmark
├── docs/           Specification, audit reports, test vectors, changelogs
├── Makefile        GNU Make build (default: Linux/macOS, also MinGW)
├── CMakeLists.txt  CMake build (cross-platform)
└── .github/        CI workflow
```

### Documentation roadmap

For different readers:

- **First-time visitor**: read this README, then [`docs/SPEC.md`](docs/SPEC.md) §1–§7.
- **Security reviewer**: [`docs/SECURITY_AUDIT.md`](docs/SECURITY_AUDIT.md) → [`docs/ADVANCED_AUDIT.md`](docs/ADVANCED_AUDIT.md) → [`docs/QRES_summary.md`](docs/QRES_summary.md).
- **Implementer**: [`docs/SPEC.md`](docs/SPEC.md) → [`include/spiral_atcg.h`](include/spiral_atcg.h) → [`src/spiral_atcg.c`](src/spiral_atcg.c) → [`docs/KAT.md`](docs/KAT.md) for test vectors.
- **Procurement / compliance**: this README + [`docs/QRES_summary.md`](docs/QRES_summary.md) framing section + [SECURITY.md](SECURITY.md).
- **Performance reviewer**: build with `make bench` and read [`docs/CHANGELOG_v2.2.4.md`](docs/CHANGELOG_v2.2.4.md) for the optimization trajectory.

### License

Apache License 2.0. See [LICENSE](LICENSE).

The Apache 2.0 license includes an explicit patent grant, which we believe is the appropriate posture for an open cryptographic research project.

### Citing this work

```bibtex
@misc{spiral_atcg_2026,
  title  = {Spiral-ATCG: An Open Meta-Material Framework for Symmetric Cipher Design},
  author = {Digital Cryptogram},
  year   = {2026},
  howpublished = {\url{https://github.com/digital-cryptogram/spiral-atcg}}
}
```

### Contact

- **Sponsoring organization**: [Digital Cryptogram (數位密碼科技股份有限公司)](https://www.digitalcryptogram.com.tw/)
- **Address**: 114 台北市內湖區瑞光路 578 號 8 樓
- **Phone**: 02-2657-5188
- **Email**: sales_digi@digitalcryptogram.com.tw
- **Bug reports / security disclosures**: see [SECURITY.md](SECURITY.md)
- **Contributions**: see [CONTRIBUTING.md](CONTRIBUTING.md)

---

## 中文

### 這是什麼

Spiral-ATCG 是一個對稱式區塊加密演算法的研究框架。它把 cipher 設計拆成「型別化的材料家族」這個層次，並透過一組 production rule grammar 從 session inputs 推導出每一輪所需的材料。框架以四個材料型別命名 (`A` anchor、`T` twist、`C` couple、`G` gate)；DNA 鹼基配對的類比是有意採用但屬於命名層的 metaphor。

具體上本專案提供：

- ATCG 元材料模型的**完整 spec** (`docs/SPEC.md`)，包含 alphabet、complement rule、antiparallel helicity、三段 transcription pipeline、以及四種命名 transcription policy (`BASE`、`SAFE`、`AGGR`、`NULL`)。
- **參考 cipher** Spiral-ATCG/v2.2.4，將框架實例化為 320-bit block / 16-round / 4 個可選 policy 的對稱式區塊加密。
- 一份不到 700 行的**可攜式 C 參考實作** (`src/spiral_atcg.c`)，可在 Linux / macOS / Windows (MinGW) 上乾淨編譯。
- 完整**安全性稽核**：4 個 battery、總計 34 個 probe（涵蓋 correctness、extended security、quantum-resistance indicators、advanced cryptanalysis）。
- **可重現的測試向量** (`docs/KAT.md`，5 scenario × 4 policy = 20 entries) 與確定性 QRES 資料集 (`docs/QRES_data.csv`，72 rows)。
- **效能基準測試**框架，使用高解析度 timer、多個 optimization level、per-policy reporting。

### 這不是什麼

為了讓使用者能正確判斷本專案的定位，明確列出 scope:

- **這不是已完成的標準、未取得認證**。沒有 FIPS 140-3、沒有 PCI DSS、沒有 Common Criteria。受監管環境的生產加密應使用 NIST 認證演算法。
- **這不是 security proof**。沒有 IND-CPA / IND-CCA / QROM 證明 — 這個 status 跟幾乎所有 standalone block cipher 一樣（包括 AES、ChaCha20、SPECK）。
- **這不是 AES 的取代品**。Spiral-ATCG 是研究框架，適合用於分析、教學、以及 `Crypto Agility` 場景中作為標準演算法的補充選擇 — 不是預設加密選項。
- **嚴格意義上不是後量子 cipher**（後量子指公開金鑰構造；對稱 cipher 的量子抗性來自 key size + Grover bound）。

### 為什麼開源

三個原因：

1. **公開研究受惠於外部審視**。Spec、reference code、audit data 全部 public 是密碼學專案唯一誠實的路徑。
2. **框架抽象本身是貢獻**。把 cipher 設計拆成 typed alphabet (Layer 1) → grammar (Layer 2) → round materials (Layer 3) → substrate (Layer 4) 的分層，比傳統「key schedule + round function」二分法提供更乾淨的分析語彙。
3. **以透明度建立 credibility**。數位密碼科技開發 enterprise PQC 與 KMS 產品；這個 open project 展示這些產品背後的密碼學深度。

### 快速開始

```bash
git clone https://github.com/digital-cryptogram/spiral-atcg.git
cd spiral-atcg
make all          # 編譯 library + 標準測試 + KAT 產生器 + benchmark
make test         # 跑 roundtrip + pair-symmetry + avalanche + batch API 測試
make full         # 加上重型 probe (thorough avalanche、material trace、QRES)
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

### 稽核結果一覽

| Battery | Probe 數 | 結果 |
|---|---|---|
| 內建 correctness 測試 | 8 | 全過 |
| 擴展安全測試 | 8（differential、linear、key avalanche、bit balance、related-key、slide、fixed-point、reduced-round） | 無 anomaly |
| Quantum-resistance 指標 | 6（Simon period、hidden shift、Grover margin、cross-round period、q-distinguishing、material diversity） | 無 anomaly |
| 進階密碼分析 | 10 + 2 修正（quantum walk、boomerang、algebraic） | 無 anomaly |
| **合計** | **34 probes / 12 attack families** | **測試範圍內無結構性異常** |

稽核報告中明確記載一次 probe 設計造成的 false-positive 信號（cube position collision 導致 trivial cube collapse）— 我們修正 probe 並 republish 兩個版本。Methodology note 見 `docs/ADVANCED_AUDIT.md`。

### 檔案結構

```
spiral-atcg/
├── src/            參考 C 實作
├── include/        公開 C API
├── tests/          Correctness 測試 + 稽核 battery
├── bench/          高解析度 benchmark
├── docs/           Spec、稽核報告、測試向量、changelog
├── Makefile        GNU Make build（預設 Linux/macOS，也支援 MinGW）
├── CMakeLists.txt  CMake build（跨平台）
└── .github/        CI workflow
```

### 文件閱讀建議

依照不同讀者：

- **第一次來訪**：讀本 README，再看 [`docs/SPEC.md`](docs/SPEC.md) §1–§7。
- **資安審查者**：[`docs/SECURITY_AUDIT.md`](docs/SECURITY_AUDIT.md) → [`docs/ADVANCED_AUDIT.md`](docs/ADVANCED_AUDIT.md) → [`docs/QRES_summary.md`](docs/QRES_summary.md)。
- **實作者**：[`docs/SPEC.md`](docs/SPEC.md) → [`include/spiral_atcg.h`](include/spiral_atcg.h) → [`src/spiral_atcg.c`](src/spiral_atcg.c) → [`docs/KAT.md`](docs/KAT.md) 取測試向量。
- **採購 / 法遵**：本 README + [`docs/QRES_summary.md`](docs/QRES_summary.md) 的 framing 章節 + [SECURITY.md](SECURITY.md)。
- **效能審查者**：用 `make bench` 編譯後執行，並參閱 [`docs/CHANGELOG_v2.2.4.md`](docs/CHANGELOG_v2.2.4.md) 看 optimization 軌跡。

### 授權

Apache License 2.0，詳見 [LICENSE](LICENSE)。

Apache 2.0 包含明確的 patent grant clause，我們認為這是 open cryptographic research project 適當的授權姿態。

### 引用方式

```bibtex
@misc{spiral_atcg_2026,
  title  = {Spiral-ATCG: An Open Meta-Material Framework for Symmetric Cipher Design},
  author = {Digital Cryptogram},
  year   = {2026},
  howpublished = {\url{https://github.com/digital-cryptogram/spiral-atcg}}
}
```

### 聯絡資訊

- **贊助組織**：[數位密碼科技股份有限公司 (Digital Cryptogram)](https://www.digitalcryptogram.com.tw/)
- **地址**：114 台北市內湖區瑞光路 578 號 8 樓
- **電話**：02-2657-5188
- **Email**：sales_digi@digitalcryptogram.com.tw
- **Bug 回報 / 資安揭露**：請見 [SECURITY.md](SECURITY.md)
- **貢獻方式**：請見 [CONTRIBUTING.md](CONTRIBUTING.md)
