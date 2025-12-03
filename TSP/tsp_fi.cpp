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

pair<int, int> farthestPair(const vector<vector<int>>& dist) {
    int n = static_cast<int>(dist.size());
    int bestA = 0, bestB = (n > 1 ? 1 : 0);
    int best = -1;
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            if (dist[i][j] > best) {
                best = dist[i][j];
                bestA = i;
                bestB = j;
            }
        }
    }
    return {bestA, bestB};
}

vector<int> farthestInsertion(const vector<vector<int>>& dist) {
    int n = static_cast<int>(dist.size());
    if (n == 0) return {};
    if (n == 1) return {0, 0};

    auto [a, b] = farthestPair(dist);
    vector<int> tour = {a, b, a};
    vector<bool> inTour(n, false);
    inTour[a] = inTour[b] = true;

    while (static_cast<int>(tour.size()) - 1 < n) {  // exclude duplicate last node
        int candidate = -1;
        int farDist = -1;

        for (int city = 0; city < n; city++) {
            if (inTour[city]) continue;
            int nearest = INT_MAX;
            for (size_t k = 0; k + 1 < tour.size(); k++) {  // iterate unique nodes
                nearest = min(nearest, dist[city][tour[k]]);
            }
            if (nearest > farDist) {
                farDist = nearest;
                candidate = city;
            }
        }

        if (candidate == -1) break;

        int bestPos = 0;
        int bestIncrease = INT_MAX;
        for (size_t pos = 0; pos + 1 < tour.size(); pos++) {
            int from = tour[pos];
            int to = tour[pos + 1];
            int increase = dist[from][candidate] + dist[candidate][to] - dist[from][to];
            if (increase < bestIncrease) {
                bestIncrease = increase;
                bestPos = static_cast<int>(pos);
            }
        }

        tour.insert(tour.begin() + bestPos + 1, candidate);
        inTour[candidate] = true;
    }

    // Ensure the tour is closed
    if (tour.front() != tour.back()) {
        tour.push_back(tour.front());
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

    cout << "\nRunning Farthest Insertion algorithm..." << endl;
    vector<int> tour = farthestInsertion(distanceMatrix);
    int totalDistance = calculateTourDistance(tour, distanceMatrix);

    cout << "\n=== Farthest Insertion Result ===\n";
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
