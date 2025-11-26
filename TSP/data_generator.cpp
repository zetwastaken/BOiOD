#include "data_generator.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <string>
#include <random>

using namespace std;

struct City {
    double x;
    double y;
};

// Funkcja do wczytania danych z pliku i utworzenia macierzy odległości
vector<vector<int>> loadDataAndCreateDistanceMatrix(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Can't open file " << filename << endl;
        return {};
    }
    
    int numCities;
    file >> numCities;
    
    vector<City> cities(numCities);
    
    // Wczytanie współrzędnych
    for (int i = 0; i < numCities; i++) {
        file >> cities[i].x >> cities[i].y;
    }
    
    file.close();
    
    // Utworzenie macierzy odległości
    vector<vector<int>> distanceMatrix(numCities, vector<int>(numCities, 0));
    
    for (int i = 0; i < numCities; i++) {
        for (int j = 0; j < numCities; j++) {
            if (i != j) {
                double dx = cities[i].x - cities[j].x;
                double dy = cities[i].y - cities[j].y;
                double distance = sqrt(dx * dx + dy * dy);
                // Zaokrąglenie w dół (podłoga)
                distanceMatrix[i][j] = static_cast<int>(floor(0.5 + distance)); //! jakiś dziwny sposób liczenia makucha
            }
        }
    }
    
    return distanceMatrix;
}

// Funkcja do wygenerowania danych dla podanej liczby miast
vector<vector<int>> generateData(int numCities, bool saveToFile, const string& filename) {
    if (numCities <= 0) {
        cerr << "Error: Number of cities must be positive" << endl;
        return {};
    }
    
    // Generator liczb losowych
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, 100);  // Współrzędne z zakresu [0, 100]
    
    // Generowanie losowych współrzędnych miast
    vector<City> cities(numCities);
    for (int i = 0; i < numCities; i++) {
        cities[i].x = dis(gen);
        cities[i].y = dis(gen);
    }
    
    // Zapis do pliku (opcjonalny)
    if (saveToFile) {
        if (filename.empty()) {
            cerr << "Error: Filename is required when saveToFile is true" << endl;
            return {};
        }
        
        ofstream file(filename);
        if (!file.is_open()) {
            cerr << "Error: Can't create file " << filename << endl;
            return {};
        }
        
        file << numCities << endl;
        for (int i = 0; i < numCities; i++) {
            file << cities[i].x << " " << cities[i].y;
            if (i < numCities - 1) {
                file << "   ";  // Separator między miastami
            }
        }
        file << endl;
        
        file.close();
        cout << "Generated data for " << numCities << " cities in " << filename << endl;
    }
    
    // Utworzenie macierzy odległości
    vector<vector<int>> distanceMatrix(numCities, vector<int>(numCities, 0));
    
    for (int i = 0; i < numCities; i++) {
        for (int j = 0; j < numCities; j++) {
            if (i != j) {
                double dx = cities[i].x - cities[j].x;
                double dy = cities[i].y - cities[j].y;
                double distance = sqrt(dx * dx + dy * dy);
                // Zaokrąglenie w dół (podłoga)
                distanceMatrix[i][j] = static_cast<int>(floor(0.5 + distance));
            }
        }
    }
    
    return distanceMatrix;
}
