@echo off

REM /*
REM  * -------------------------------------------------------------------------------------
REM  * Carnegie Mellon University
REM  * Copyright (C) 2023 - 2023 Carnegie Mellon University. All Rights Reserved.
REM  * 
REM  * This software is the proprietary information of Carnegie Mellon University.
REM  * Use is subject to license terms.
REM  * 
REM  * Any reproduction, modification, distribution, or disclosure of this software, or any part of it, without the express
REM  * written consent of Carnegie Mellon University. is strictly prohibited.
REM  * -------------------------------------------------------------------------------------
REM  * 
REM  * Authors: Miao Yu
REM  *          Virgil Gligor
REM  */

REM Convert the format of the AIK file in <userdata absolute path of tpmtools> from PEM to DER. 
REM Usage: win_tpm2_aik_convert.bat <userdata absolute path of tpmtools>

cd openssl
openssl rsa -pubin -in %1\aik.pem -outform DER -out %1\aik.der
del /Q %1\aik.pem