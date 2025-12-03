#include <algorithm>
#include <climits>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include "nearestNeighbor.h"
#include "data_generator.h"
#include "ascii_viz.h"

using namespace std;

int main() {
    string filename = "./problemData/data_002.txt";
    
    cout << "Loading data from " << filename << "..." << endl;
    vector<vector<int>> distanceMatrix = loadDataAndCreateDistanceMatrix(filename);
    // vector<vector<int>> distanceMatrix = generateData(40, false);
    
    if (distanceMatrix.empty()) {
        cerr << "Failed to load data!" << endl;
        return 1;
    }
    
    cout << "Loaded data for " << distanceMatrix.size() << " cities." << endl;
    
    // Utworzenie obiektu algorytmu najbliższego sąsiada
    NearestNeighbor nn(distanceMatrix);
    
    cout << "\nRunning Nearest Neighbor algorithm..." << endl;
    
    // Znajdź najlepszą trasę spośród wszystkich startowych miast
    int bestDistance = INT_MAX;
    vector<int> bestTour;
    int bestStartCity = 0;
    
    // for (int startCity = 0; startCity < nn.getNumCities(); startCity++) {
    //     auto [tour, distance] = nn.getTour(startCity);
        
    //     if (distance < bestDistance) {
    //         bestDistance = distance;
    //         bestTour = tour;
    //         bestStartCity = startCity;
    //     }
    // }
        auto [tour, distance] = nn.getTour(bestStartCity);
        
        if (distance < bestDistance) {
            bestDistance = distance;
            bestTour = tour;
            bestStartCity = bestStartCity;
        }
    
    auto coords = loadCityCoordinates(filename);

    cout << "\n=== Best Solution ===" << endl;
    cout << "Starting city: " << bestStartCity << endl;
    cout << "Total distance: " << bestDistance << endl << endl;
    printTourStepByStep(bestTour, distanceMatrix);

    if (!coords.empty()) {
        cout << "\nASCII visualization (80x20):\n";
        drawTourAscii(bestTour, coords);
    } else {
        cout << "\n(No coordinates available for ASCII visualization)\n";
    }
    
    return 0;
}
