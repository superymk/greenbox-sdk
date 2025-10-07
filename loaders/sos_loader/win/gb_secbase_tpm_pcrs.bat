@echo off

REM // SPDX-License-Identifier: GPL-2.0-only OR Proprietary
REM // Author: Miao Yu (Carnegie Mellon University)
REM // Author: Virgil Gligor (Carnegie Mellon University)

REM Store TPM PCRs to files.
cd ../tools/tpmtools
gb_tpm.exe -pf %1 %2