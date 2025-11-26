#!/usr/bin/env python3
"""
Analiza wyników benchmarku CPM i PERT
Wczytuje pliki CSV wygenerowane przez BenchmarkExporter i tworzy wykresy oraz statystyki
"""

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from pathlib import Path
import sys

def analyze_cpm_benchmark(csv_file):
    """Analiza wyników benchmarku CPM"""
    print(f"\n{'='*60}")
    print(f"Analiza CPM: {csv_file}")
    print(f"{'='*60}\n")
    
    # Wczytaj dane
    df = pd.read_csv(csv_file, comment='#')
    
    # Statystyki podstawowe
    print("Statystyki podstawowe:")
    print(df[['Nodes', 'TopoTimeMs', 'BellmanTimeMs', 'Ratio']].describe())
    
    # Złożoność empiryczna
    print("\n\nAnaliza złożoności:")
    
    # Topological - O(V+E) - liniowa względem V
    if len(df) > 2:
        # Log-log regression dla topological
        log_nodes = np.log(df['Nodes'])
        log_topo_time = np.log(df['TopoTimeMs'])
        topo_coeffs = np.polyfit(log_nodes, log_topo_time, 1)
        print(f"Topological: złożoność ~ O(n^{topo_coeffs[0]:.2f})")
        
        # Log-log regression dla Bellman-Ford
        log_bellman_time = np.log(df['BellmanTimeMs'])
        bellman_coeffs = np.polyfit(log_nodes, log_bellman_time, 1)
        print(f"Bellman-Ford: złożoność ~ O(n^{bellman_coeffs[0]:.2f})")
    
    # Wykresy
    fig, axes = plt.subplots(2, 2, figsize=(15, 12))
    fig.suptitle('CPM Benchmark: Topological vs Bellman-Ford', fontsize=16, fontweight='bold')
    
    # 1. Czas wykonania - skala liniowa
    ax1 = axes[0, 0]
    ax1.plot(df['Nodes'], df['TopoTimeMs'], 'o-', label='Topological', color='blue', linewidth=2)
    ax1.plot(df['Nodes'], df['BellmanTimeMs'], 's-', label='Bellman-Ford', color='red', linewidth=2)
    ax1.set_xlabel('Liczba węzłów', fontsize=12)
    ax1.set_ylabel('Czas [ms]', fontsize=12)
    ax1.set_title('Czas wykonania (skala liniowa)', fontsize=14)
    ax1.legend(fontsize=10)
    ax1.grid(True, alpha=0.3)
    
    # 2. Czas wykonania - skala logarytmiczna
    ax2 = axes[0, 1]
    ax2.loglog(df['Nodes'], df['TopoTimeMs'], 'o-', label='Topological', color='blue', linewidth=2)
    ax2.loglog(df['Nodes'], df['BellmanTimeMs'], 's-', label='Bellman-Ford', color='red', linewidth=2)
    ax2.set_xlabel('Liczba węzłów', fontsize=12)
    ax2.set_ylabel('Czas [ms]', fontsize=12)
    ax2.set_title('Czas wykonania (skala log-log)', fontsize=14)
    ax2.legend(fontsize=10)
    ax2.grid(True, alpha=0.3)
    
    # 3. Ratio Bellman/Topo
    ax3 = axes[1, 0]
    ax3.plot(df['Nodes'], df['Ratio'], 'o-', color='green', linewidth=2, markersize=8)
    ax3.set_xlabel('Liczba węzłów', fontsize=12)
    ax3.set_ylabel('Ratio (Bellman/Topo)', fontsize=12)
    ax3.set_title('Stosunek czasu Bellman-Ford / Topological', fontsize=14)
    ax3.grid(True, alpha=0.3)
    ax3.axhline(y=1, color='gray', linestyle='--', alpha=0.5)
    
    # 4. Różnica czasów
    ax4 = axes[1, 1]
    time_diff = df['BellmanTimeMs'] - df['TopoTimeMs']
    ax4.plot(df['Nodes'], time_diff, 'o-', color='purple', linewidth=2, markersize=8)
    ax4.set_xlabel('Liczba węzłów', fontsize=12)
    ax4.set_ylabel('Różnica czasu [ms]', fontsize=12)
    ax4.set_title('Różnica: Bellman-Ford - Topological', fontsize=14)
    ax4.grid(True, alpha=0.3)
    ax4.axhline(y=0, color='gray', linestyle='--', alpha=0.5)
    
    plt.tight_layout()
    
    # Zapisz wykres
    output_file = csv_file.replace('.csv', '_plot.png')
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"\n✓ Wykres zapisany: {output_file}")
    
    return df

def analyze_pert_benchmark(csv_file):
    """Analiza wyników benchmarku PERT"""
    print(f"\n{'='*60}")
    print(f"Analiza PERT: {csv_file}")
    print(f"{'='*60}\n")
    
    # Wczytaj dane
    df = pd.read_csv(csv_file, comment='#')
    
    # Filtruj tylko rekordy z symulacją
    df_sim = df[df['HasSimulation'] == 1].copy()
    
    if df_sim.empty:
        print("Brak danych z symulacji!")
        return None
    
    # Statystyki podstawowe
    print("Statystyki podstawowe:")
    print(df_sim[['Nodes', 'AnalyticTimeUs', 'SimulationTimeUs', 'Simulations']].describe())
    
    # Porównanie dokładności
    print("\n\nPorównanie dokładności (analityka vs symulacja):")
    print(f"Średnia różnica wartości oczekiwanej: {df_sim['ExpectedDiff'].abs().mean():.4f}")
    print(f"Średnia różnica std dev: {df_sim['StdDevDiff'].abs().mean():.4f}")
    print(f"Średnia różnica prawdopodobieństwa: {df_sim['ProbabilityDiff'].abs().mean():.4f}")
    
    # Wykresy
    fig, axes = plt.subplots(2, 3, figsize=(18, 12))
    fig.suptitle('PERT Benchmark: Analityka vs Monte Carlo', fontsize=16, fontweight='bold')
    
    # 1. Czas wykonania (mikro -> milisekundy)
    ax1 = axes[0, 0]
    ax1.plot(df_sim['Nodes'], df_sim['AnalyticTimeUs']/1000, 'o-', label='Analityka', color='blue', linewidth=2)
    ax1.plot(df_sim['Nodes'], df_sim['SimulationTimeUs']/1000, 's-', label='Symulacja', color='red', linewidth=2)
    ax1.set_xlabel('Liczba węzłów', fontsize=12)
    ax1.set_ylabel('Czas [ms]', fontsize=12)
    ax1.set_title('Czas wykonania', fontsize=14)
    ax1.legend(fontsize=10)
    ax1.grid(True, alpha=0.3)
    
    # 2. Czas symulacji vs liczba iteracji
    ax2 = axes[0, 1]
    unique_sims = df_sim.groupby('Simulations').agg({
        'SimulationTimeUs': 'mean'
    }).reset_index()
    ax2.plot(unique_sims['Simulations'], unique_sims['SimulationTimeUs']/1000, 
             'o-', color='red', linewidth=2, markersize=8)
    ax2.set_xlabel('Liczba symulacji', fontsize=12)
    ax2.set_ylabel('Średni czas [ms]', fontsize=12)
    ax2.set_title('Czas symulacji vs liczba iteracji', fontsize=14)
    ax2.grid(True, alpha=0.3)
    
    # 3. Ratio czasów
    ax3 = axes[0, 2]
    time_ratio = (df_sim['SimulationTimeUs'] / df_sim['AnalyticTimeUs'])
    ax3.plot(df_sim['Nodes'], time_ratio, 'o-', color='green', linewidth=2)
    ax3.set_xlabel('Liczba węzłów', fontsize=12)
    ax3.set_ylabel('Ratio (Symulacja/Analityka)', fontsize=12)
    ax3.set_title('Stosunek czasu Symulacja/Analityka', fontsize=14)
    ax3.grid(True, alpha=0.3)
    
    # 4. Porównanie wartości oczekiwanej
    ax4 = axes[1, 0]
    ax4.plot(df_sim['Nodes'], df_sim['AnalyticExpected'], 'o-', label='Analityka', color='blue', linewidth=2)
    ax4.plot(df_sim['Nodes'], df_sim['SimExpected'], 's-', label='Symulacja', color='red', linewidth=2)
    ax4.set_xlabel('Liczba węzłów', fontsize=12)
    ax4.set_ylabel('Wartość oczekiwana', fontsize=12)
    ax4.set_title('Porównanie wartości oczekiwanej', fontsize=14)
    ax4.legend(fontsize=10)
    ax4.grid(True, alpha=0.3)
    
    # 5. Porównanie odchylenia standardowego
    ax5 = axes[1, 1]
    ax5.plot(df_sim['Nodes'], df_sim['AnalyticStdDev'], 'o-', label='Analityka', color='blue', linewidth=2)
    ax5.plot(df_sim['Nodes'], df_sim['SimStdDev'], 's-', label='Symulacja', color='red', linewidth=2)
    ax5.set_xlabel('Liczba węzłów', fontsize=12)
    ax5.set_ylabel('Odchylenie standardowe', fontsize=12)
    ax5.set_title('Porównanie odchylenia standardowego', fontsize=14)
    ax5.legend(fontsize=10)
    ax5.grid(True, alpha=0.3)
    
    # 6. Różnica prawdopodobieństw
    ax6 = axes[1, 2]
    prob_diff = (df_sim['SimProbability'] - df_sim['AnalyticProbability']).abs()
    ax6.plot(df_sim['Nodes'], prob_diff, 'o-', color='purple', linewidth=2, markersize=8)
    ax6.set_xlabel('Liczba węzłów', fontsize=12)
    ax6.set_ylabel('|P_sim - P_analytic|', fontsize=12)
    ax6.set_title('Różnica prawdopodobieństw', fontsize=14)
    ax6.grid(True, alpha=0.3)
    ax6.axhline(y=0, color='gray', linestyle='--', alpha=0.5)
    
    plt.tight_layout()
    
    # Zapisz wykres
    output_file = csv_file.replace('.csv', '_plot.png')
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"\n✓ Wykres zapisany: {output_file}")
    
    return df_sim

def main():
    if len(sys.argv) < 2:
        print("Usage: python analyze_benchmark.py <csv_file> [type]")
        print("  type: 'cpm' or 'pert' (auto-detected if not specified)")
        sys.exit(1)
    
    csv_file = sys.argv[1]
    
    if not Path(csv_file).exists():
        print(f"Error: File not found: {csv_file}")
        sys.exit(1)
    
    # Auto-detect type from filename or content
    benchmark_type = None
    if len(sys.argv) > 2:
        benchmark_type = sys.argv[2].lower()
    else:
        # Try to detect from filename
        if 'cpm' in csv_file.lower():
            benchmark_type = 'cpm'
        elif 'pert' in csv_file.lower():
            benchmark_type = 'pert'
        else:
            # Try to detect from content
            with open(csv_file, 'r') as f:
                first_line = f.readline()
                if 'Bellman' in first_line or 'Topo' in first_line:
                    benchmark_type = 'cpm'
                elif 'Analytic' in first_line or 'Simulation' in first_line:
                    benchmark_type = 'pert'
    
    if benchmark_type == 'cpm':
        analyze_cpm_benchmark(csv_file)
    elif benchmark_type == 'pert':
        analyze_pert_benchmark(csv_file)
    else:
        print("Error: Could not determine benchmark type. Please specify 'cpm' or 'pert'")
        sys.exit(1)
    
    plt.show()

if __name__ == '__main__':
    main()
