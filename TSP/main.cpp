#include <iostream>
#include <vector>
#include <climits>
#include <iomanip>
#include "nearestNeighbor.h"
#include "data_generator.h"

using namespace std;

void printTourStepByStep(const vector<int>& tour, const vector<vector<int>>& distanceMatrix) {
    cout << "step : length : path" << endl;
    
    int currentDistance = 0;
    
    for (size_t i = 0; i < tour.size(); i++) {
        cout << setw(4) << (i + 1) << " : " << setw(6) << currentDistance << " : ";
        
        for (size_t j = 0; j <= i; j++) {
            cout << tour[j];
            if (j < i) cout << " ";
        }
        cout << "  " << endl;
        
        if (i < tour.size() - 1) {
            currentDistance += distanceMatrix[tour[i]][tour[i + 1]];
        }
    }
}

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
    
    cout << "\n=== Best Solution ===" << endl;
    cout << "Starting city: " << bestStartCity << endl;
    cout << "Total distance: " << bestDistance << endl << endl;
    printTourStepByStep(bestTour, distanceMatrix);
    
    return 0;
}
