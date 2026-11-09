# Spiral-ATCG v2.2.5 experimental Feistel substrate

This incompatible research revision replaces the v2.2.4 simultaneous-mask
construction after exact full-round parity and differential relations were found.
See [release notes](CHANGELOG_v2.2.5.md), [validation](VALIDATION_v2.2.5.md),
and the [historical substrate](SUBSTRATE_v2.2.4.md).

## Parameters and encoding

- Block: 40 bytes, loaded as ten little-endian 32-bit words.
- Master seed: 32 bytes; nonce and session ID: 16 bytes each.
- Default: 16 rounds. The API accepts 1–32 for experiments; acceptance is not a
  security recommendation for a reduced-round profile.
- State: `L = words[0..4]`, `R = words[5..9]`.
- Policies: BASE, SAFE, AGGR, NULL. Projection/transcription and the engineering
  KDF are unchanged. `H_r` is determined by the session and round.
- Helper arithmetic is modulo `2^32`; rotate counts are modulo 32.

## Round and inverse

```text
Forward round r: (L, R)   -> (R, L XOR H_r(R))
Inverse round r: (L', R') -> (R' XOR H_r(L'), L')
```

Encryption applies rounds `0..R-1`; decryption applies their inverses in order
`R-1..0`. There is no additional swap after the final round. The final ten words
are serialized in little-endian order.

The forward left output equals the original right input, so the inverse can
recompute the same `H_r`. No inverse of `H_r` is needed. This proves correctness,
not pseudorandomness or security of the chosen `H_r`.

## Definition of H_r

`src/spiral_atcg.c:feistel_function` is the byte-level reference. It reuses the
previous nonlinear helpers but changes the input from `L XOR R` to `R`:

```text
S[i] = R[i] XOR round_cache[r].s_xor[i]
X = atcg_dual_input(S, material[r], r)
f1 = atcg_F1(X, round_cache[r].k, material[r], r)
f2 = atcg_F2(X, round_cache[r].k, material[r], r)
for i = 0..4:
    T[i] = f1[i] XOR f2[i] XOR CD[(i+r) mod 10]
    T[i] = T[i] XOR ROTL32(T[i], ((T[i] XOR r XOR RC[i]) AND 31) OR 1)
    T[i] = T[i] + ROTL32(T[i] XOR SK[(i+2) mod 10], ((r+7*i) AND 31) OR 1)
H_r(R) = atcg_spiral_mix_v2(T, material[r], r)
```

Each assignment uses the previous value on its right side. Only `k[5]` and
`s_xor[5]` remain in the round cache. The old output twist and alpha/beta/rotation
cache fields are removed. The unrolled 16-round path and generic path evaluate
identical rounds; `test_round_paths` compares both directions.

## Security scope

The previous invariant proof no longer applies: only one half receives `H_r`,
which is computed from the other half. Dedicated tests reject v2.2.4 and pass
v2.2.5 for the sampled sessions. Full-rank witness matrices also exclude all exact
affine plaintext/ciphertext relations for 16 tested full-round sessions.

This does not establish pseudorandomness of `H_r`, independence of round keys,
resistance to biased or nonlinear attacks, a quantified 16-round security margin,
side-channel resistance, or a quantum security level. A Feistel structure alone
does not transfer ideal-round-function theorems to these helpers.

## Compatibility

All 20 ciphertext vectors change, as does context/cache layout. Recompile callers;
do not serialize contexts or mix headers/libraries from different versions.
Store an algorithm version in any external container or protocol. The raw API
has neither version detection nor authentication: wrong-version decryption can
return success and meaningless bytes. Recover old research ciphertexts with the
matching historical implementation before re-encrypting.

This remains a block primitive, not an AEAD or a production encryption system.
