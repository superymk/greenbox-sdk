@echo off
REM Store TPM PCRs to files.
cd ../tools/tpmtools
gb_tpm.exe -pf %1 %2