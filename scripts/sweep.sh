#!/usr/bin/env bash
# Sweep UE density with sensing on and off, then plot.
set -euo pipefail

BIN=build/v2xsim
OUT=results/sweep.csv

rm -f "$OUT"

for n in 10 20 30 40 50 60 80 100; do
  "$BIN" --ues "$n" --out "$OUT"
  "$BIN" --ues "$n" --sensing --out "$OUT"
done

python3 scripts/plot.py "$OUT"
