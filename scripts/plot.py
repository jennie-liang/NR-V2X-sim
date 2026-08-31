#!/usr/bin/env python3
"""Plot simulation results.

Usage:
    python3 scripts/plot.py results/run.csv
"""
import sys
import pandas as pd
import matplotlib.pyplot as plt


def main(path: str) -> None:
    df = pd.read_csv(path)

    fig, axes = plt.subplots(1, 2, figsize=(11, 4))

    for sensing, group in df.groupby("sensing"):
        label = "sensing on" if sensing else "sensing off"
        g = group.sort_values("num_ues")
        axes[0].plot(g["num_ues"], g["collision_rate"] * 100, marker="o", label=label)
        axes[1].plot(g["num_ues"], g["prr"] * 100, marker="o", label=label)

    axes[0].set_xlabel("Number of UEs")
    axes[0].set_ylabel("Collision rate (%)")
    axes[0].set_title("Collision rate vs UE density")
    axes[0].grid(alpha=0.3)
    axes[0].legend()

    axes[1].set_xlabel("Number of UEs")
    axes[1].set_ylabel("PRR (%)")
    axes[1].set_title("Packet reception ratio vs UE density")
    axes[1].grid(alpha=0.3)
    axes[1].legend()

    fig.tight_layout()
    out = path.rsplit(".", 1)[0] + ".png"
    fig.savefig(out, dpi=150)
    print(f"saved {out}")


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print(__doc__)
        sys.exit(1)
    main(sys.argv[1])
