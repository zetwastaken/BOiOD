#!/usr/bin/env python3.11
"""
Wykresy dla benchmarku algorytmów max-flow (Ford–Fulkerson vs Edmonds–Karp).
Korzysta z plików benchmark_maxflow.csv oraz benchmark_maxflow_agg.csv
generowanych przez output/maxflow_benchmark.
"""
from pathlib import Path

import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns

BASE_DIR = Path(__file__).parent
RAW_CSV = BASE_DIR / "benchmark_maxflow.csv"
AGG_CSV = BASE_DIR / "benchmark_maxflow_agg.csv"
PLOT_DIR = BASE_DIR / "output" / "plots"

sns.set_theme(style="whitegrid", context="talk")
PALETTE = {
    "FordFulkerson": "#1b9e77",
    "EdmondsKarp": "#d95f02",
}


def load_data():
    if not RAW_CSV.exists() or not AGG_CSV.exists():
        raise SystemExit("Brak plików benchmark_maxflow.csv / benchmark_maxflow_agg.csv – uruchom najpierw benchmark.")
    raw = pd.read_csv(RAW_CSV)
    agg = pd.read_csv(AGG_CSV)
    return raw, agg


def plot_time_vs_size(agg: pd.DataFrame):
    data = agg.copy()
    data["density_str"] = data["density"].map(lambda x: f"g={x}")
    plt.figure(figsize=(11, 6))
    sns.lineplot(
        data=data,
        x="graph_size",
        y="mean_us",
        hue="algorithm",
        style="density_str",
        markers=True,
        dashes=False,
        palette=PALETTE,
    )
    plt.yscale("log")
    plt.xlabel("Rozmiar grafu (liczba wierzchołków)")
    plt.ylabel("Średni czas wykonania [µs] (skala log)")
    plt.title("Wydajność algorytmów max-flow w funkcji rozmiaru grafu")
    plt.tight_layout()
    out = PLOT_DIR / "maxflow_time_vs_size.png"
    plt.savefig(out, dpi=200)
    plt.close()
    return out


def plot_time_per_edge(raw: pd.DataFrame):
    df = raw[raw["edges"] > 0].copy()
    df["time_per_edge_us"] = df["execution_time_us"] / df["edges"]
    grouped = (
        df.groupby(["algorithm", "graph_size", "density"], as_index=False)
        .agg({"time_per_edge_us": "median", "edges": "median"})
        .sort_values("edges")
    )

    plt.figure(figsize=(11, 6))
    sns.lineplot(
        data=grouped,
        x="edges",
        y="time_per_edge_us",
        hue="algorithm",
        marker="o",
        palette=PALETTE,
    )
    plt.xscale("log")
    plt.yscale("log")
    plt.xlabel("Liczba krawędzi w grafie (skala log)")
    plt.ylabel("Mediana czasu na krawędź [µs] (skala log)")
    plt.title("Czas przetwarzania na krawędź vs liczba krawędzi")
    plt.tight_layout()
    out = PLOT_DIR / "maxflow_time_per_edge.png"
    plt.savefig(out, dpi=200)
    plt.close()
    return out


def plot_speedup(agg: pd.DataFrame):
    pivot = (
        agg.pivot_table(index=["graph_size", "density"], columns="algorithm", values="median_us")
        .reset_index()
        .dropna()
    )
    pivot["ff_vs_ek"] = pivot["EdmondsKarp"] / pivot["FordFulkerson"]
    pivot["density_str"] = pivot["density"].map(lambda x: f"g={x}")

    plt.figure(figsize=(11, 6))
    sns.barplot(
        data=pivot,
        x="graph_size",
        y="ff_vs_ek",
        hue="density_str",
        palette="light:b",
        edgecolor="black",
    )
    plt.axhline(1.0, color="black", linewidth=1, linestyle="--")
    plt.yscale("log")
    plt.xlabel("Rozmiar grafu (liczba wierzchołków)")
    plt.ylabel("Ile razy FF szybszy od EK (skala log)")
    plt.title("Przewaga czasowa Ford–Fulkerson vs Edmonds–Karp")
    plt.tight_layout()
    out = PLOT_DIR / "maxflow_speedup_ff_over_ek.png"
    plt.savefig(out, dpi=200)
    plt.close()
    return out


def plot_density_facets(agg: pd.DataFrame):
    sizes = sorted(agg["graph_size"].unique())
    pick = [s for s in sizes if s in (50, 200, 400, 800)]
    subset = agg[agg["graph_size"].isin(pick)].copy()
    subset["density_label"] = subset["density"].map(lambda x: f"{x:.2f}")

    g = sns.FacetGrid(
        subset,
        col="graph_size",
        hue="algorithm",
        col_wrap=2,
        sharey=False,
        height=4,
        palette=PALETTE,
    )
    g.map_dataframe(sns.lineplot, x="density", y="median_us", marker="o")
    g.set(yscale="log")
    g.set_axis_labels("Gęstość grafu", "Mediana czasu [µs] (log)")
    g.fig.subplots_adjust(top=0.88)
    g.fig.suptitle("Wpływ gęstości grafu na czas działania (median)")
    g.add_legend(title="Algorytm")

    out = PLOT_DIR / "maxflow_density_facets.png"
    g.savefig(out, dpi=200)
    plt.close()
    return out


def main():
    PLOT_DIR.mkdir(parents=True, exist_ok=True)
    raw, agg = load_data()
    outputs = [
        plot_time_vs_size(agg),
        plot_time_per_edge(raw),
        plot_speedup(agg),
        plot_density_facets(agg),
    ]
    print("Zapisano wykresy:")
    for p in outputs:
        print(" -", p)


if __name__ == "__main__":
    main()
