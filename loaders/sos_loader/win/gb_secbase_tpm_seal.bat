@echo off

REM // SPDX-License-Identifier: GPL-2.0-only OR Proprietary
REM // Author: Miao Yu (Carnegie Mellon University)
REM // Author: Virgil Gligor (Carnegie Mellon University)

REM Invoke SecBase to seal its secrets and save the output file alongside UXMHF64 files (Scan and use the first UXMHF64
REM location only).
cd ../tools/tpmtools
gb_tpm.exe --secbase-seal