# Metodyka Badań Wydajności Algorytmów Wyznaczania Najkrótszych Ścieżek

## 1. Cel Badania

Celem badania jest przeprowadzenie kompleksowej analizy porównawczej wydajności trzech algorytmów wyznaczania najkrótszych ścieżek w grafach:
- **Algorytm Dijkstry** — dla grafów z nieujemnymi wagami krawędzi
- **Algorytm Bellmana-Forda** — dla grafów ogólnych, z możliwością wykrywania cykli o ujemnej wadze
- **Algorytm A\*** — algorytm heurystyczny wykorzystujący funkcję oceny odległości

Badanie koncentruje się na analizie skalowalności algorytmów oraz wpływu struktury grafu (gęstości) na ich wydajność czasową.

## 2. Środowisko Testowe

### 2.1. Platforma Sprzętowo-Programowa
- **System operacyjny:** Windows 10/11
- **Kompilator:** MSVC (Microsoft Visual C++) / g++ (MSYS2)
- **Standard C++:** C++17
- **Konfiguracja kompilacji:** Debug (z możliwością późniejszego przetestowania w trybie Release)
- **Narzędzia pomiarowe:** `std::chrono::high_resolution_clock` z precyzją do mikrosekundy

### 2.2. Implementacja
Wszystkie algorytmy zaimplementowano w języku C++ z wykorzystaniem:
- Macierzy sąsiedztwa jako reprezentacji grafu
- Jednolitego interfejsu dla wszystkich algorytmów
- Deterministycznego generatora grafów testowych

## 3. Parametry Badania

### 3.1. Zmienne Niezależne

#### Rozmiar Grafu (liczba wierzchołków)
Badanie przeprowadzono dla następujących rozmiarów:
- **10, 25, 50** — grafy małe (analiza zachowania dla małych instancji)
- **100, 250, 500** — grafy średnie (typowe zastosowania praktyczne)
- **1000, 2500, 5000** — grafy duże (analiza skalowalności)

Zakres został dobrany tak, aby:
- Uchwycić różnice w złożoności obliczeniowej algorytmów
- Zidentyfikować punkty przegięcia wydajności
- Zaobserwować skalowanie dla dużych instancji problemu

#### Gęstość Grafu (density)
Gęstość grafu definiowana jako stosunek liczby krawędzi do maksymalnej możliwej liczby krawędzi w grafie skierowanym:

$$\text{density} = \frac{|E|}{|V| \cdot (|V| - 1)}$$

Badane wartości:
- **0.1** — graf rzadki (~10% możliwych krawędzi)
- **0.25** — graf nisko-gęsty
- **0.5** — graf średnio-gęsty
- **0.75** — graf wysoko-gęsty
- **0.9** — graf prawie pełny (~90% możliwych krawędzi)

Uzasadnienie: Gęstość grafu ma bezpośredni wpływ na liczbę operacji wykonywanych przez algorytmy, szczególnie dla Bellmana-Forda (O(V·E)) i Dijkstry (O((V+E)·log V)).

### 3.2. Generator Grafów Testowych

#### Tryb Euclidean (wykorzystany w badaniu)
- Każdemu wierzchołkowi przypisywane są losowe współrzędne (x, y) w przestrzeni 1000×1000
- Wagi krawędzi obliczane jako odległość euklidesowa między wierzchołkami:
  $$w(i,j) = \lceil \sqrt{(x_i - x_j)^2 + (y_i - y_j)^2} \rceil$$
- Krawędzie generowane losowo z zadaną gęstością
- **Zalety:**
  - Wagi są spójne z geometrią przestrzenną (przydatne dla A*)
  - Heurystyka A* (odległość euklidesowa) jest admisible i consistent
  - Realistyczne dla zastosowań geograficznych/routingu

#### Tryb Random (dostępny, nie wykorzystany w tym badaniu)
- Wagi krawędzi losowane z zakresu [1, maxWeight]
- Brak zależności geometrycznych
- A* działa bez heurystyki (tryb "Dijkstra + priorytet")

### 3.3. Parametry Eksperymentu
- **Liczba powtórzeń:** 5 przebiegów dla każdej konfiguracji (n, density)
- **Wierzchołek startowy:** 0
- **Wierzchołek końcowy:** n-1 (najdalszy możliwy)
- **Całkowita liczba konfiguracji:** 9 rozmiarów × 5 gęstości = 45 konfiguracji
- **Całkowita liczba przypadków testowych:** 45 konfiguracji × 5 powtórzeń × 3 algorytmy = **675 przebiegów**

## 4. Zmienne Zależne (Metryki)

### 4.1. Metryki Podstawowe (dla każdego przebiegu)
- **execution_time_us** — czas wykonania algorytmu w mikrosekundach (μs)
- **success** — flaga sukcesu (czy znaleziono ścieżkę)
- **path_cost** — łączny koszt znalezionej ścieżki
- **path_length** — liczba wierzchołków w ścieżce
- **edges** — rzeczywista liczba krawędzi w grafie
- **actual_density** — rzeczywista gęstość wygenerowanego grafu

### 4.2. Metryki Zagregowane (dla każdej konfiguracji)
Dla każdej kombinacji (algorytm, n, density):
- **mean_us, stddev_us** — średni czas wykonania i odchylenie standardowe
- **median_us, min_us, max_us** — mediana, minimum i maksimum czasu
- **success_rate** — odsetek udanych przebiegów
- **mean_cost, median_cost** — średni i medianowy koszt ścieżki
- **mean_path_len** — średnia długość ścieżki
- **edges_mean, edges_std** — średnia liczba krawędzi i jej odchylenie
- **matches_dijkstra_cost** — flaga zgodności wyniku z algorytmem Dijkstry (weryfikacja poprawności)

## 5. Procedura Badawcza

### 5.1. Proces Pojedynczego Przebiegu
Dla każdej konfiguracji (n, density, run_id):

1. **Generacja grafu** — wygenerowanie losowego grafu o zadanych parametrach
2. **Pomiar czasu:**
   - Zapis czasu startu (`t_start = high_resolution_clock::now()`)
   - Wykonanie algorytmu
   - Zapis czasu końca (`t_end = high_resolution_clock::now()`)
   - Obliczenie różnicy w mikrosekundach
3. **Zapis wyników** — zapisanie metryk do pliku CSV
4. **Powtórzenie** — kolejny przebieg dla tej samej konfiguracji (z nowym grafem)

### 5.2. Agregacja Wyników
Po zakończeniu wszystkich 5 powtórzeń dla danej konfiguracji:
- Obliczenie statystyk opisowych (średnia, odchylenie, mediana)
- Weryfikacja zgodności wyników między algorytmami
- Zapis zagregowanych metryk do osobnego pliku CSV

## 6. Weryfikacja Poprawności

### 6.1. Kontrola Jakości Wyników
- **Zgodność kosztów:** Dla każdej konfiguracji sprawdzane jest, czy wszystkie algorytmy zwróciły ten sam koszt ścieżki
- **Metryka matches_dijkstra_cost:** Flaguje przypadki rozbieżności wyników
- **Flaga success:** Identyfikuje przypadki braku ścieżki (graf niespójny)

### 6.2. Warunki Poprawności
- Dla grafów z nieujemnymi wagami (generator euclidean): wszystkie algorytmy powinny dać identyczne wyniki
- Brak ujemnych cykli: Bellman-Ford nie powinien zgłaszać błędu
- Heurystyka A*: dla generatora euclidean heurystyka jest admisible, więc A* zwraca rozwiązanie optymalne

## 7. Format Danych Wyjściowych

### 7.1. Plik Surowy (`benchmark_results_1.csv`)
Zawiera szczegółowe wyniki każdego pojedynczego przebiegu:
```
algorithm,graph_size,density,generator,max_weight,run_id,start,end,
execution_time_us,success,path_cost,path_length,edges,actual_density
```

### 7.2. Plik Zagregowany (`benchmark_results_1_agg.csv`)
Zawiera statystyki zebrane dla każdej konfiguracji:
```
algorithm,graph_size,density,generator,max_weight,runs,success_rate,
mean_us,stddev_us,median_us,min_us,max_us,mean_cost,median_cost,
mean_path_len,edges_mean,edges_std,actual_density_mean,matches_dijkstra_cost
```

## 8. Hipotezy Badawcze

### H1: Skalowalność względem rozmiaru grafu
**Hipoteza:** Czas wykonania skaluje się zgodnie z teoretyczną złożonością:
- Dijkstra: O((V+E)·log V) ≈ O(V²·log V) dla grafów gęstych
- Bellman-Ford: O(V·E) ≈ O(V³) dla grafów gęstych
- A*: O(b^d) w praktyce lepszy niż Dijkstra dzięki heurystyce

**Oczekiwanie:** Dla dużych grafów (n > 1000) różnice w czasach powinny być wyraźne, z Bellman-Fordem jako najwolniejszym.

### H2: Wpływ gęstości grafu
**Hipoteza:** Wraz ze wzrostem gęstości:
- Dijkstra i Bellman-Ford będą wolniejsze (więcej krawędzi do przetworzenia)
- A* może być szybszy dla gęstych grafów (więcej alternatywnych ścieżek, lepsza skuteczność heurystyki)

### H3: Efektywność A* z heurystyką
**Hipoteza:** A* z heurystyką euklidesową będzie znacząco szybszy niż Dijkstra i Bellman-Ford dla wszystkich rozmiarów grafów, szczególnie dla grafów rzadkich i dalekich par wierzchołków (start=0, end=n-1).

## 9. Ograniczenia Badania

### 9.1. Ograniczenia Metodologiczne
- **Jednowątkowość:** Algorytmy działają w trybie sekwencyjnym (brak równoległości)
- **Tryb Debug:** Pomiary wykonane w trybie Debug (z możliwością optymalizacji w Release)
- **Reprezentacja grafu:** Macierz sąsiedztwa (O(V²) pamięci) — dla bardzo dużych grafów rzadkich lista sąsiedztwa byłaby efektywniejsza
- **Pojedyncza para wierzchołków:** Testowane tylko dla start=0, end=n-1

### 9.2. Ograniczenia Sprzętowe
- Wyniki zależą od platformy sprzętowej (procesor, pamięć cache, RAM)
- Procesy systemowe w tle mogą wpływać na pomiary czasu
- Mitigacja: wielokrotne powtórzenia (5×) i agregacja statystyczna

## 10. Narzędzia i Automatyzacja

### 10.1. Program Benchmarkowy
Stworzony dedykowany program `minPath_benchmark.exe` umożliwiający:
- Automatyczne generowanie grafów testowych
- Pomiar czasu z wysoką precyzją
- Zapis wyników w formacie CSV (surowy + zagregowany)
- Parametryzację przez argumenty wiersza poleceń

### 10.2. Przykład Uruchomienia
```powershell
.\minPath_benchmark.exe `
  --sizes 10,25,50,100,250,500,1000,2500,5000 `
  --densities 0.1,0.25,0.5,0.75,0.9 `
  --repeats 5 `
  --generator euclidean `
  --output benchmark_results_1.csv
```

### 10.3. Analiza Wyników
Wyniki w formacie CSV umożliwiają:
- Import do arkuszy kalkulacyjnych (Excel, Google Sheets)
- Wizualizację w Pythonie (matplotlib, seaborn, pandas)
- Analizę statystyczną (R, SPSS)
- Łatwą reprodukcję badań

## 11. Planowane Rozszerzenia

### 11.1. Dodatkowe Scenariusze Testowe
- Różne pary wierzchołków (start, end) — losowe, bliskie, dalekie
- Grafy specjalne: łańcuch, gwiazda, sieć siatki
- Grafy z ujemnymi wagami (tylko Bellman-Ford)

### 11.2. Dodatkowe Metryki
- Liczba odwiedzonych wierzchołków (dla A* vs Dijkstra)
- Zużycie pamięci (heap size, liczba alokacji)
- Współczynnik przyspieszenia (speedup ratio)

### 11.3. Optymalizacje
- Porównanie trybu Debug vs Release
- Reprezentacja listy sąsiedztwa vs macierz
- Wersje równoległe/wielowątkowe algorytmów
