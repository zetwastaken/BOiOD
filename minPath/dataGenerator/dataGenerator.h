#ifndef DATA_GENERATOR_H
#define DATA_GENERATOR_H

#include <vector>
#include <string>
#include <limits>

constexpr int INF = std::numeric_limits<int>::max();

std::vector<std::vector<int>> createAdjacencyMatrixFromFile(const std::string& filename);
std::vector<std::vector<int>> generateAdjacencyMatrix(int numVertices, double density, int maxWeight);


#endif  //! DATA_GENERATOR_H