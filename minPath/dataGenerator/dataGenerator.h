#ifndef DATA_GENERATOR_H
#define DATA_GENERATOR_H

#include <vector>
#include <string>
#include <limits>

constexpr int INF = std::numeric_limits<int>::max();

struct Point;

std::vector<std::vector<int>> createAdjacencyMatrixFromFile(const std::string& filename);
std::vector<std::vector<int>> generateAdjacencyMatrix(int numVertices, double density, int maxWeight);
std::vector<std::vector<int>> generateAdjacencyMatrixWithCoordinates(int numVertices, double density, std::vector<Point>& coordinates);

// Generuje macierz przepustowości dla sieci przepływowej
std::vector<std::vector<int>> generateFlowNetwork(int numVertices, double density, int maxCapacity);

#endif  //! DATA_GENERATOR_H