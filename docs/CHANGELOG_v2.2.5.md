# v2.2.5-feistel-research — structural weakness repair

Date: 2026-11-09. Research revision; no production or post-quantum certification.

v2.2.4 applied the same nonlinear mask to both state halves. Combined with its
output routing, this preserved two public parity functions and three
probability-one differences through all rounds. v2.2.5 replaces that transition
with `(L,R) -> (R,L XOR H_r(R))`, with the corresponding inverse. ATCG transcription
and the engineering KDF are retained to isolate the substrate change.

## Changes

- Shared `feistel_function` for encryption/decryption; removed the output twist
  and its rotation helpers/cache fields.
- Added `test_v225_regressions`: 1–32-round roundtrip/in-place coverage and
  16–32-round checks against dense-mask affine relations and constant differences.
- Added `test_affine_rank`: rank of `[plaintext,ciphertext,1]` across all 640 data
  bits, for four policies and four sessions each at the default 16 rounds.
- Added `test_round_paths`: unrolled/generic encryption and decryption equivalence.
- Extended batch tests to 1–32 rounds, in-place buffers and empty batches.
- Froze 20 ciphertext vectors in `tests/kat_expected_v225.h`; ordinary KAT execution
  now checks them, not just roundtrip. `--generate` explicitly disables comparison
  and must not be used as a validation step.
- Integrated tests into Make, CTest and CI. `BUILD` can be overridden for isolated
  builds (relative or absolute paths). `fast-bench` creates the build directory. CI compares documented ciphertexts in scenario/policy order.
- Updated the substrate specification and corrected the framework's explanation
  of inverse-round correctness. Previous audit/KAT/substrate records are retained
  as history. New diffusion data: `QRES_v2.2.5.csv`.

## Breaking changes

All 20 existing ciphertext vectors differ. Function signatures and input lengths
are retained, but ciphertext semantics and `spiral_atcg_ctx_t` layout are not
backward compatible. Rebuild consumers; do not mix versions. Raw ciphertext has
no version marker or tag, so the library cannot detect a wrong version. Old data
requires its original implementation; do not overwrite it based on an unchecked
v2.2.5 decryption result.

## Validation and remaining work

See [VALIDATION_v2.2.5.md](VALIDATION_v2.2.5.md) for executed tests, negative controls,
reduced-round behavior and platform limitations. These results do not establish
AES-256-equivalent or superior security, 128-bit quantum security, or a
production-safe KDF/AEAD.

Further research must address the KDF, biased and nonlinear trails, weak keys,
round-key relationships, side channels, quantum attack models and independent
review. Increasing key length or round count alone would not have repaired the
v2.2.4 algebraic weaknesses.

中文：本版修復已知的雙側同遮罩結構，補上可拒絕舊版的回歸測試與真正固定的
KAT。密文與 context 結構不相容。這是可繼續分析的研究版本，不是安全認證。
