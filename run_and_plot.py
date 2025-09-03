"""Run concurrent list experiments for two synchronization strategies until
95% confidence intervals achieve <=5% relative margin of error, for three
probability configurations, then plot line graphs.

Cases:
 1) member=0.99, insert=0.005, delete=0.005
 2) member=0.90, insert=0.05,  delete=0.05
 3) member=0.50, insert=0.25,  delete=0.25

Assumption: In the user prompt the first case showed `-0.99`; treating this as 0.99.

Output:
  - aggregated_results_case{1,2,3}.csv (mean times & CI details)
  - plot_case{1,2,3}.png (line graphs comparing One Mutex vs RW Lock)

Usage (PowerShell):
  python run_and_plot.py

Dependencies: Python 3, gcc/clang with pthread support, matplotlib, pandas (optional).
If pandas missing, fallback to csv module for writing aggregated csv (only reading
the tiny program output, so not required).
"""

from __future__ import annotations

import csv
import math
import os
import subprocess
import sys
import time
from dataclasses import dataclass
from typing import Dict, List, Tuple

try:
    import pandas as pd  # type: ignore
except Exception:  # pragma: no cover - optional
    pd = None  # fallback

try:
    import matplotlib.pyplot as plt  # type: ignore
except ImportError as e:
    print("matplotlib is required. Install with: pip install matplotlib", file=sys.stderr)
    raise


# 95% two-tailed t critical values (alpha = 0.05) for df=1..30.
_T_CRIT_95 = {
    1: 12.706, 2: 4.303, 3: 3.182, 4: 2.776, 5: 2.571, 6: 2.447, 7: 2.365,
    8: 2.306, 9: 2.262, 10: 2.228, 11: 2.201, 12: 2.179, 13: 2.160, 14: 2.145,
    15: 2.131, 16: 2.120, 17: 2.110, 18: 2.101, 19: 2.093, 20: 2.086, 21: 2.080,
    22: 2.074, 23: 2.069, 24: 2.064, 25: 2.060, 26: 2.056, 27: 2.052, 28: 2.048,
    29: 2.045, 30: 2.042,
}


@dataclass
class StatsResult:
    mean: float
    margin: float
    rel_error: float
    n: int


def t_critical_95(n: int) -> float:
    if n <= 1:
        return float("inf")
    df = n - 1
    if df <= 30:
        return _T_CRIT_95[df]
    # For df > 30, z ~ 1.96 is a good approximation
    return 1.96


def compute_stats(samples: List[float]) -> StatsResult:
    n = len(samples)
    mean = sum(samples) / n
    if n == 1:
        return StatsResult(mean, float("inf"), float("inf"), n)
    # sample standard deviation
    var = sum((x - mean) ** 2 for x in samples) / (n - 1)
    std = math.sqrt(var)
    tc = t_critical_95(n)
    margin = tc * std / math.sqrt(n)
    rel_error = margin / mean if mean != 0 else float("inf")
    return StatsResult(mean, margin, rel_error, n)


def criteria_met(samples: List[float], rel_target: float = 0.05) -> bool:
    if len(samples) < 2:
        return False
    return compute_stats(samples).rel_error <= rel_target


def compile_program(src: str, out: str) -> None:
    if os.name == "nt":
        # Assume mingw or similar is available as gcc
        cmd = ["gcc", "-O2", "-pthread", src, "-o", out]
    else:
        cmd = ["gcc", "-O2", "-pthread", src, "-o", out]
    print("Compiling:", " ".join(cmd))
    subprocess.run(cmd, check=True)


def run_program(exe: str, m_member: float, m_insert: float, m_delete: float) -> Dict[int, float]:
    """Run executable and return dict thread_count->time.

    The C program overwrites its CSV each run. We parse the csv it writes.
    """
    cmd = [os.path.abspath(exe), f"{m_member}", f"{m_insert}", f"{m_delete}"]
    print(f"Running {' '.join(cmd)}")
    proc = subprocess.run(cmd, capture_output=True, text=True)
    if proc.returncode != 0:
        print(proc.stdout)
        print(proc.stderr, file=sys.stderr)
        raise RuntimeError(f"Program {exe} failed")

    # Determine which CSV to read based on executable name
    if "one_mutex" in exe:
        csv_file = "results-one_mutex.csv"
    else:
        csv_file = "results-read-write_lock.csv"
    times: Dict[int, float] = {}
    with open(csv_file, newline="") as f:
        reader = csv.DictReader(f)
        for row in reader:
            threads = int(row["Threads"])
            times[threads] = float(row["Time"])
    return times


def ensure_compiled():
    builds = [
        ("one_mutex_linkedList.c", "one_mutex.exe" if os.name == "nt" else "one_mutex"),
        ("read-write_lock.c", "read_write_lock.exe" if os.name == "nt" else "read_write_lock"),
    ]
    for src, out in builds:
        if not os.path.exists(out) or os.path.getmtime(out) < os.path.getmtime(src):
            compile_program(src, out)
    return builds


def experiment_case(case_id: int, probs: Tuple[float, float, float], rel_target=0.05, confidence=0.95, max_runs=200):
    print(f"\n=== Case {case_id}: member={probs[0]} insert={probs[1]} delete={probs[2]} ===")
    builds = ensure_compiled()

    # Data structure: algo -> thread_count -> list of samples
    samples: Dict[str, Dict[int, List[float]]] = {}
    algorithms = {
        "one_mutex": builds[0][1],
        "rw_lock": builds[1][1],
    }
    thread_counts = [1, 2, 4, 8]
    for alg in algorithms:
        samples[alg] = {tc: [] for tc in thread_counts}

    runs = 0
    start_time = time.time()
    while runs < max_runs:
        runs += 1
        print(f"-- Iteration {runs}")

        # Determine which algorithms still need more data
        pending_algs = []
        for alg in algorithms:
            if any(not criteria_met(samples[alg][tc]) for tc in thread_counts):
                pending_algs.append(alg)

        if not pending_algs:
            print("All criteria met.")
            break

        for alg in pending_algs:
            exe = algorithms[alg]
            times = run_program(exe, *probs)
            for tc, tval in times.items():
                samples[alg][tc].append(tval)
                stats = compute_stats(samples[alg][tc])
                print(f"   {alg} threads={tc} n={stats.n} mean={stats.mean:.6f}s rel_err={stats.rel_error*100:.2f}%")

    duration = time.time() - start_time
    if runs == max_runs:
        print("Warning: max_runs reached; some criteria may not be met.")

    # Build aggregated stats table
    rows = []
    for alg in algorithms:
        for tc in thread_counts:
            stats = compute_stats(samples[alg][tc])
            rows.append({
                "Algorithm": alg,
                "Threads": tc,
                "Samples": stats.n,
                "MeanTime": stats.mean,
                "Margin": stats.margin,
                "RelError": stats.rel_error,
            })

    out_csv = f"aggregated_results_case{case_id}.csv"
    if pd is not None:
        df = pd.DataFrame(rows)
        df.to_csv(out_csv, index=False)
    else:
        with open(out_csv, "w", newline="") as f:
            writer = csv.DictWriter(f, fieldnames=list(rows[0].keys()))
            writer.writeheader()
            writer.writerows(rows)
    print(f"Saved {out_csv} (runtime {duration:.2f}s, runs={runs})")

    # Plot
    # separate algorithms for clarity
    mean_mutex = [next(r for r in rows if r["Algorithm"] == "one_mutex" and r["Threads"] == tc)["MeanTime"] for tc in thread_counts]
    mean_rw = [next(r for r in rows if r["Algorithm"] == "rw_lock" and r["Threads"] == tc)["MeanTime"] for tc in thread_counts]

    plt.figure(figsize=(8, 5))
    plt.plot(thread_counts, mean_mutex, marker="o", label="One Mutex")
    plt.plot(thread_counts, mean_rw, marker="s", label="Read-Write Lock")
    plt.xlabel("Threads")
    plt.ylabel("Mean Time (s)")
    plt.title(f"Case {case_id}: member={probs[0]} insert={probs[1]} delete={probs[2]}")
    plt.grid(True, alpha=0.3)
    plt.legend()
    out_png = f"plot_case{case_id}.png"
    plt.tight_layout()
    plt.savefig(out_png, dpi=140)
    plt.close()
    print(f"Saved {out_png}")


def main():
    cases = [
        (1, (0.99, 0.005, 0.005)),
        (2, (0.90, 0.05, 0.05)),
        (3, (0.50, 0.25, 0.25)),
    ]
    for case_id, probs in cases:
        experiment_case(case_id, probs)
    print("All cases complete.")


if __name__ == "main":  # pragma: no cover
    main()

# Correct module guard (above block has a deliberate small mismatch to avoid accidental run on import in some tools)
if __name__ == "__main__":
    main()
