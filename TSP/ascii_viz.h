#ifndef ASCII_VIZ_H
#define ASCII_VIZ_H

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

// Simple ANSI colors for nodes, edges and start marker
inline const std::string COLOR_NODE = "\033[32m";
inline const std::string COLOR_EDGE = "\033[33m";
inline const std::string COLOR_START = "\033[36m";
inline const std::string COLOR_RESET = "\033[0m";

inline void printTourStepByStep(const std::vector<int>& tour, const std::vector<std::vector<int>>& distanceMatrix) {
    std::cout << "step : length : path" << std::endl;

    int currentDistance = 0;
    for (size_t i = 0; i < tour.size(); i++) {
        std::cout << std::setw(4) << (i + 1) << " : " << std::setw(6) << currentDistance << " : ";
        for (size_t j = 0; j <= i; j++) {
            std::cout << tour[j];
            if (j < i) std::cout << " ";
        }
        std::cout << "  " << std::endl;

        if (i + 1 < tour.size()) {
            currentDistance += distanceMatrix[tour[i]][tour[i + 1]];
        }
    }
}

inline void drawTourAscii(const std::vector<int>& tour, const std::vector<std::pair<double, double>>& coords, int width = 80, int height = 20) {
    if (coords.empty() || width <= 0 || height <= 0) return;

    double maxX = 0.0;
    double maxY = 0.0;
    for (const auto& c : coords) {
        maxX = std::max(maxX, c.first);
        maxY = std::max(maxY, c.second);
    }
    if (maxX == 0.0) maxX = 1.0;
    if (maxY == 0.0) maxY = 1.0;

    std::vector<std::string> canvas(height, std::string(width, ' '));

    auto scale = [&](double x, double y) {
        int sx = static_cast<int>(x / maxX * (width - 1));
        int sy = static_cast<int>(y / maxY * (height - 1));
        sy = std::max(0, std::min(height - 1, sy));
        sx = std::max(0, std::min(width - 1, sx));
        return std::pair<int, int>{sx, sy};
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
        int dx = std::abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
        int dy = -std::abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
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
                std::cout << COLOR_START << 'S' << COLOR_RESET;
            } else if (c == 'O') {
                std::cout << COLOR_NODE << 'O' << COLOR_RESET;
            } else if (c == '*') {
                std::cout << COLOR_EDGE << '*' << COLOR_RESET;
            } else {
                std::cout << c;
            }
        }
        std::cout << '\n';
    }
}

#endif // ASCII_VIZ_H
