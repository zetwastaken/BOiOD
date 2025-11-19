# Algorytmy Grafowe - Projekt

## Struktura projektu

Ten projekt zawiera implementacje różnych algorytmów grafowych podzielonych na dwie główne kategorie:

### 1. Algorytmy Najkrótszej Ścieżki
- **Dijkstra** - Algorytm znajdowania najkrótszej ścieżki (grafy bez ujemnych wag)
- **Bellman-Ford** - Algorytm najkrótszej ścieżki (obsługuje ujemne wagi)
- **A*** - Heurystyczny algorytm najkrótszej ścieżki

### 2. Algorytmy Maksymalnego Przepływu
- **Ford-Fulkerson** - Algorytm znajdowania maksymalnego przepływu (DFS do szukania ścieżek powiększających)
- **Edmonds-Karp** - Algorytm maksymalnego przepływu (BFS do szukania ścieżek powiększających)

## Kompilacja

### Używając Makefile

#### Algorytmy najkrótszej ścieżki:
```bash
make                    # Kompiluj program główny
make run                # Kompiluj i uruchom
```

#### Algorytmy maksymalnego przepływu:
```bash
make maxflow            # Kompiluj program max flow
make run-maxflow        # Kompiluj i uruchom max flow
```

#### Benchmark:
```bash
make benchmark          # Kompiluj benchmark
make run-benchmark      # Kompiluj i uruchom benchmark
```

#### Inne:
```bash
make all-targets        # Kompiluj wszystkie programy
make clean              # Usuń skompilowane pliki
make rebuild            # Wyczyść i przebuduj
```

### Używając CMake

```bash
mkdir -p build && cd build
cmake ..
make

# Uruchom:
./bin/minPath           # Program najkrótszych ścieżek
./bin/minPath_maxflow   # Program maksymalnego przepływu
./bin/minPath_benchmark # Benchmark
```

## Opis algorytmów

### Dijkstra
- **Złożoność:** O(V²) w podstawowej implementacji
- **Zastosowanie:** Znajdowanie najkrótszej ścieżki od źródła do wszystkich wierzchołków
- **Ograniczenia:** Nie działa z ujemnymi wagami krawędzi

### Bellman-Ford
- **Złożoność:** O(V·E)
- **Zastosowanie:** Znajdowanie najkrótszej ścieżki, wykrywanie cykli ujemnych
- **Zalety:** Obsługuje ujemne wagi krawędzi

### A* (A-Star)
- **Złożoność:** Zależna od heurystyki
- **Zastosowanie:** Znajdowanie najkrótszej ścieżki między dwoma wierzchołkami
- **Zalety:** Używa heurystyki (odległość euklidesowa) do szybszego szukania

### Ford-Fulkerson
- **Złożoność:** O(E·|f*|) gdzie f* to maksymalny przepływ
- **Zastosowanie:** Znajdowanie maksymalnego przepływu w sieci
- **Metoda:** Używa DFS do znajdowania ścieżek powiększających

### Edmonds-Karp
- **Złożoność:** O(V·E²)
- **Zastosowanie:** Znajdowanie maksymalnego przepływu w sieci
- **Metoda:** Używa BFS do znajdowania najkrótszych ścieżek powiększających
- **Zalety:** Gwarantowana złożoność czasowa

## Generowanie danych

### Grafy dla najkrótszych ścieżek:
```cpp
// Graf losowy z wagami
generateAdjacencyMatrix(numVertices, density, maxWeight);

// Graf z współrzędnymi (dla A*)
generateAdjacencyMatrixWithCoordinates(numVertices, density, coordinates);
```

### Sieci przepływowe:
```cpp
// Sieć przepływowa z przepustowościami
generateFlowNetwork(numVertices, density, maxCapacity);
```

## Struktura katalogów

```
minPath/
├── aStar/                  # Algorytm A*
├── Bellman-Ford/           # Algorytm Bellmana-Forda
├── Dijkstra/               # Algorytm Dijkstry
├── Ford-Fulkerson/         # Algorytm Forda-Fulkersona
├── Edmonds-Karp/           # Algorytm Edmondsa-Karpa
├── dataGenerator/          # Generator grafów i sieci
├── output/                 # Skompilowane programy
├── problemData/            # Pliki z danymi testowymi
├── main.cpp                # Program główny (najkrótsze ścieżki)
├── main_maxflow.cpp        # Program max flow
├── benchmark.cpp           # Program benchmarkowy
├── Makefile                # Plik kompilacji Make
└── CMakeLists.txt          # Plik konfiguracji CMake
```

## Przykładowe użycie

### Program najkrótszych ścieżek (main.cpp):
```bash
./output/main
```
Wyświetli:
- Czas wykonania każdego algorytmu
- Koszt najkrótszej ścieżki od wierzchołka startowego do końcowego

### Program maksymalnego przepływu (main_maxflow.cpp):
```bash
./output/main_maxflow
```
Wyświetli:
- Czas wykonania algorytmów Ford-Fulkerson i Edmonds-Karp
- Wartość maksymalnego przepływu
- Weryfikację zgodności wyników

## Parametry

### Dla grafów najkrótszych ścieżek:
- `size` - liczba wierzchołków (domyślnie: 100)
- `density` - gęstość grafu 0.0-1.0 (domyślnie: 0.6)
- `maxWeight` - maksymalna waga krawędzi (domyślnie: 100)

### Dla sieci przepływowych:
- `size` - liczba wierzchołków (domyślnie: 100)
- `density` - gęstość sieci 0.0-1.0 (domyślnie: 0.3)
- `maxCapacity` - maksymalna przepustowość (domyślnie: 100)

## Uwagi

- Wszystkie algorytmy używają reprezentacji macierzowej grafów
- Dla grafów najkrótszych ścieżek: `INF` oznacza brak krawędzi
- Dla sieci przepływowych: `0` oznacza brak krawędzi
- Ścieżki są zwracane jako wektory indeksów wierzchołków
