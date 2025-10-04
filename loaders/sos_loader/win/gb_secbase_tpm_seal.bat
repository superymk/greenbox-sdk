@echo off
REM Invoke SecBase to seal its secrets and save the output file alongside UXMHF64 files (Scan and use the first UXMHF64
REM location only).
cd ../tools/tpmtools
gb_tpm.exe --secbase-seal