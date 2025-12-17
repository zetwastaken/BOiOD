#include <algorithm>
#include <climits>
#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include "ascii_viz.h"
#include "data_generator.h"

using namespace std;

struct TabuParams {
    int maxIterations = 500;
    int tabuTenure = 15;
};

int calculateTourDistance(const vector<int>& tour, const vector<vector<int>>& dist) {
    int total = 0;
    for (size_t i = 0; i + 1 < tour.size(); i++) {
        total += dist[tour[i]][tour[i + 1]];
    }
    return total;
}

int delta2Opt(const vector<int>& tour, const vector<vector<int>>& dist, size_t i, size_t k) {
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

vector<int> initialTour(int n) {
    vector<int> tour(n + 1);
    for (int i = 0; i < n; i++) tour[i] = i;
    tour[n] = 0;
    return tour;
}

vector<int> tabuSearch2Opt(const vector<vector<int>>& dist, const TabuParams& params) {
    int n = static_cast<int>(dist.size());
    if (n == 0) return {};
    if (n == 1) return {0, 0};

    vector<int> current = initialTour(n);
    int currentDist = calculateTourDistance(current, dist);
    vector<int> bestTour = current;
    int bestDist = currentDist;

    unordered_map<long long, int> tabu;  // key -> expiration iteration
    auto key = [](int a, int b) {
        if (a > b) swap(a, b);
        return (static_cast<long long>(a) << 32) | static_cast<unsigned int>(b);
    };

    for (int iter = 0; iter < params.maxIterations; iter++) {
        int bestDelta = INT_MAX;
        size_t bestI = 1, bestK = 1;
        int candidateDist = currentDist;

        for (size_t i = 1; i < current.size() - 2; i++) {
            for (size_t k = i + 1; k < current.size() - 1; k++) {
                int d = delta2Opt(current, dist, i, k);
                int newDist = currentDist + d;
                long long moveKey = key(current[i], current[k]);

                bool isTabu = tabu.count(moveKey) && tabu[moveKey] > iter;
                bool aspiration = newDist < bestDist;  // allow if improves global best

                if ((d < bestDelta && (!isTabu || aspiration))) {
                    bestDelta = d;
                    bestI = i;
                    bestK = k;
                    candidateDist = newDist;
                }
            }
        }

        if (bestDelta == INT_MAX) {
            break;  // no move found
        }

        apply2Opt(current, bestI, bestK);
        currentDist = candidateDist;

        long long moveKey = key(current[bestI], current[bestK]);
        tabu[moveKey] = iter + params.tabuTenure;

        if (currentDist < bestDist) {
            bestDist = currentDist;
            bestTour = current;
        }
    }

    return bestTour;
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

    TabuParams params;
    cout << "\nRunning Tabu Search (2-Opt neighborhood)...\n";
    vector<int> tour = tabuSearch2Opt(distanceMatrix, params);
    int totalDistance = calculateTourDistance(tour, distanceMatrix);

    cout << "\n=== Tabu Search Result ===\n";
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
