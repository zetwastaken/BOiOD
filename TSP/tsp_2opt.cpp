#include <algorithm>
#include <climits>
#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include "ascii_viz.h"
#include "data_generator.h"

using namespace std;

int calculateTourDistance(const vector<int>& tour, const vector<vector<int>>& dist) {
    int total = 0;
    for (size_t i = 0; i + 1 < tour.size(); i++) {
        total += dist[tour[i]][tour[i + 1]];
    }
    return total;
}

int delta2Opt(const vector<int>& tour, const vector<vector<int>>& dist, size_t i, size_t k) {
    // tour is closed (last == first), swap edges (i-1,i) and (k,k+1)
    int n = static_cast<int>(tour.size());
    int a = tour[(i - 1 + n) % n];
    int b = tour[i];
    int c = tour[k];
    int d = tour[(k + 1) % n];
    int before = dist[a][b] + dist[c][d];
    int after = dist[a][c] + dist[b][d];
    return after - before;
}

void apply2Opt(vector<int>& tour, size_t i, size_t k) {
    reverse(tour.begin() + static_cast<long>(i), tour.begin() + static_cast<long>(k) + 1);
}

vector<int> twoOpt(const vector<vector<int>>& dist) {
    int n = static_cast<int>(dist.size());
    if (n == 0) return {};
    if (n == 1) return {0, 0};

    vector<int> tour(n + 1);
    for (int i = 0; i < n; i++) tour[i] = i;
    tour[n] = 0;

    bool improved = true;
    while (improved) {
        improved = false;
        int bestDelta = 0;
        size_t bestI = 1, bestK = 1;
        for (size_t i = 1; i < tour.size() - 2; i++) {
            for (size_t k = i + 1; k < tour.size() - 1; k++) {
                int d = delta2Opt(tour, dist, i, k);
                if (d < bestDelta) {
                    bestDelta = d;
                    bestI = i;
                    bestK = k;
                    improved = true;
                }
            }
        }
        if (improved) {
            apply2Opt(tour, bestI, bestK);
        }
    }

    return tour;
}

int main(int argc, char* argv[]) {
    string filename = "./problemData/data_002.txt";
    if (argc > 1) {
        filename = argv[1];
    }

    cout << "Loading data from " << filename << "..." << endl;
    vector<vector<int>> distanceMatrix = loadDataAndCreateDistanceMatrix(filename);
    if (distanceMatrix.empty()) {
        cerr << "Failed to load data!" << endl;
        return 1;
    }

    cout << "Loaded data for " << distanceMatrix.size() << " cities." << endl;

    cout << "\nRunning 2-Opt algorithm..." << endl;
    vector<int> tour = twoOpt(distanceMatrix);
    int totalDistance = calculateTourDistance(tour, distanceMatrix);

    cout << "\n=== 2-Opt Result ===\n";
    cout << "Starting city: " << (tour.empty() ? -1 : tour.front()) << "\n";
    cout << "Total distance: " << totalDistance << "\n\n";
    printTourStepByStep(tour, distanceMatrix);

    auto coords = loadCityCoordinates(filename);
    if (!coords.empty()) {
        cout << "\nASCII visualization (80x20):\n";
        drawTourAscii(tour, coords);
    } else {
        cout << "\n(No coordinates available for ASCII visualization)\n";
    }

    return 0;
}
