#!/usr/bin/env python3
"""Plot simulation results, averaging over seeds.

Usage:
    python3 scripts/plot.py results/sweep.csv
"""
import sys
import pandas as pd
import matplotlib.pyplot as plt


def panel(ax, df, column, ylabel, title):
    for sps, group in df.groupby("sps"):
        stats = (group.groupby("num_ues")[column]
                      .mean()
                      .reset_index()
                      .sort_values("num_ues"))

        label = "SPS + sensing" if sps else "random"
        ax.plot(stats["num_ues"], stats[column] * 100, marker="o", label=label)

    ax.set_xlabel("Number of UEs")
    ax.set_ylabel(ylabel)
    ax.set_title(title)
    ax.grid(alpha=0.3)
    ax.legend()


def main(path: str) -> None:
    df = pd.read_csv(path)

    seeds = df["seed"].nunique() if "seed" in df.columns else 1
    print(f"{len(df)} runs, {seeds} seed(s) per point")

    fig, axes = plt.subplots(1, 2, figsize=(11, 4))
    panel(axes[0], df, "collision_rate", "Collision rate (%)",
          "Collision rate vs UE density")
    panel(axes[1], df, "prr", "PRR (%)",
          "Packet reception ratio vs UE density")

    fig.tight_layout()
    out = path.rsplit(".", 1)[0] + ".png"
    fig.savefig(out, dpi=150)
    print(f"saved {out}")


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print(__doc__)
        sys.exit(1)
    main(sys.argv[1])
