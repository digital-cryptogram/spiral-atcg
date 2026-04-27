# Contributing to Spiral-ATCG

Thank you for your interest in contributing.

This document describes the kinds of contribution we welcome and how to submit them. Spiral-ATCG is an open research project, but it is also tied to Digital Cryptogram's commercial brand, so we maintain a slightly more conservative process than typical open-source projects.

## Code of conduct

Be technical, be specific, be respectful. We expect contributors to engage with the spec and the audit reports rather than rely on generic claims. Disagreement is welcome; flames are not.

## Kinds of contributions

### Highly welcome

- **Cryptanalytic findings** beyond what the audit reports already document. See [SECURITY.md](SECURITY.md) for the disclosure process.
- **Independent reimplementation** in another language (Python, Rust, Go, etc.) that bit-exactly matches the C reference KAT.
- **Reproducibility verification**: building on a new platform / compiler / OS and reporting that all tests pass (or fail).
- **Documentation improvements**: clarifications, typo fixes, additional examples, translations beyond English / 中文.
- **Performance contributions**: SIMD ports (AVX2, NEON), FPGA reference, GPU implementation. These should not change the cipher semantics — bit-exact KAT match is required.
- **Portability fixes**: build issues on platforms we have not tested.
- **Test additions**: new probe families, attack-class coverage we have not addressed.

### Welcome but discuss first

- **API extensions**: new public functions in `include/spiral_atcg.h`. These need to fit cleanly with the existing API and have a use case justification.
- **Build system changes**: please open an issue before submitting large refactors of `Makefile` or `CMakeLists.txt`.
- **New transcription policies** beyond `BASE`/`SAFE`/`AGGR`/`NULL`: these are spec-level changes and need design discussion.

### Not in scope

- **Substrate cipher changes** that would alter KAT: these change the cipher semantics and are spec-version events, not patches. If you have an idea for a substrate change, open an issue for design discussion before any code.
- **Removing the audit reports or KAT** from the repo: these are reproducibility commitments.
- **Removing the disclaimer language** from README/SECURITY.md: this language is intentional.
- **Adding cryptocurrency / blockchain / token features**: out of scope for this project.

## Contribution process

### For small fixes (typos, doc, single-file bug fix)

1. Fork the repository.
2. Create a branch: `git checkout -b fix/short-description`.
3. Make your change.
4. Verify tests still pass: `make all && make test && make full`.
5. Open a pull request with a clear title and description.

### For substantive changes (new probe, new platform port, performance work)

1. **Open an issue first** describing what you want to do.
2. Wait for feedback before investing significant time.
3. Once approved, follow the small-fix process above.
4. Larger changes will get more review iterations — this is normal and not personal.

### For cryptanalytic findings

See [SECURITY.md](SECURITY.md). Findings should go through the disclosure process, not through public pull requests.

## Coding style

- **C source**: follow the existing style. C11, 4-space indent, `snake_case` for functions and variables, `UPPER_CASE` for macros and constants.
- **Build clean with**: `-std=c11 -Wall -Wextra -Wpedantic`. The CI enforces this.
- **No runtime dependencies** beyond `libc` and `libm` (used by some test programs only). The reference implementation is intentionally portable C.
- **No new test framework**: tests follow the existing pattern of standalone C programs that print `PASSED` / `FAILED`.

## Documentation style

- **English first**, 中文 second where bilingual is appropriate (README, top-level docs).
- **Spec terminology**: use the names defined in [`docs/SPEC.md`](docs/SPEC.md) (Layer 1–4, ATCG, transcription policy, etc.) consistently.
- **Be honest about scope**: do not introduce wording that overstates security claims. If you find such wording in existing docs, please flag it in an issue.

## Sign-off and licensing

By submitting a contribution you agree that:

1. Your contribution is licensed under the same Apache 2.0 license as the rest of the project (see [LICENSE](LICENSE)).
2. You have the right to submit the contribution (e.g., if it was developed at work, you have employer permission).
3. You understand that Digital Cryptogram may incorporate your contribution into its commercial products under the Apache 2.0 patent grant.

We do not require a Contributor License Agreement (CLA). We do require that pull request commits include a `Signed-off-by:` line:

```
Signed-off-by: Your Name <your.email@example.com>
```

This certifies the [Developer Certificate of Origin (DCO)](https://developercertificate.org/).

You can add this automatically by using `git commit -s`.

## Recognition

Significant contributors are credited in:

- The relevant changelog (`docs/CHANGELOG_*.md`).
- The audit reports if their finding affected one.
- A future acknowledgments section if/when the project produces a paper.

## Questions

For general questions, open an issue with the `question` label.

For commercial inquiries (about Digital Cryptogram's products or partnerships), email sales_digi@digitalcryptogram.com.tw rather than opening a public issue.
