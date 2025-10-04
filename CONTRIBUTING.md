# Contributing

Thanks for contributing! This project is **dual-licensed**: `GPL-2.0-only OR Proprietary`.

By submitting a contribution (code, docs, tests, etc.), you agree that:
1. **Inbound = Outbound**. You license your contribution under the same
   terms as the project: **GPL-2.0-only OR Proprietary**.
2. You have the necessary rights to grant this license for your contribution
   (e.g., you wrote it, or it’s from a compatible source).

We use the **Developer Certificate of Origin (DCO)**. Sign your commits with:

Signed-off-by: Your Name you@example.com

This attests (per https://developercertificate.org/) that you have the right
to submit the work under the project’s licenses.

## How to Contribute

- Fork, create a feature branch, and keep commits atomic.
- Add/adjust tests if relevant.
- Run the linters/build locally before opening a PR.
- In each source file you touch, keep the SPDX line:

```c
/* SPDX-License-Identifier: GPL-2.0-only OR Proprietary */
