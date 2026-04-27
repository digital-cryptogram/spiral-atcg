# Security Policy

## Project status and scope

Spiral-ATCG is **research-grade cryptography**. Please read this carefully before using or evaluating it.

### What Spiral-ATCG is

- An open meta-material framework for symmetric cipher design, with a fully specified 320-bit reference cipher.
- A research artifact intended for analysis, education, evaluation, and as a `Crypto Agility` complement to standard ciphers.
- A demonstration of the cryptographic depth behind Digital Cryptogram's enterprise products.

### What Spiral-ATCG is NOT

- **Not certified.** No FIPS 140-3, no PCI DSS, no Common Criteria, no NIST endorsement.
- **Not a formally proven cipher.** No IND-CPA, IND-CCA, or QROM proof has been published. (This is the same status as AES, ChaCha20, and most standalone block ciphers — formal reductions for symmetric ciphers are rare in cryptography, but we list this explicitly to be honest about scope.)
- **Not a replacement for AES** in production cryptography for regulated environments.

### Production cryptography recommendation

For production cryptography in regulated environments (banking, government, healthcare, payment processing), use NIST-approved algorithms such as AES-GCM, ChaCha20-Poly1305, or NIST PQC standards (CRYSTALS-Kyber, CRYSTALS-Dilithium, etc.).

For commercial enterprise solutions integrating these standards, see Digital Cryptogram's product line:
[https://www.digitalcryptogram.com.tw/](https://www.digitalcryptogram.com.tw/)

---

## Reporting security vulnerabilities

We welcome responsible disclosure of any findings.

### Where to report

Send vulnerability reports to:

**Email**: sales_digi@digitalcryptogram.com.tw  
**Subject prefix**: `[SPIRAL-ATCG SECURITY]`

For sensitive disclosures, you may request our PGP key in your initial email.

### What to include

A useful report typically includes:

1. **Description**: a clear summary of the issue.
2. **Reproduction steps**: minimal code or test vectors that demonstrate the issue against the reference implementation.
3. **Expected vs observed behavior**: what the cipher should do vs what it actually does.
4. **Impact assessment**: as you understand it (we will independently assess).
5. **Suggested mitigation**: optional but appreciated.

### Disclosure timeline

Our preferred process:

| Step | Target timeline |
|---|---|
| Acknowledge receipt of report | Within 5 working days |
| Initial assessment + classification | Within 30 days |
| Patch / spec update / coordinated disclosure | Within 90 days for valid findings |
| Public disclosure with credit (with reporter's permission) | After patch is published |

For findings that affect Digital Cryptogram's commercial products in addition to the open Spiral-ATCG project, we may extend the timeline to coordinate vendor patching and customer notification.

### Scope of in-scope findings

Findings that are clearly in scope:

- Cryptanalytic attacks against the Spiral-ATCG cipher beyond what is acknowledged in `docs/SECURITY_AUDIT.md` and `docs/ADVANCED_AUDIT.md`.
- Roundtrip / pair-symmetry / decryption-correctness bugs in the reference implementation.
- Implementation bugs that cause incorrect ciphertext under specific inputs.
- Side-channel vulnerabilities in the reference implementation that fall outside the scope already noted in the audit reports.
- Specification ambiguities that allow incompatible implementations.

Out of scope:

- Issues with build tools, CI configuration, or Makefile (these are infrastructure, not security).
- Performance complaints (this is a reference implementation; SIMD optimization is future work).
- Findings that the audit reports already document as expected statistical noise.

### Recognition

Reporters who follow this disclosure process will be acknowledged in the project changelog and audit reports (with their permission and preferred name / handle / affiliation).

### No bounty program

This is an open research project. We do not currently offer monetary bounties, but we are happy to provide:

- Public attribution in audit reports.
- A formal letter of acknowledgment from Digital Cryptogram (useful for portfolio / academic CV).
- Feedback on findings to support reporter's own publication, if applicable.

---

## Out-of-band questions

For procurement / compliance / commercial questions about Digital Cryptogram's certified products (Zentra KMS, Zentra CSP, etc.), contact: sales_digi@digitalcryptogram.com.tw or visit [https://www.digitalcryptogram.com.tw/](https://www.digitalcryptogram.com.tw/).
