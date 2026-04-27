# Spiral-ATCG v2.2.2 Fast Scalar Core

Status: implementation optimization only.

This revision keeps the v2.2-strength cipher semantics unchanged and adds a Level-2 scalar fast path.

## Scope

Unchanged:

- ATCG v2.0 transcription framework
- BASE / SAFE / AGGR / NULL policy definitions
- LM / RC / CD / SK material generation
- v2.2-strength substrate round mathematics
- 16-round default profile
- KAT output

Added:

- Fixed-16-round scalar dispatch for the default profile.
- Generic scalar fallback for non-default round counts.
- Batch API:
  - `spiral_atcg_encrypt_blocks(...)`
  - `spiral_atcg_decrypt_blocks(...)`
- Batch API regression test.
- Fast benchmark target.

## Security / correctness rule

The fast scalar core is required to be bit-exact with the existing block API. The batch API test verifies:

1. batch encryption equals repeated single-block encryption;
2. batch decryption restores the original plaintext;
3. all four policies pass.

Therefore this revision is a performance implementation change, not a cryptographic design change.

## Build / test

```bash
make clean
make test
make bench
make fast-bench
```

`make bench` uses the default debug/reference CFLAGS. `make fast-bench` uses `FAST_CFLAGS`, defaulting to `-O1` in this sandbox-friendly build.
