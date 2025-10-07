@echo off

REM // SPDX-License-Identifier: GPL-2.0-only OR Proprietary
REM // Author: Miao Yu (Carnegie Mellon University)
REM // Author: Virgil Gligor (Carnegie Mellon University)

REM ============================================================
REM Batch File: generateQuote.bat
REM Description: Generates TPM quote of the PCRs from the physical TPM device.
REM Usage: generateQuote.bat <PCR1> <PCR2> <nonce-base64>
REM Example: generateQuote.bat "PCR0_Value" "PCR1_Value" "YWJjZGVmZw=="
REM ============================================================

REM ------------------------------------------------------------
REM Input Validation: Check for correct number of arguments
REM ------------------------------------------------------------
IF "%~3"=="" (
    ECHO.
    ECHO ================== Usage ==================
    ECHO Usage: %~nx0 ^<PCR1^> ^<PCR2^> ^<nonce-base64^>
    ECHO Example: %~nx0 "PCR0_Value" "PCR1_Value" "YWJjZGVmZw=="
    ECHO ================================================
    ECHO.
    EXIT /B 1
)

REM ------------------------------------------------------------
REM Assigning Parameters to Variables for Clarity
REM ------------------------------------------------------------
SET "PCR1=%~1"
SET "PCR2=%~2"
SET "NONCE=%~3"

REM ------------------------------------------------------------
REM Debugging: Echo the parameters (optional)
REM ------------------------------------------------------------
@REM ECHO Parameter 1 (PCR1): "%PCR1%"
@REM ECHO Parameter 2 (PCR2): "%PCR2%"
@REM ECHO Parameter 3 (Nonce Base64): "%NONCE%"

REM ------------------------------------------------------------
REM Change Directory to TPM Tools
REM ------------------------------------------------------------
CD /D "%~dp0..\tools\tpmtools" 2>nul
IF ERRORLEVEL 1 (
    ECHO Error: Failed to change directory to "..\tools\tpmtools".
    ECHO Please ensure the TPM tools directory exists.
    EXIT /B 1
)

REM ------------------------------------------------------------
REM Execute the TPM Quote Command with Quoted Parameters
REM ------------------------------------------------------------
REM Running gb_tpm.exe with the provided parameters

gb_tpm.exe -qnb64 "%PCR1%" "%PCR2%" "%NONCE%"
IF ERRORLEVEL 1 (
    ECHO Error: TPM quote command failed.
    EXIT /B 1
)

REM ------------------------------------------------------------
REM Success Message (optional)
REM ------------------------------------------------------------
ECHO TPM quote generated successfully.
EXIT /B 0
