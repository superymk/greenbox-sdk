#!/usr/bin/env bash

# // SPDX-License-Identifier: GPL-2.0-only OR Proprietary
# // Author: Miao Yu (Carnegie Mellon University)
# // Author: Virgil Gligor (Carnegie Mellon University)

set -euo pipefail

# update_certs.sh
# Usage:
#   ./update_certs.sh [base_ca] [out_ca] [host1:port host2:port ...]
#
# Examples:
#   ./update_certs.sh ubuntu-ca-certificates.crt ubuntu-ca-certificates.with-extras.crt
#   ./update_certs.sh ubuntu-ca-certificates.crt ubuntu-ca-certificates.with-extras.crt oauth2.googleapis.com:443 api.dropboxapi.com:443
#
# Behavior:
#  - Copies base_ca to out_ca
#  - For each host:port, fetches the full chain with openssl s_client
#  - Writes certs.pem (concatenated chain as sent by server) for the LAST processed host
#  - Appends unique CA certificates (intermediates only, not self-signed roots) to out_ca
#  - Deletes intermediate PEM files it generated (chain-*.pem, intermediates-*.pem, leaf-*.pem)
#  - Verifies the leaf certificate with the updated out_ca

BASE_CA="${1:-ubuntu-ca-certificates.crt}"
OUT_CA="${2:-ubuntu-ca-certificates.with-extras.crt}"
shift 2 || true

# Default hosts if none passed:
if [ "$#" -eq 0 ]; then
  set -- \
    "oauth2.googleapis.com:443" \
    "www.googleapis.com:443" \
    "content.googleapis.com:443" \
    "api.dropboxapi.com:443" \
    "content.dropboxapi.com:443"
fi

# Basic prerequisites
for cmd in openssl awk sed grep; do
  command -v "$cmd" >/dev/null 2>&1 || { echo "Missing dependency: $cmd"; exit 127; }
done

if [ ! -s "$BASE_CA" ]; then
  echo "Base CA bundle '$BASE_CA' not found or empty."; exit 2
fi

# Start from a fresh copy of base CA
cp -f "$BASE_CA" "$OUT_CA"

# Helper: safe fingerprint test
has_fingerprint_in_file() {
  local fingerprint="$1" file="$2"
  # Match either "SHA256 Fingerprint=" format or raw hex with colons
  grep -qiE "$fingerprint" "$file" 2>/dev/null
}

# Process one host:port
process_host() {
  local hostport="$1"
  local host="${hostport%:*}"
  local port="${hostport##*:}"

  echo
  echo "==> Processing $host:$port ..."

  # Clean any prior temp files for this host
  rm -f chain-*.pem intermediates-*.pem leaf-*.pem certs-*.pem 2>/dev/null || true

  # 1) Fetch the chain (leaf + intermediates; typically no root)
  #    Produces chain-00.pem, chain-01.pem, ...
  openssl s_client -connect "${host}:${port}" -servername "${host}" -showcerts </dev/null 2>/dev/null \
  | awk '
    /BEGIN CERTIFICATE/ { fn = sprintf("chain-%02d.pem", n++); w=1 }
    w { print > fn }
    /END CERTIFICATE/   { close(fn); w=0 }
  '

  if ! ls chain-*.pem >/dev/null 2>&1; then
    echo "No certificates captured from ${host}:${port}; skipping."
    return 0
  fi

  # 2) Save the leaf separately (assume first is leaf)
  cp chain-00.pem "leaf-${host}.pem" 2>/dev/null || true

  # 3) Concatenate entire chain to certs-${host}.pem (for inspection/verification)
  cat chain-*.pem > "certs-${host}.pem"
  # Also copy to certs.pem (the "latest" chain)
  cp "certs-${host}.pem" certs.pem

  # 4) Build intermediates list: CA:TRUE and not self-signed (subject != issuer)
  : > "intermediates-${host}.pem"
  for f in chain-*.pem; do
    [ -e "$f" ] || continue
    if openssl x509 -in "$f" -noout -text | grep -q "CA:TRUE"; then
      subj=$(openssl x509 -in "$f" -noout -subject)
      issu=$(openssl x509 -in "$f" -noout -issuer)
      if [ "$subj" != "$issu" ]; then
        cat "$f" >> "intermediates-${host}.pem"
      fi
    fi
  done

  # 5) Append any *new* CA certs (by SHA256 fingerprint) to OUT_CA
  if [ -s "intermediates-${host}.pem" ]; then
    # Split into individual certs and de-duplicate before append
    awk '
      /BEGIN CERTIFICATE/ { fn = sprintf("tmp-int-%02d.pem", n++); w=1 }
      w { print > fn }
      /END CERTIFICATE/   { close(fn); w=0 }
    ' "intermediates-${host}.pem"

    for part in tmp-int-*.pem; do
      [ -e "$part" ] || continue
      fp=$(openssl x509 -in "$part" -noout -fingerprint -sha256 | sed 's/^.*=//; s/://g; s/[[:space:]]//g')
      if ! has_fingerprint_in_file "$fp" "$OUT_CA"; then
        echo "  [+] Adding new CA (SHA256=$fp) from ${host}:${port}"
        cat "$part" >> "$OUT_CA"
      else
        echo "  [=] CA already present (SHA256=$fp); skipping"
      fi
      rm -f "$part"
    done
  else
    echo "  [!] No intermediates detected for ${host}:${port}."
  fi

  # 6) Verify using the updated OUT_CA (roots) + untrusted chain (as sent)
  if [ -s "$OUT_CA" ] && [ -s "leaf-${host}.pem" ]; then
    echo "  Verifying leaf against updated CA bundle..."
    if openssl verify -CAfile "$OUT_CA" -untrusted "certs-${host}.pem" "leaf-${host}.pem" >/dev/null 2>&1; then
      echo "  Verification: OK (${host})"
    else
      echo "  Verification: FAILED (${host})"
      # Don¡¯t exit; continue other hosts
    fi
  fi

  # 7) Delete *intermediate* pem files after run (keep certs-${host}.pem & certs.pem for inspection)
  rm -f chain-*.pem "intermediates-${host}.pem" "leaf-${host}.pem" 2>/dev/null || true
}

# Loop through requested hosts
for hp in "$@"; do
  process_host "$hp"
done

echo
echo "==> Updated bundle: $OUT_CA"
echo "    Last fetched chain: ./certs.pem"
echo "    Per-host chains:    ./certs-<host>.pem"
echo "Note: 'make clean' removes certs*.pem and '$OUT_CA'."
