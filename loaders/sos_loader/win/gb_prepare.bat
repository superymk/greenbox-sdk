@echo off
REM cd ../tools/tpmtools

REM Reload and restart SecAgent
sc stop SecAgent
sc delete SecAgent

sc create SecAgent binPath=%2 type=kernel
sc start SecAgent