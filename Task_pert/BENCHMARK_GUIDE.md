# Benchmark Results - Instrukcja użycia

## 📊 Dostępne narzędzia do analizy wyników

### 1. **Logowanie do konsoli** (już działa)
Program automatycznie wyświetla wyniki w konsoli z:
- ✨ Kolorowaniem (ścieżka krytyczna, nagłówki)
- 📋 Tabelami z danymi
- 📊 Histogramem dla symulacji PERT
- ⏱️ Czasami wykonania na końcu

### 2. **Eksport do CSV** (nowe!)
Zapisywanie wyników benchmarku do plików CSV dla dalszej analizy.

### 3. **Analiza w Pythonie** (nowe!)
Automatyczna analiza CSV + generowanie wykresów.

---

## 🚀 Jak uruchomić benchmark

### Podstawowy benchmark CPM i PERT:
```bash
cd Task_pert/build
./task_pert --benchmark --benchmark-pert-sim 10000
```

### Benchmark z większymi rozmiarami:
```bash
./task_pert --benchmark --benchmark-sizes 10,50,100,200,500,1000
```

### Benchmark tylko CPM:
```bash
./task_pert --benchmark --benchmark-sizes 10,50,100,200,500,1000,2000
```

### Benchmark PERT z różnymi liczbami symulacji:
```bash
# Mało symulacji (szybkie)
./task_pert --benchmark --benchmark-pert-sim 100

# Średnio symulacji
./task_pert --benchmark --benchmark-pert-sim 1000

# Dużo symulacji (dokładne, ale wolne)
./task_pert --benchmark --benchmark-pert-sim 100000
```

---

## 💾 Zapisywanie wyników (TODO - integracja)

### Po dodaniu do main.cpp:

```bash
# Zapisz wyniki CPM do CSV
./task_pert --benchmark --export-cpm results_cpm.csv

# Zapisz wyniki PERT do CSV
./task_pert --benchmark --benchmark-pert-sim 10000 --export-pert results_pert.csv

# Zapisz wszystkie wyniki
./task_pert --benchmark --benchmark-pert-sim 10000 --export-all results.csv
```

---

## 📈 Analiza wyników w Pythonie

### Instalacja wymagań:
```bash
pip install pandas matplotlib numpy
```

### Analiza CPM:
```bash
python analyze_benchmark.py results_cpm.csv
```

### Analiza PERT:
```bash
python analyze_benchmark.py results_pert.csv
```

### Co zostanie wygenerowane:
- 📊 **Wykresy PNG** z analizą wydajności
- 📋 **Statystyki** w konsoli
- 📉 **Analiza złożoności** (empiryczna O-notation)

---

## 📊 Przykładowe wykresy (CPM)

### 1. Czas wykonania (liniowa)
Porównanie czasu Topological vs Bellman-Ford na skali liniowej

### 2. Czas wykonania (log-log)
Analiza złożoności algorytmicznej - linia prosta na log-log = potęga

### 3. Ratio Bellman/Topo
Jak rośnie stosunek czasu - im większy problem, tym większa różnica

### 4. Różnica czasów
Absolutna różnica w czasie - pokazuje praktyczne znaczenie

---

## 📊 Przykładowe wykresy (PERT)

### 1. Czas wykonania
Porównanie czasu analitycznego vs symulacyjnego

### 2. Czas vs liczba symulacji
Zależność liniowa? Czy warto więcej iteracji?

### 3. Ratio czasów
Ile razy wolniejsza jest symulacja?

### 4-6. Porównanie dokładności
- Wartość oczekiwana (expected)
- Odchylenie standardowe (std dev)
- Różnica prawdopodobieństw

---

## 📝 Format plików CSV

### CPM CSV:
```csv
Nodes,Edges,TopoTimeMs,TopoTimeUs,BellmanTimeMs,BellmanTimeUs,Ratio,TotalDuration,CriticalPathLength
10,15,0.123,123,0.456,456,3.707,25,5
50,120,1.234,1234,5.678,5678,4.601,180,12
...
```

### PERT CSV:
```csv
Nodes,Edges,TargetTime,TargetProbability,AnalyticTimeUs,AnalyticExpected,AnalyticStdDev,...
10,15,25.5,0.90,123,24.8,2.3,...
...
```

---

## 🎯 Eksperymenty do przeprowadzenia

### Eksperyment 1: Skalowalność CPM
```bash
# Test małych problemów
./task_pert --benchmark --benchmark-sizes 10,20,30,40,50 --export-cpm exp1_small.csv

# Test średnich problemów
./task_pert --benchmark --benchmark-sizes 100,200,300,400,500 --export-cpm exp1_medium.csv

# Test dużych problemów
./task_pert --benchmark --benchmark-sizes 1000,1500,2000,2500,3000 --export-cpm exp1_large.csv

# Analiza
python analyze_benchmark.py exp1_small.csv cpm
python analyze_benchmark.py exp1_medium.csv cpm
python analyze_benchmark.py exp1_large.csv cpm
```

### Eksperyment 2: Dokładność PERT vs liczba symulacji
```bash
# Test z różną liczbą symulacji
./task_pert --benchmark --benchmark-pert-sim 100 --export-pert exp2_sim100.csv
./task_pert --benchmark --benchmark-pert-sim 1000 --export-pert exp2_sim1000.csv
./task_pert --benchmark --benchmark-pert-sim 10000 --export-pert exp2_sim10000.csv
./task_pert --benchmark --benchmark-pert-sim 100000 --export-pert exp2_sim100000.csv

# Analiza
python analyze_benchmark.py exp2_sim100.csv pert
python analyze_benchmark.py exp2_sim1000.csv pert
python analyze_benchmark.py exp2_sim10000.csv pert
python analyze_benchmark.py exp2_sim100000.csv pert
```

### Eksperyment 3: Stabilność wyników
```bash
# Uruchom ten sam test 10 razy
for i in {1..10}; do
    ./task_pert --benchmark --benchmark-seed $i --export-cpm stability_$i.csv
done

# Połącz i analizuj wariancję wyników
```

---

## 📚 Interpretacja wyników

### CPM - Co oznacza Ratio?
- **Ratio < 2**: Bellman-Ford niewiele wolniejszy
- **Ratio 2-5**: Zauważalna różnica
- **Ratio > 10**: Znacząca różnica - Topological wyraźnie lepszy
- **Ratio rosnący z n**: Potwierdza różnicę w złożoności O(V+E) vs O(V·E)

### PERT - Kiedy symulacja ma sens?
- **Dla małych problemów**: Analityka szybsza i wystarczająco dokładna
- **Dla asymetrycznych rozkładów**: Symulacja bardziej dokładna
- **Trade-off**: Dokładność vs czas
  - 1000 symulacji: szybkie, przybliżone
  - 10000 symulacji: dobre dla większości zastosowań
  - 100000+ symulacji: bardzo dokładne, ale wolne

### Różnice w wynikach:
- **ExpectedDiff < 5%**: Analityka wystarczająco dobra
- **ProbabilityDiff < 0.05**: Akceptowalna różnica
- **Większe różnice**: Rozkład prawdopodobnie nie jest normalny

---

## 🔧 TODO - Integracja z main.cpp

Aby włączyć eksport do CSV, trzeba:
1. Dodać `BenchmarkExporter.h` i `BenchmarkExporter.cpp` do CMakeLists.txt
2. Zmodyfikować funkcję `runBenchmark()` w main.cpp
3. Dodać flagi: `--export-cpm`, `--export-pert`, `--export-all`

Czy chcesz, żebym to zrobił? 🤔
