# Spiral-ATCG v2.2.4 安全性重新評估

本文保存 v2.2.4 升級前的稽核結果及重現方式；不描述目前 v2.2.5 的執行結果。舊版重現器 `tests/test_structural_invariants.c` 必須搭配舊版原始碼。

日期：2026-11-09。標的為本機 `spiral-atcg-main`，版本字串 `2.2.4-full-scalar-cleanup`。

## 判定

**目前版本不能宣稱強於 AES-256，也不能宣稱具有可靠的量子抗性。** 本次不是單純因為缺少安全證明而保留判斷：在完整 16 輪找到可證明、可重現的線性不變量與機率 1 差分，已破壞標準分組密碼需要的偽隨機置換（PRP）性質。

這些攻擊只需傳統電腦；全部四種政策受影響。增加輪數、換金鑰、換 nonce 或強化 KDF，都不會消除下述由輪結構造成的關係。

本次沒有完成主金鑰恢復，也沒有證明能恢復任意完整明文。因此不能把結果描述成「已破解所有資料」，也不能把洩漏兩個線性位元簡化為「仍有 254-bit 安全性」。區分攻擊、資訊洩漏與金鑰搜尋是不同安全目標。

## 1. 完整輪數的兩個線性不變量

程式位置：`src/spiral_atcg.c:364–383`，以及旋轉參數 `246–254`。

每輪的核心可整理為：

```text
S = L XOR R XOR session_constant
T = F_r(S; round_material)
YL = L XOR T
YR = R XOR T
output = 對 YL、YR 做 word 排列、左右交換與逐 word 旋轉
```

定義 `q0(X)` 為所有 10 個 32-bit word 的偶數位元位置（0,2,...,30）之 XOR；`q1(X)` 為奇數位置（1,3,...,31）之 XOR。亦即分別對整個 40-byte 區塊套用重複 `0x55`、`0xAA` 遮罩後取 parity。

1. 同一個 `T[i]` 同時 XOR 到左右兩半，因此在 `q0`、`q1` 中各出現兩次而抵消。
2. word 排列和左右交換不改變全區塊這兩組 XOR。
3. 所有輸出旋轉量都由 `... | 1` 產生，必為奇數；每個 word 的偶數與奇數位置互換。

因此對任何有效 session、任何輸入，每輪都滿足：

```text
q0(output) = q1(input)
q1(output) = q0(input)
```

預設 16 輪，以及任何偶數輪數，故有：

```text
q0(E_K(P)) = q0(P)
q1(E_K(P)) = q1(P)
```

奇數輪則交換這兩個值。這是代數恆等式，不是有限樣本的統計推測。

**影響：** 不知道金鑰也能從一個原始密文區塊直接讀出明文的兩個固定線性函數。不是兩個特定位元，而是各 160 個位元的 XOR。對均勻明文，這是兩個獨立資訊位元；對任意分布，則仍確定暴露這兩個函數值。

作為 PRP 區分器，只要查詢一個選定明文，再檢查兩條式子：Spiral 必然通過，均勻隨機的 320-bit 置換通過機率為 1/4，兩者接受機率相差 3/4。這已是非常強的低成本區分器。若直接把此區塊 API 當成訊息加密，選擇 `q0` 不同的兩個候選明文即可確定區分挑戰密文；此分析不是對尚未指定的所有外層協定一概宣稱可解密。

## 2. 完整輪數的機率 1 差分

令 `D55`、`DAA`、`DFF` 分別是 40 個 `0x55`、`0xAA`、`0xFF` 組成的區塊。

左右兩半同時 XOR 相同的逐 word 遮罩，不會改變 `S=L XOR R XOR constant`，因此非線性函數產生的 `T` 完全相同。輸出差分只經過公開的排列與旋轉。

所有奇數旋轉都把 `0x55555555` 與 `0xAAAAAAAA` 互換；`0xFFFFFFFF` 則不變。因此：

```text
偶數輪：E_K(P XOR D55) = E_K(P) XOR D55
        E_K(P XOR DAA) = E_K(P) XOR DAA
任何輪：E_K(P XOR DFF) = E_K(P) XOR DFF
奇數輪：D55 的輸出差分為 DAA，DAA 的輸出差分為 D55
```

預設 16 輪全部成立。不需要知道金鑰、round material 或 KDF 的內部值。對特定非零輸入差分，理想隨機 320-bit 置換在兩次查詢中恰好產生指定非零輸出差分的機率是 `1/(2^320-1)`；這裡是 1。

由一組已知明密文，可直接導出另外三組相關明密文，無須加密金鑰。這不是恢復任意明文或主金鑰的演算法，但已是全輪數確定性差分與可預測的相關密文修改關係。

對偶數輪定義 `g(P)=E_K(P) XOR P`，則 `D55`、`DAA` 都是 `g` 的 XOR period。這些週期公開、與金鑰無關，且未證明滿足 Simon 演算法特定 promise；**不能據此直接宣稱 Simon 可恢復主金鑰**。傳統區分攻擊本身已足以否定當前 PRP 安全性。

## 3. 重現結果

重現程式：[tests/test_structural_invariants.c](../tests/test_structural_invariants.c)。未修改密碼實作、header 或既有測試。

測試對每個政策、每個輪數 1–32，生成 4 組不同的 key/nonce/session，再測 32 個明文。使用公開 API，對所有呼叫檢查回傳狀態，並確認 roundtrip。

| 政策 | 兩個 parity 關係同時成立 | 三種確定差分成立 | Roundtrip |
|---|---:|---:|---:|
| BASE | 4,096 / 4,096 | 12,288 / 12,288 | 4,096 / 4,096 |
| SAFE | 4,096 / 4,096 | 12,288 / 12,288 | 4,096 / 4,096 |
| AGGR | 4,096 / 4,096 | 12,288 / 12,288 | 4,096 / 4,096 |
| NULL | 4,096 / 4,096 | 12,288 / 12,288 | 4,096 / 4,096 |
| 合計 | 16,384 / 16,384 | 49,152 / 49,152 | 16,384 / 16,384 |

使用 Apple Clang 17、arm64 macOS、`-O1 -fsanitize=address,undefined`；執行結束碼 0，無 sanitizer 錯誤。編譯器僅對原始實作的四個未使用 helper 發出警告。有限測試本身不證明所有輸入；全稱結論來自前兩節的代數推導。

本機既有 `make test` 也完成且通過。完整輸出分別存於 [structural-results.txt](validation-v2.2.4/structural-results.txt) 與 [existing-tests.txt](validation-v2.2.4/existing-tests.txt)。

從本儲存庫的 Git clone 根目錄重現。使用固定的 v2.2.4 commit
`2b2fe493e3d975dc12de380adfe93b48d965adb8`，避免誤用目前的 v2.2.5 原始碼。
這個 commit 的全部 44 個檔案已與升級前快照逐一比對 Git blob SHA，完全一致。
若使用 shallow clone，須先取得該 commit。

```sh
mkdir -p build-v224-repro/include build-v224-repro/src
git show 2b2fe493e3d975dc12de380adfe93b48d965adb8:src/spiral_atcg.c > build-v224-repro/src/spiral_atcg.c
git show 2b2fe493e3d975dc12de380adfe93b48d965adb8:include/spiral_atcg.h > build-v224-repro/include/spiral_atcg.h
clang -O1 -std=c11 -Wall -Wextra -Wpedantic \
  -fsanitize=address,undefined -fno-omit-frame-pointer \
  -Ibuild-v224-repro/include \
  tests/test_structural_invariants.c \
  build-v224-repro/src/spiral_atcg.c \
  -o build-v224-repro/test_structural_invariants
./build-v224-repro/test_structural_invariants
```

注意：新增程式回傳 0 代表「已成功重現弱點」，不是「密碼安全性通過」。它是漏洞重現器；未加入原有 `make test`。

檢查時檔案 SHA-256：

```text
src/spiral_atcg.c
e3afc1987b495e293083fbc5663ff25511550f3537fd4538c9dcfb564c614fc7
include/spiral_atcg.h
2a7a6447baca7cec9d75634e7c99c4cba84e1264c291d78a90a0a0f7749f5457
```

## 4. 與 AES-256 的合理比較

| 項目 | AES-256 | 本次 Spiral-ATCG |
|---|---|---|
| 主金鑰長度 | 256 bits | 32 bytes = 256 bits |
| 區塊長度 | 128 bits | 320 bits |
| 理想通用窮舉搜尋量級 | 約 `2^256` 次候選金鑰測試 | 最多同一搜尋空間；不代表已達此安全性 |
| 理想 Grover 搜尋量級 | 約 `2^128` 次 oracle 呼叫 | 只有在忽略其他弱點時才有同級通用估計 |
| 本次分析 | 作為已標準化比較基準 | 找到全輪數常數查詢區分器與線性洩漏 |

AES 的區塊大小與命名見 [NIST FIPS 197](https://csrc.nist.gov/pubs/fips/197/final)。Spiral 的參數來自本機 header。

**320-bit 區塊不代表 320-bit 金鑰強度。** 若主金鑰均勻隨機且 nonce/session 是公開值，展開成再多材料也不會增加主秘密的熵。大區塊可能改善特定操作模式的資料量界限，但這需要具體模式的分析，且不能補救本次的結構弱點。也不能從單一 `kdf_word` 使用 32-bit accumulator，直接推論整套密碼只剩 32-bit 金鑰；它有多個 domain/round/lane 呼叫，需另外分析其聯合結構。

表中的指數只描述通用搜尋，不是 AES 所有已知攻擊的精確最佳成本，也不是完整系統的安全位元數。量子 oracle 的可逆電路深度、閘數、錯誤修正、查詢資料與平行化限制均未量化；不能把 `2^128` oracle 次數當作 `2^128` 個廉價 CPU 操作。

## 5. 量子抗性判讀

對稱密碼可以討論後量子安全；這個概念並非只適用公開金鑰。AES-256 不會因為量子電腦就自動失效。[NIST PQC FAQ](https://csrc.nist.gov/projects/post-quantum-cryptography/faqs) 說明 Grover 的理想平方加速，以及序列執行、硬體成本和平行化等限制；目前仍允許繼續使用 AES，包括 AES-256。

但「金鑰 256 bits」只提供一個通用搜尋基準，並不證明未知結構攻擊不存在。必須區分：

- **Q1：** 攻擊者可做量子計算，但對真正加密服務作傳統查詢。
- **Q2：** 攻擊者能對加密 oracle 作量子疊加查詢，假設更強。

[Kaplan 等人的 CRYPTO 2016 論文](https://arxiv.org/abs/1602.05973) 展示了在疊加查詢模型下，部分對稱構造可遭量子週期攻擊；不能只看 Grover 就排除所有量子攻擊，也不能把該模型的結果無條件套用一般網路 API。

本次 Spiral 的問題在傳統模型已存在，量子攻擊者也能執行相同攻擊。因此當前版本無法滿足相應的量子 PRP 安全目標。**尚無根據給出「至少 128-bit 量子安全」或「比 AES-256 更抗量子」的結論。** 這不是宣稱已完成量子金鑰恢復。

## 6. 為何原本的測試沒有抓到

1. `tests/test_sec_battery.c:129–168` 的線性測試只選單一輸入 bit 與單一輸出 bit；本次遮罩每個含 160 個 bits，不在測試範圍內。
2. `tests/test_q_battery.c:104–108` 的 Simon-period probe 只試前 32 個單 bit 差分，不含跨完整區塊的 `D55/DAA/DFF`。
3. 同檔 `168–205` 的「Grover-margin」只有 4,096 個樣本，且僅比較密文前 64 bits；key、nonce、session 同時改變。沒有碰撞不能證明 256-bit 秘密熵，也不能量化 Grover 安全裕度。理想 64-bit 值在此樣本數的預期碰撞只有約 `4.55×10^-13`。
4. 同檔 `246–275` 的「q-IND-CPA advantage upper bound」實際只計算第一個密文字節的頻率偏差，沒有安全歸約或涵蓋所有攻擊者的上界。更根本地，任何置換對均勻明文都產生均勻密文，即使是完全不加密的恆等置換也一樣；此測試不足以判斷保密性。
5. Roundtrip 與 KAT roundtrip 證明的是可逆性和程式一致性。雪崩的平均 HD 接近 160 也無法排除多位元線性關係或結構化差分。

因此歷史報告的「未發現結構異常」只能保留為當時那些探針的結果，不能延伸到本次已找到的關係。

## 7. 其他需處理的設計事項

- `src/spiral_atcg.c:68–78` 明確把目前 KDF 標為工程測試用。它不是已標準化的密碼 KDF。本次未證明它可被有效反演；建議下一個研究版本採用明確 domain separation 的 HMAC/KMAC 等標準方案作比較。[NIST SP 800-108](https://csrc.nist.gov/pubs/sp/800/108/r1/upd1/final) 提供這類 KDF 的基礎規範。
- `encrypt_blocks` 只是同一 context 下逐區塊呼叫，沒有每區塊 tweak、tag 或驗證。直接拿來加密多區塊訊息會暴露重複區塊。這是原始 block API 的使用限制，不應與 AES-GCM 這種完整 AEAD 不對等比較。
- 資料相依旋轉等操作的側通道性質需針對平台與編譯輸出檢驗。本次未證明存在實用 timing 攻擊，也沒有聲稱完成 constant-time 稽核。
- `docs/SPEC.md` §7.3 將 pair 索引反轉解釋為同兩元素交換，並不普遍成立；`(main[R-1-r],var[r])` 不等於 `(var[R-1-r],main[r])`。實作正確解密實際使用 `inv_spiral_round` 及反向材料順序。此文件論證需另行修正，與上面的安全弱點獨立。

## 8. 下一個研究版本的必要工作

先重新設計狀態轉換，破除「同一 T 同時 XOR 左右半部，再只靠排列／旋轉輸出」產生的全輪數關係。不能只增加輪數或提高 KDF 強度；只改成混合奇偶旋轉，也仍保留總 parity 與全 1 差分，並非完整修復。

把本次的結構遮罩與差分加入新版本的負面安全測試，再做系統性的線性／差分路徑搜尋、旋轉關係、相關金鑰分析與獨立密碼分析。修掉這兩類弱點只代表移除已知反例，不代表新版本立即等同或超過 AES-256。若目標是實際保護資料，應使用成熟實作的 AES-256 AEAD；量子安全還需另行確認系統的金鑰建立與身分驗證層。

本報告是對提供版本的程式審查、代數分析與可重現實驗，不是外部認證，也沒有涵蓋未提供的商業版本或不同 substrate。
