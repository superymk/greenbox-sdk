# // SPDX-License-Identifier: GPL-2.0-only OR Proprietary
# // Author: Miao Yu (Carnegie Mellon University)
# // Author: Virgil Gligor (Carnegie Mellon University)

# Portable Makefile for Ubuntu 24.04 and MSYS2/UCRT64

SHELL := /usr/bin/env bash

SCRIPT := update_certs.sh
BASE_CA := ca-certificates.crt
OUT_CA  := ca-certificates.with-extras.crt

.PHONY: all clean

all: $(OUT_CA)

$(OUT_CA): $(SCRIPT) $(BASE_CA)
	@chmod +x "$(SCRIPT)"
	@echo "==> Updating CA bundle '$(OUT_CA)' from base '$(BASE_CA)' ..."
	@./$(SCRIPT) "$(BASE_CA)" "$(OUT_CA)"

clean:
	@echo "==> Cleaning generated files..."
	@rm -f chain-*.pem leaf-*.pem intermediates-*.pem certs-*.pem certs.pem "$(OUT_CA)"
	@echo "Done."
