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

using namespace std;

const string COLOR_NODE = "\033[32m";
const string COLOR_EDGE = "\033[33m";
const string COLOR_START = "\033[36m";
const string COLOR_RESET = "\033[0m";

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

        if (i + 1 < tour.size()) {
            currentDistance += distanceMatrix[tour[i]][tour[i + 1]];
        }
    }
}

void drawTourAscii(const vector<int>& tour, const vector<pair<double, double>>& coords, int width = 80, int height = 20) {
    if (coords.empty() || width <= 0 || height <= 0) return;

    double maxX = 0.0;
    double maxY = 0.0;
    for (const auto& c : coords) {
        maxX = max(maxX, c.first);
        maxY = max(maxY, c.second);
    }
    if (maxX == 0.0) maxX = 1.0;
    if (maxY == 0.0) maxY = 1.0;

    vector<string> canvas(height, string(width, ' '));

    auto scale = [&](double x, double y) {
        int sx = static_cast<int>(x / maxX * (width - 1));
        int sy = static_cast<int>(y / maxY * (height - 1));
        sy = max(0, min(height - 1, sy));
        sx = max(0, min(width - 1, sx));
        return pair<int, int>{sx, sy};
    };

    for (size_t i = 0; i < coords.size(); i++) {
        auto [x, y] = scale(coords[i].first, coords[i].second);
        canvas[y][x] = 'O';
    }

    if (!tour.empty()) {
        int startIdx = tour.front();
        if (startIdx >= 0 && startIdx < static_cast<int>(coords.size())) {
            auto [sx, sy] = scale(coords[startIdx].first, coords[startIdx].second);
            canvas[sy][sx] = 'S';
        }
    }

    auto drawLine = [&](int x0, int y0, int x1, int y1) {
        int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
        int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
        int err = dx + dy;
        while (true) {
            if (canvas[y0][x0] == ' ') canvas[y0][x0] = '*';
            if (x0 == x1 && y0 == y1) break;
            int e2 = 2 * err;
            if (e2 >= dy) { err += dy; x0 += sx; }
            if (e2 <= dx) { err += dx; y0 += sy; }
        }
    };

    for (size_t i = 0; i + 1 < tour.size(); i++) {
        int from = tour[i];
        int to = tour[i + 1];
        if (from < 0 || to < 0 || from >= static_cast<int>(coords.size()) || to >= static_cast<int>(coords.size())) {
            continue;
        }
        auto [x0, y0] = scale(coords[from].first, coords[from].second);
        auto [x1, y1] = scale(coords[to].first, coords[to].second);
        drawLine(x0, y0, x1, y1);
    }

    for (const auto& row : canvas) {
        for (char c : row) {
            if (c == 'S') {
                cout << COLOR_START << 'S' << COLOR_RESET;
            } else if (c == 'O') {
                cout << COLOR_NODE << 'O' << COLOR_RESET;
            } else if (c == '*') {
                cout << COLOR_EDGE << '*' << COLOR_RESET;
            } else {
                cout << c;
            }
        }
        cout << '\n';
    }
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
