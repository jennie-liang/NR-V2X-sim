#!/usr/bin/env bash
# Sweep UE density with sensing on and off, repeating each point over several
# seeds so the plot can show a mean and a spread rather than a single sample.
set -euo pipefail

BIN=./v2xsim
OUT=results/sweep.csv

UES=(10 20 30 40 50 60 80 100)
SEEDS=(1 2 3 4 5)

rm -f "$OUT"
mkdir -p results

total=$(( ${#UES[@]} * ${#SEEDS[@]} * 2 ))
i=0

for n in "${UES[@]}"; do
  for s in "${SEEDS[@]}"; do
    for mode in "" "--sensing"; do
      i=$((i + 1))
      printf '\r[%d/%d] ues=%s seed=%s %s   ' "$i" "$total" "$n" "$s" "${mode:-random}"
      # shellcheck disable=SC2086
      "$BIN" --ues "$n" --seed "$s" $mode --out "$OUT" > /dev/null
    done
  done
done
printf '\n'

python3 scripts/plot.py "$OUT"
