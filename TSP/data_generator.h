#ifndef DATA_GENERATOR_H
#define DATA_GENERATOR_H

#include <vector>
#include <string>

// Funkcja do wczytania danych z pliku i utworzenia macierzy odległości
std::vector<std::vector<int>> loadDataAndCreateDistanceMatrix(const std::string& filename);

// Funkcja do wygenerowania danych dla podanej liczby miast
std::vector<std::vector<int>> generateData(int numCities, bool saveToFile = false, const std::string& filename = "");

#endif // DATA_GENERATOR_H
