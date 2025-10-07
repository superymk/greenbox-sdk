@echo off

REM // SPDX-License-Identifier: GPL-2.0-only OR Proprietary
REM // Author: Miao Yu (Carnegie Mellon University)
REM // Author: Virgil Gligor (Carnegie Mellon University)

REM Convert the format of the AIK file in <userdata absolute path of tpmtools> from PEM to DER. 
REM Usage: win_tpm2_aik_convert.bat <userdata absolute path of tpmtools>

cd openssl
openssl rsa -pubin -in %1\aik.pem -outform DER -out %1\aik.der
del /Q %1\aik.pem