@echo off

REM // SPDX-License-Identifier: GPL-2.0-only OR Proprietary
REM // Author: Miao Yu (Carnegie Mellon University)
REM // Author: Virgil Gligor (Carnegie Mellon University)

REM cd ../tools/tpmtools

REM Reload and restart SecAgent
sc stop SecAgent
sc delete SecAgent

sc create SecAgent binPath=%2 type=kernel
sc start SecAgent