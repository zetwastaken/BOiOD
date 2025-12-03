#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include <utility>
#include <vector>
#include "data_generator.h"
#include "ascii_viz.h"

using namespace std;

struct SaConfig {
    double tempStart = 10000.0;
    double tempEnd = 1e-3;
    double coolingRate = 0.995;
    int innerIterations = 2000;
};

struct SaResult {
    vector<int> path;
    int distance = 0;
};

int calculateDistance(const vector<int>& path, const vector<vector<int>>& matrix) {
    int total = 0;
    const int n = static_cast<int>(path.size());
    for (int i = 0; i < n; i++) {
        int from = path[i];
        int to = path[(i + 1) % n];
        total += matrix[from][to];
    }
    return total;
}

pair<int, int> randomSwapIndices(int n, mt19937& gen) {
    uniform_int_distribution<> dist(0, n - 1);
    int a = dist(gen);
    int b = dist(gen);
    while (b == a) {
        b = dist(gen);
    }
    if (a > b) swap(a, b);
    return {a, b};
}

SaResult simulatedAnnealing(const vector<vector<int>>& matrix, const SaConfig& cfg) {
    const int n = static_cast<int>(matrix.size());
    vector<int> path(n);
    iota(path.begin(), path.end(), 0);

    random_device rd;
    mt19937 gen(rd());
    shuffle(path.begin(), path.end(), gen);

    int currentDistance = calculateDistance(path, matrix);
    vector<int> bestPath = path;
    int bestDistance = currentDistance;

    double temp = cfg.tempStart;
    uniform_real_distribution<> real01(0.0, 1.0);

    while (temp > cfg.tempEnd) {
        for (int i = 0; i < cfg.innerIterations; i++) {
            auto [a, b] = randomSwapIndices(n, gen);
            reverse(path.begin() + a, path.begin() + b + 1);  // 2-opt move

            int newDistance = calculateDistance(path, matrix);
            int delta = newDistance - currentDistance;

            if (delta < 0 || exp(-delta / temp) > real01(gen)) {
                currentDistance = newDistance;
                if (newDistance < bestDistance) {
                    bestDistance = newDistance;
                    bestPath = path;
                }
            } else {
                reverse(path.begin() + a, path.begin() + b + 1);  // revert
            }
        }
        temp *= cfg.coolingRate;
    }

    return {bestPath, bestDistance};
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

    SaConfig cfg;
    auto result = simulatedAnnealing(distanceMatrix, cfg);

    cout << "\n=== Simulated Annealing Result ===\n";
    vector<int> tourWithReturn = result.path;
    tourWithReturn.push_back(result.path.front());  // close the tour
    cout << "Starting city: " << tourWithReturn.front() << "\n";
    cout << "Total distance: " << result.distance << "\n\n";
    printTourStepByStep(tourWithReturn, distanceMatrix);

    auto coords = loadCityCoordinates(filename);
    if (!coords.empty()) {
        cout << "\nASCII visualization (80x20):\n";
        drawTourAscii(tourWithReturn, coords);
    } else {
        cout << "\n(No coordinates available for ASCII visualization)\n";
    }

    return 0;
}
