#include "dataGenerator.h"

#include <iostream>
#include <fstream>
#include <random>
#include <ctime>

std::vector<std::vector<int>> createAdjacencyMatrixFromFile(const std::string &filename)
{
    std::ifstream file(filename);
    if(!file.is_open())
    {
        std::cerr << "Error: Can not open file " << filename << std::endl;
        return {};
    }

    int numVertices;
    file >> numVertices;

    if(numVertices <=0)
    {
        std::cerr << "Error: Invalid number of vertices: " << numVertices << std::endl;
        return {};
    }

    //* Initialize matrix
    std::vector<std::vector<int>> adjacencyMatrix(numVertices, std::vector<int>(numVertices));

    //* Read from file and convert into antoher representaion
    for (int i = 0; i < numVertices; ++i)
    {
        for (int j = 0; j < numVertices; ++j)
        {
            int weight;
            file >> weight;

            if (weight == 0) adjacencyMatrix[i][j] = INF;
            else adjacencyMatrix [i][j] = weight;

        }
    }

    //* make sure the diagonal i made of 0
    for (int i = 0; i < numVertices; ++i)
    {
        adjacencyMatrix[i][i] = 0;
    }

    file.close();
    return adjacencyMatrix;
}

std::vector<std::vector<int>> generateAdjacencyMatrix(int numVertices, double density, int maxWeight)
{
    if (numVertices <= 0)
    {
        std::cerr << "Error: numVertices must be positive." << std::endl;
        return {};
    }
    if (density < 0.0 || density > 1.0)
    {
        std::cerr << "Error: Graph density must be in range [0.0, 1.0]" << std::endl;
        return {};
    }
        if (maxWeight <= 0)
    {
        std::cerr << "Error: maxWeight must be positive" << std::endl;
        return {};
    }

    //* Initialize random number generator
    static std::mt19937 gen(static_cast<unsigned int>(time(0)));
    std::uniform_real_distribution<> densityDist(0.0, 1.0);
    std::uniform_int_distribution<> weightDist(1, maxWeight);

    std::vector<std::vector<int>> adjacencyMatrix(numVertices, std::vector<int>(numVertices, INF));
    for (int i = 0; i < numVertices; ++i)
    {
        adjacencyMatrix[i][i] = 0;
    }

    for (int i = 0; i < numVertices; ++i)
    {
        for (int j = 0; j < numVertices; ++j)
        {
            if (i == j) continue;

            //? If there is an edge, generate its weight
            if (densityDist(gen) < density) adjacencyMatrix[i][j] = weightDist(gen);
        }
    }

    return adjacencyMatrix;
}

void printMatrix(const std::vector<std::vector<int>> &matrix)
{
    if (matrix.empty())
    {
        std::cout << "Matrix is empty." << std::endl;
        return;
    }

    for (const auto& row : matrix)
    {
        for (int val : row)
        {
            if (val == INF) std::cout << "INF\t";
            else std::cout << val <<"\t";
        }
        std::cout << std::endl;
    }
}
