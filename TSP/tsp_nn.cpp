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

using namespace std;

const string COLOR_NODE = "\033[32m";
const string COLOR_EDGE = "\033[33m";
const string COLOR_START = "\033[36m";
const string COLOR_RESET = "\033[0m";

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
