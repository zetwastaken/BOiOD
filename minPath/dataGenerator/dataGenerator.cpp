#include "dataGenerator.h"
#include "../aStar/aStar.h"

#include <iostream>
#include <fstream>
#include <random>
#include <ctime>
#include <cmath>

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

std::vector<std::vector<int>> generateAdjacencyMatrixWithCoordinates(int numVertices, double density, std::vector<Point>& coordinates)
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

    //* Initialize random number generator
    static std::mt19937 gen(static_cast<unsigned int>(time(0)));
    std::uniform_int_distribution<> coordDist(0, 1000); // Larger range for better distribution
    std::uniform_real_distribution<> densityDist(0.0, 1.0);

    //* Generate random coordinates for each vertex
    coordinates.clear();
    coordinates.reserve(numVertices);
    for (int i = 0; i < numVertices; ++i)
    {
        coordinates.push_back({coordDist(gen), coordDist(gen)});
    }

    //* Initialize adjacency matrix
    std::vector<std::vector<int>> adjacencyMatrix(numVertices, std::vector<int>(numVertices, INF));
    for (int i = 0; i < numVertices; ++i)
    {
        adjacencyMatrix[i][i] = 0;
    }

    //* Generate edges based on density, with weights equal to Euclidean distance
    for (int i = 0; i < numVertices; ++i)
    {
        for (int j = 0; j < numVertices; ++j)
        {
            if (i == j) continue;

            //? If there is an edge, calculate its weight based on Euclidean distance
            if (densityDist(gen) < density)
            {
                int dx = coordinates[i].x - coordinates[j].x;
                int dy = coordinates[i].y - coordinates[j].y;
                int distance = static_cast<int>(std::ceil(std::sqrt(dx * dx + dy * dy)));
                
                // Ensure minimum weight of 1 to avoid zero-weight edges
                adjacencyMatrix[i][j] = std::max(1, distance);
            }
        }
    }

    return adjacencyMatrix;
}

std::vector<std::vector<int>> generateFlowNetwork(int numVertices, double density, int maxCapacity)
{
    if (numVertices <= 0)
    {
        std::cerr << "Error: numVertices must be positive." << std::endl;
        return {};
    }
    if (density < 0.0 || density > 1.0)
    {
        std::cerr << "Error: Network density must be in range [0.0, 1.0]" << std::endl;
        return {};
    }
    if (maxCapacity <= 0)
    {
        std::cerr << "Error: maxCapacity must be positive" << std::endl;
        return {};
    }

    //* Initialize random number generator
    static std::mt19937 gen(static_cast<unsigned int>(time(0)));
    std::uniform_real_distribution<> densityDist(0.0, 1.0);
    std::uniform_int_distribution<> capacityDist(1, maxCapacity);

    //* Inicjalizacja macierzy przepustowości zerami (brak krawędzi = przepustowość 0)
    std::vector<std::vector<int>> capacityMatrix(numVertices, std::vector<int>(numVertices, 0));

    //* Generuj krawędzie skierowane z przepustowościami
    for (int i = 0; i < numVertices; ++i)
    {
        for (int j = 0; j < numVertices; ++j)
        {
            if (i == j) continue;

            //? Jeśli istnieje krawędź, nadaj jej przepustowość
            if (densityDist(gen) < density)
            {
                capacityMatrix[i][j] = capacityDist(gen);
            }
        }
    }

    return capacityMatrix;
}
