# SPIRAL-ATCG ATCG v2 Spec

**Spiral-ATCG: a material-driven block cipher under the ATCG meta-material model**

版本：v2.0  (ATCG meta-material model component of Spiral-ATCG v3)
狀態：規格定案草案（spec freeze candidate）
取代：`SPIRAL_ATCG_META_MATERIAL_MODEL.md` v1.0 (concept draft)
配套：`Spiral-ATCG Technical Specification v3`、`SPIRAL_V3_NAMING_PROPOSAL.md`

---

## 0. 文件目的

本文件正式定義 **Spiral-ATCG** 的 ATCG meta-material model，作為 Spiral-ATCG round material 組織原理的規格組件。本文件的目標是把 v1 概念草案中以箭頭與敘事描述的部分，全部 cash out 成可實作、可驗證、可逆的 algorithmic spec。

Spiral-ATCG 是 Spiral 家族在 v3 階段的正式名稱：a material-driven 320-bit block cipher organized under the ATCG meta-material model。本文件涵蓋 ATCG 上層材料模型的部分；substrate round function 規格見配套的 *Spiral-ATCG Technical Specification v3*。

本文件範圍：

- alphabet, complement rule, helicity rule
- 從 session inputs 到 round materials 的完整 transcription pipeline
- encrypt / decrypt 的對稱模型
- 與既有 `RC / LM / SK / CD` 材料族的正式對應

本文件 **不** 涵蓋：

- substrate round function `SpiralRound` 的內部規格（見配套 *Spiral-ATCG Technical Specification v3*，繼承自 `Spiral_Technical_Specification_v1.docx`）
- 形式化安全證明（empirical 安全敘事另見 *Spiral-ATCG Quantum Margin v3* 與 *Spiral-ATCG QRES v1 Summary*）
- protocol 層 lifting

---

## 1. 字母表與下層材料族

### 1.1 ATCG alphabet

```
Σ_ATCG = { A, T, C, G }
```

語義：

| 字母 | 名稱 | 角色 |
|---|---|---|
| **A** | Anchor | 結構錨點、context binding、deterministic alignment |
| **T** | Twist | 螺旋扭結、helicity interaction、local rotational mutation |
| **C** | Couple | 鏈間配對、cross-lane coupling、structural adjacency |
| **G** | Gate | 條件觸發、subkey-like gating、nonlinearity injection |

### 1.2 下層材料族

```
Μ = { LM, RC, CD, SK }
```

語義（沿用既有定義）：

| 符號 | 全名 | 角色 |
|---|---|---|
| **LM** | Lane Mask | per-(r, i) lane positional 材料 |
| **RC** | Round Constant | per-(r, i) rotation/twist injection 材料 |
| **CD** | Cross-lane Drift | per-(r, i) cross-lane coupling 材料 |
| **SK** | Round Subkey | per-(r, i) conditional subkey 材料 |

---

## 2. Production rules（ATCG → 下層材料）

ATCG 不直接產出 round-level word，而是經由 **typed production rules** 派生 `LM / RC / CD / SK`。本 spec 採用以下 production rule（v2 定案）：

```
LM := A
RC := T ∘ C
CD := C ∘ A
SK := G
```

說明：

- `LM := A`：anchor-class material 直接落到 lane mask。
- `RC := T ∘ C`：round constant 是 twist 經由 couple 組織後的合成材料。`∘` 表 ordered composition：先取 twist 再受 couple 約束。
- `CD := C ∘ A`：cross-lane drift 是 couple 受 anchor 約束的合成材料。
- `SK := G`：gate-class material 直接落到 round subkey。

> **設計取捨筆記**：本 spec 不採用 1-to-1 mapping，因為 RC / CD 確實兼具兩個 ATCG 字母的語義。直接用 1-to-1 會產生 "RC 兼具 T 與 C" 這種模糊敘述。Production rule 用 ordered composition 把這個語義 cash out 成 deterministic function，而 reviewer 可以對 production rule 提問（「順序為什麼是 T 在 C 之前」）但無法質疑 rule 本身的存在。

### 2.1 Composition 操作的 spec

`∘` 在本 spec 中指 typed composition operator，必須滿足以下三個性質：

1. **Determinism**：給定輸入，輸出唯一。
2. **Associativity not required**：`X ∘ Y` 跟 `Y ∘ X` 一般不同；composition 是有方向的。
3. **Type preservation**：`X ∘ Y` 的輸出仍是 ATCG-space 的 typed token，可進一步 compose 或被 production rule 消化。

具體實作見 §6.3 的 `Compose_TC`, `Compose_CA` 函數定義。

---

## 3. Complement rule

固定為：

```
comp(A) = T
comp(T) = A
comp(C) = G
comp(G) = C
```

互補不是 byte-level equality，而是 **policy-defined compatibility**。在最 minimal 的 BASE policy 下，互補可以 instantiate 為 byte-wise XOR-with-domain-tag 或類似的 deterministic involution（見 §6.5）。SAFE / AGGR policy 可以用更強或更弱的 compatibility predicate。

### 3.1 為什麼 A↔T, C↔G 不是 A↔C, T↔G

A 跟 T 配對的依據是「結構錨定 vs 局部扭結」的對偶 — anchor 提供穩定參照，twist 提供 perturbation，兩者一強一弱。C 跟 G 配對的依據是「鏈間關聯 vs 條件門控」的對偶 — couple 處理多 lane 互動，gate 處理單 lane 條件觸發。

A↔C 不選的原因是 anchor 跟 couple 都偏「結構性」材料，缺乏對偶；T↔G 不選的原因類似，twist 跟 gate 都偏「動態性」材料。

---

## 4. Helicity rule（antiparallel pairing）

本 spec 採用 antiparallel helicity rule：

```
H(r) = R - 1 - r
```

也就是 encrypt 第 `r` 輪使用主鏈位置 `r`，**同時** 配對變異鏈位置 `R - 1 - r`。

正式的 round-`r` pair 寫成：

```
Pair_r = (S_main[r], S_var[R - 1 - r])
```

### 4.1 Helicity rule 的三個 invariant

H(r) = R - 1 - r 的選擇同時滿足：

1. **DNA antiparallel**：兩條 strand 方向相反，這是 DNA 雙螺旋的真實物理性質。
2. **Decrypt reverse-round alignment**：encrypt 走 r = 0..R-1，decrypt 走 r = R-1..0。在 decrypt 第 r' = R-1-r 輪時，pair 變成 `(S_main[R-1-r], S_var[r])`，這跟 encrypt 第 r 輪的 pair `(S_main[r], S_var[R-1-r])` 在「使用了同樣兩個 strand 元素」的意義上一致 — 只是讀取順序顛倒。decrypt 不需要重新計算 pair。
3. **Self-inverse**：`H(H(r)) = R - 1 - (R - 1 - r) = r`。helicity rule 是 involution，這保證 decrypt 端對 helicity 的處理跟 encrypt 端對稱，沒有額外推導需求。

### 4.2 為什麼這條規則是 paper 的 selling point

DNA 敘事在 v1 草案是純 inspiration。在本 spec 下，complement rule + helicity rule 同時生效後，**decrypt 的 reverse round ordering 不再是 ad-hoc engineering choice，而是 helicity rule 的直接後果**。換句話說，paper 不需要單獨論證 decrypt 為什麼要 reverse round；reverse round 本身就是 antiparallel helicity 的推論。

---

## 5. Transcription pipeline 三段式

本 spec 把 (master_seed, nonce, session_id) → (LM_r, RC_r, CD_r, SK_r) 拆成三個正式步驟：

```
Step A.  ATCG_Project        : session inputs → (S_main, S_var)
Step B.  ATCG_Pair           : (S_main, S_var, r) → P_r
Step C.  ATCG_TranscribeRound: P_r → (LM_r, RC_r, CD_r, SK_r)
```

每一步都是 deterministic、stateless（Step A 之後）、並提供 inverse-friendly 性質。

---

## 6. Pseudo-code

### 6.1 Step A — ATCG_Project

```
Algorithm ATCG_Project
Input:
    master_seed   : bytes[32]
    nonce         : bytes[16]
    session_id    : bytes[16]
    rounds R      : uint
Output:
    S_main[0..R-1] : array of ATCG_TokenSet
    S_var [0..R-1] : array of ATCG_TokenSet

1. root_main ← KDF(master_seed || DOMAIN_TAG_MAIN)
2. root_var  ← KDF(master_seed || nonce || session_id || DOMAIN_TAG_VAR)
3. for r in 0..R-1:
       S_main[r] ← ATCG_Expand(root_main, r)
       S_var [r] ← ATCG_Expand(root_var,  r)
4. return (S_main, S_var)
```

設計 invariants：

- `root_main` **不** 含 nonce 或 session_id：master strand 是 session-invariant，這對「同一 master_seed 跨 session 的 helicity 結構」保持一致。
- `root_var` 含 nonce 與 session_id：variable strand 是 session-bound，這是 per-session 擾動的來源。
- `DOMAIN_TAG_MAIN` 與 `DOMAIN_TAG_VAR` 必須是兩個 byte-distinct 的 domain separation tag。建議：
  - `DOMAIN_TAG_MAIN = "SPIRAL.ATCG.STRAND.MAIN.v2"` (26 bytes)
  - `DOMAIN_TAG_VAR  = "SPIRAL.ATCG.STRAND.VAR.v2"`  (25 bytes)
- KDF 在 BASE policy 下 instantiate 為現行 MCMF context derivation；spec 不綁死 KDF 演算法，但要求：(a) collision-resistant、(b) byte-domain-separation-aware、(c) deterministic。

### 6.2 Step B — ATCG_Pair（含 helicity rule）

```
Algorithm ATCG_Pair
Input:
    S_main[0..R-1], S_var[0..R-1]
    round r : uint, 0 <= r < R
Output:
    P_r : (ATCG_TokenSet, ATCG_TokenSet)

1. j ← R - 1 - r
2. P_r ← ( S_main[r], S_var[j] )
3. return P_r
```

> **Spec 鎖定**：line 1 是 antiparallel helicity rule，**不可** 改成 `j = r`（parallel）或其他 `j = f(r)`。任何 alternative pairing 都要產生 v3 spec。

### 6.3 Step C — ATCG_TranscribeRound

```
Algorithm ATCG_TranscribeRound
Input:
    S_main[0..R-1], S_var[0..R-1]
    round r : uint
Output:
    LM_r, RC_r, CD_r, SK_r : per-lane material vectors

1. P_r ← ATCG_Pair(S_main, S_var, r)
2. a_r ← Extract_A(P_r)
3. t_r ← Extract_T(P_r)
4. c_r ← Extract_C(P_r)
5. g_r ← Extract_G(P_r)
6. LM_r ← Produce_LM(a_r)
7. RC_r ← Produce_RC( Compose_TC(t_r, c_r) )
8. CD_r ← Produce_CD( Compose_CA(c_r, a_r) )
9. SK_r ← Produce_SK(g_r)
10. return (LM_r, RC_r, CD_r, SK_r)
```

對應到 §2 的 production rules：

```
Line 6 :  LM := A
Line 7 :  RC := T ∘ C   (注意順序：先 T 再 C)
Line 8 :  CD := C ∘ A   (注意順序：先 C 再 A)
Line 9 :  SK := G
```

### 6.4 Extract / Produce 函數規範

`Extract_X(P_r)` 從 `P_r = (S_main[r], S_var[R-1-r])` 中萃取 X-class typed material。Spec 不綁死實作，但要求：

| 函數 | 輸入 | 輸出 | 性質 |
|---|---|---|---|
| `Extract_A(P_r)` | pair tuple | A-token | deterministic |
| `Extract_T(P_r)` | pair tuple | T-token | deterministic |
| `Extract_C(P_r)` | pair tuple | C-token | deterministic |
| `Extract_G(P_r)` | pair tuple | G-token | deterministic |
| `Compose_TC(t, c)` | T-token, C-token | TC-composite | type-preserving |
| `Compose_CA(c, a)` | C-token, A-token | CA-composite | type-preserving |
| `Produce_LM(a)` | A-token | LM material vector | deterministic |
| `Produce_RC(tc)` | TC-composite | RC material vector | deterministic |
| `Produce_CD(ca)` | CA-composite | CD material vector | deterministic |
| `Produce_SK(g)` | G-token | SK material vector | deterministic |

### 6.5 Complement compatibility check（optional, for SAFE policy）

```
Algorithm ATCG_ComplementCheck
Input:  P_r = (X, Y)
Output: bool

1. return Compatible(Y, comp(X))
```

`Compatible(·, ·)` 在 BASE policy 不檢查（永遠回傳 true）；在 SAFE policy 可以 instantiate 為 e.g. byte-wise XOR domain tag check。AGGR policy 可以放鬆甚至 disable。

---

## 7. Encrypt / Decrypt 對稱模型

### 7.1 Encrypt

```
Algorithm Spiral_Encrypt_MaterialDriven
Input:
    master_seed, nonce, session_id : bytes
    plaintext_state                : 320-bit state
    rounds R                        : uint
Output:
    ciphertext_state : 320-bit state

1. (S_main, S_var) ← ATCG_Project(master_seed, nonce, session_id, R)
2. state ← plaintext_state
3. for r in 0..R-1:
       (LM_r, RC_r, CD_r, SK_r) ← ATCG_TranscribeRound(S_main, S_var, r)
       state ← SpiralRound(state, LM_r, RC_r, CD_r, SK_r)
4. return state
```

### 7.2 Decrypt

```
Algorithm Spiral_Decrypt_MaterialDriven
Input:
    master_seed, nonce, session_id : bytes
    ciphertext_state               : 320-bit state
    rounds R                        : uint
Output:
    plaintext_state : 320-bit state

1. (S_main, S_var) ← ATCG_Project(master_seed, nonce, session_id, R)
2. state ← ciphertext_state
3. for r in R-1..0:
       (LM_r, RC_r, CD_r, SK_r) ← ATCG_TranscribeRound(S_main, S_var, r)
       state ← InvSpiralRound(state, LM_r, RC_r, CD_r, SK_r)
4. return state
```

### 7.3 Decrypt correctness 來自 helicity rule

Decrypt 第 `r' = R-1-r` 輪呼叫 `ATCG_TranscribeRound(S_main, S_var, r')`，內部 `ATCG_Pair` 算 `j = R - 1 - r' = R - 1 - (R - 1 - r) = r`，所以 pair 變成：

```
P_{r'} = ( S_main[R-1-r], S_var[r] )
```

而 encrypt 第 `r` 輪的 pair 是：

```
P_r = ( S_main[r], S_var[R-1-r] )
```

兩個 pair **使用了同樣兩個 strand 元素**，只是 first/second 位置交換。如果 `Extract_*` 跟 `Produce_*` 函數對 pair 的兩個位置 **對稱**（i.e., 不依賴 first/second 區分），那麼 decrypt 第 `r'` 輪產生的 round material 跟 encrypt 第 `r` 輪產生的相同。這是 decrypt round-trip 正確性的結構基礎。

> **Spec invariant**: `Extract_*` 跟 `Produce_*` 必須 **pair-symmetric**。實作必須驗證這個性質，否則 helicity rule 失效。

---

## 8. Transcription policy

ATCG framework 之下可以有多個 transcription policy。本 spec 認可四個：

| Policy | 對外正式名 | Legacy 命名（過渡期） | 用途 |
|---|---|---|---|
| **BASE** | `Spiral-ATCG/BASE` | `REAL_V2_HOOKED_T_RC_SK` + `T_RC_SK_BASE` preset | 主研究線、QRES v1 主預設 |
| **SAFE** | `Spiral-ATCG/SAFE` | `T_RC_SK_SAFE` 派生 | 保守候選；額外啟用 `ATCG_ComplementCheck` |
| **AGGR** | `Spiral-ATCG/AGGR` | `T_RC_SK_AGGR` 派生 | 激進候選；放寬 compatibility |
| **NULL** | `Spiral-ATCG/NULL` | `REAL_V2` (純 KDF) | engineering baseline；只用 `LM_r ← Produce_LM(...)` 派生整個 expanded key 一次，其他 material 不在 round 中注入 |

NULL policy 對應之前的 REAL_V2 backend：把 ATCG transcription 的整體輸出 collapse 成 40-byte expanded key 一次給 substrate 用，等同於傳統 cipher 的 standard key schedule mode。

---

## 9. 與 v1 概念草案的對應

| v1 概念 | v2 spec 對應 |
|---|---|
| ATCG alphabet 概念 | §1.1 |
| ATCG → RC/LM/SK/CD「兼具」對應 (N-to-N) | §2 production rules（換成 ordered composition） |
| 「DNA 雙螺旋」敘事 | §4 helicity rule (`H(r) = R-1-r`) |
| 「complement rule A↔T, C↔G」 | §3，policy-defined compatibility |
| 「轉錄模型」三步驟 | §5, §6 三段 algorithm pseudo-code |
| 「主鏈 / 變異鏈」雙鏈 | §6.1 `S_main` / `S_var` 跟 domain tag 拆分 |
| BASE / SAFE / AGGR policy | §8 transcription policy |

---

## 10. 與既有研究資產的相容性

| 資產 | 影響 |
|---|---|
| QRES v1 differential probe 數據 | **不失效**。QRES 量的是下層材料政策的 round-by-round 行為，ATCG framing 不改變下層 byte-level 計算，只重新組織敘事。 |
| `T_RC_SK_BASE` preset | **不失效**。重新理解為「BASE transcription policy 在 ATCG framework 下的 instantiation」。 |
| `REAL_V2` engineering baseline | **不失效**。重新理解為「NULL transcription policy 的 instantiation」。 |
| Decrypt patch (this work) | **完全相容**。decrypt 的 reverse round ordering 在 v2 spec 下變成 helicity rule 的直接後果。 |
| Test vectors v1.1 (五條 TV) | **不失效**。同一個 cipher，同一個下層 material 計算路徑。 |

---

## 11. Spec freeze invariant

本 spec 一旦凍結為 v2，以下任何變動都需要產生 v3：

1. ATCG alphabet 字母數量或語義變動
2. Production rule 變動（包括 composition 順序）
3. Complement rule 變動
4. Helicity rule 變動（含 `H(r)` 函數）
5. Transcription pipeline 步驟數量或順序變動

下列變動 **不** 觸發 v3，可以在同一 spec 下做 minor revision：

- `Extract_*` / `Produce_*` 函數的 byte-level 實作細節
- KDF instantiation（只要符合 §6.1 的三個性質）
- `Compose_TC` / `Compose_CA` 的具體實作（只要符合 §2.1 的三個性質）
- 新增 transcription policy（BASE / SAFE / AGGR / NULL 之外）

---

## 12. Paper-ready contribution statement

> We present **Spiral-ATCG**, a material-driven 320-bit block cipher organized under the ATCG meta-material model. The model defines: (1) a four-letter material alphabet `Σ_ATCG = {A, T, C, G}` with semantic roles anchor / twist / couple / gate; (2) a complement rule `A↔T, C↔G` with policy-defined compatibility; (3) an antiparallel helicity rule `H(r) = R - 1 - r` pairing round `r` on the main strand with round `R - 1 - r` on the variable strand; and (4) typed production rules `LM := A`, `RC := T ∘ C`, `CD := C ∘ A`, `SK := G` that deterministically transcribe paired ATCG strands into concrete round materials. We instantiate Spiral-ATCG under the BASE transcription policy as the primary research line, with NULL / SAFE / AGGR policies as engineering baseline and design-space candidates. Decryption correctness follows directly from the helicity rule combined with `Extract` / `Produce` pair-symmetry, without requiring inverse round components.

---

## 13. 接口命名建議（給後續工程實作用）

對應 §6 的 algorithm，建議公開 API 命名：

```c
/* Step A */
spiral_status_t spiral_atcg_project(
    const uint8_t master_seed[32],
    const uint8_t nonce[16],
    const uint8_t session_id[16],
    uint32_t rounds,
    spiral_atcg_strands_t *out);

/* Step B */
spiral_status_t spiral_atcg_pair(
    const spiral_atcg_strands_t *strands,
    uint32_t r,
    spiral_atcg_pair_t *out_pair);

/* Step C */
spiral_status_t spiral_atcg_transcribe_round(
    const spiral_atcg_strands_t *strands,
    uint32_t r,
    spiral_round_material_t *out);  /* 含 LM, RC, CD, SK */
```

對應的 context type:

```c
typedef struct {
    /* Per-round ATCG token sets, R rounds. */
    spiral_atcg_tokenset_t main_strand[SPIRAL_MAX_ROUNDS];
    spiral_atcg_tokenset_t var_strand [SPIRAL_MAX_ROUNDS];
    uint32_t rounds;
} spiral_atcg_strands_t;
```

---

## 14. Spec metadata

- 文件正式名：Spiral-ATCG ATCG v2 Spec
- 文件版本：v2.0
- 母品牌：Spiral-ATCG（演算法家族 v3 階段正式名）
- 狀態：spec freeze candidate
- 取代：v1.0 concept draft
- 作者：本 spec 為 Spiral-ATCG 研究線階段性整合
- 配套文件：
  - *Spiral-ATCG Technical Specification v3*（substrate cipher 規格）
  - *Spiral-ATCG Quantum Margin v3*（取代 `QRES_quantum_margin.md`，empirical quantum-margin narrative）
  - *Spiral-ATCG QRES v1 Summary*（QRES 比較數據）
  - *SPIRAL_V3_NAMING_PROPOSAL.md*（正式命名規範）
  - decrypt path 實作 patch（已附）

---

## 15. 凍結這個 spec 之後接下來要做的事

| 順序 | 工作 | 預期產出 |
|---|---|---|
| 1 | 把 §6 的 algorithm 實作成 C code，把現有 MCMF code refactor 為 ATCG 介面 | `spiral_atcg.c`, `spiral_atcg.h`（API prefix `spiral_atcg_*`） |
| 2 | 確認既有 5 條 TV 在 v3 spec 下產生相同 ciphertext | `Spiral-ATCG KAT v3` |
| 3 | 跑既有 QRES probe，確認下層材料數值未變 | `qres_atcg_v3_passthrough.csv` |
| 4 | 重畫 paper 圖 1：`ATCG_Project → ATCG_Pair → ATCG_TranscribeRound → SpiralRound` 一條 vertical pipeline | `paper_figure_1_v3.svg` |
| 5 | 改寫 paper §1 introduction 用 §12 的 contribution statement 開場 | `paper_intro_v3.md` |
| 6 | 改寫 paper §3 從「meta layer + substrate layer」變成「Spiral-ATCG framework with BASE policy as primary instantiation」 | `paper_section_3_v3.md` |

工作量估計：1-2 兩天可以完成，3-6 各半天到一天。

---

## 16. Implementation conformance — Spiral-ATCG v2.1.1-fast

This spec was originally written as a freeze candidate independent of any specific implementation. Since freeze, two engineering implementations have been produced:

| Implementation | Status | Notes |
|---|---|---|
| `spiral_atcg_v0_2_policy_project` | **superseded** | Predecessor; lacked strands, helicity, production rules |
| `spiral_atcg_v2_1_fast_project` | **superseded by v2.1.1** | First full v2 spec impl; had a substrate diffusion bug (fixed pair matching) |
| `spiral_atcg_v2_1_1_fast_project` | **conformant reference** | Spec-conformant, diffusion-correct, all four invariant tests pass |

### 16.1 Conformance to spec invariants

`spiral_atcg_v2_1_1_fast` implements every invariant defined in §1 through §11:

| Spec invariant | Implementation site | Verified by |
|---|---|---|
| §1 ATCG alphabet | `spiral_atcg_tokenset_t` (4 fields A/T/C/G) | source inspection |
| §2 Production rules `LM:=A, RC:=T∘C, CD:=C∘A, SK:=G` | `spiral_atcg_transcribe_pair`, lines producing `out->LM/RC/CD/SK` | source inspection |
| §3 Complement rule (policy-defined compatibility) | `complement_check` invoked under SAFE policy | source inspection |
| §4 Helicity rule `H(r) = R - 1 - r` | `spiral_atcg_pair`, line `j = strands->rounds - 1u - r` | `test_material_trace` output: `r=00 pair=15, r=01 pair=14, ...` |
| §5/§6 Three-step pipeline | `spiral_atcg_project` / `_pair` / `_transcribe_round` | direct API mapping |
| §7.3 Pair-symmetric extraction | `pair_sym(a, b, ...)` helper using XOR + ADD only | `test_pair_symmetry` (4 policies × 16 rounds) |
| §8 Four policies BASE / SAFE / AGGR / NULL | `spiral_atcg_policy_t` enum + `transcribe_pair` switch | `test_roundtrip` (all 4 policies) |
| Roundtrip correctness | `spiral_round` / `inv_spiral_round` symmetry | `test_roundtrip` (all 4 policies × 16 rounds) |
| Avalanche (not a v2 spec invariant per se, but required for any block cipher) | substrate `spiral_round` includes round-dependent lane shuffle | `test_avalanche` + thorough_avalanche (2560 cells per policy, all-10-lanes 100%) |

### 16.2 v2.1.1 substrate fix vs v2.1

v2.1-fast had a substrate-only diffusion bug: the `spiral_round` Couple phase used a fixed pair matching `(0,9), (1,8), (2,7), (3,6), (4,5)`, with no other cross-lane state mixing. This caused state to form 5 closed two-lane components, so a 1-bit plaintext change in lane 0 would not propagate to lanes 1..8 even after 16 rounds.

v2.1.1 adds a single round-dependent bijective lane shuffle between Phase 1 and Phase 2:

```c
tmp[i] = s[(i * 3 + r * 7) % 10]
```

Stride-3 is coprime to 10 (so `i*3 mod 10` is a bijection); the `r*7` shift makes the permutation round-dependent. The inverse is computed as `tmp[(i*3 + r*7) % 10] = s[i]` and applied in `inv_spiral_round`.

This fix:
- Does NOT change the ATCG alphabet, grammar, helicity, or transcription pipeline (Layers 1–3).
- Does NOT change the production rules.
- Does NOT change policy semantics.
- ONLY changes the substrate `SpiralRound` (Layer 4).

Therefore v2.1.1 is fully conformant with this v2.0 spec.

### 16.3 Engineering note: precompute-at-session-init

Both v2.1 and v2.1.1 precompute the per-round materials `LM_r, RC_r, CD_r, SK_r` once at `spiral_atcg_init_session()` rather than recomputing them per-block. This is an optimization, not a spec change: the precompute is just memoization of the deterministic `ATCG_TranscribeRound`. Per-session setup cost is roughly 100–300 µs depending on policy; per-block encrypt cost is roughly 7.6 MiB/s reference (unoptimized) on a typical x86-64 sandbox.

### 16.4 KAT vectors

Authoritative KAT vectors for v2.1.1 are published in `spiral_atcg_v211_kat_v1.md` — five test scenarios (TV1–TV5) × four policies (BASE/SAFE/AGGR/NULL) = 20 entries, all verified roundtrip-correct.

---

## 17. Implementation conformance update — Spiral-ATCG v2.2-strength

`spiral_atcg_v2_2_strength` supersedes `spiral_atcg_v2_1_1_fast` as the active strength-oriented reference implementation.

The ATCG framework remains unchanged:

```text
ATCG_Project -> ATCG_Pair -> ATCG_TranscribeRound -> LM/RC/CD/SK -> SpiralRound
```

The v2.2 change is limited to the internal substrate `SpiralRound / InvSpiralRound`. The round function now uses a Spiral-DNA-320 v1-inspired reversible half-state structure:

- `S = L ^ R` dual-input path
- round-dependent lane references
- F1/F2 stride-3 round-dependent permutations
- SpiralMix_v2-style five-lane butterfly
- round-dependent output twist via `alpha(i,r)` and `beta(i,r)`

This restores reduced-round diffusion behavior to the same class as the earlier v1 substrate. The generated QRES v2.2 CSV reports input-probe average HD near 160 and 100% all-lane diffusion at rounds 2, 3, 4, 6, 8, and 12 for all four policies.

Current authoritative KAT for this implementation:

```text
docs/spiral_atcg_v22_kat_v1.md
```

Current QRES output:

```text
docs/spiral_atcg_qres_v2_2_strength_comparison.csv
```
