#!/usr/bin/env bash
# Sweep UE density across the three selection strategies, repeating each point
# over several seeds so the plot shows a mean rather than a single sample.
set -euo pipefail

BIN=./v2xsim
OUT=results/sweep.csv

UES=(10 20 30 40 50 60 80 100)
SEEDS=(1 2 3 4 5)
MODES=("" "--sps" "--sps --sensing")

rm -f "$OUT"
mkdir -p results

total=$(( ${#UES[@]} * ${#SEEDS[@]} * ${#MODES[@]} ))
i=0

for n in "${UES[@]}"; do
  for s in "${SEEDS[@]}"; do
    for mode in "${MODES[@]}"; do
      i=$((i + 1))
      printf '\r[%d/%d] ues=%-3s seed=%s %-16s' "$i" "$total" "$n" "$s" "${mode:-random}"
      # shellcheck disable=SC2086
      "$BIN" --ues "$n" --seed "$s" $mode --out "$OUT" > /dev/null
    done
  done
done
printf '\n'

python3 scripts/plot.py "$OUT"
