#include <iostream>

#include "dataGenerator/dataGenerator.h"
#include "Dijkstra/dijkstra_algorithm.h"

void printPath(const std::vector<int>& path)
{
    if (path.empty())
    {
        std::cout << "[]";
        return;
    }
    std::cout << "[";
    for (int i = 0; i < path.size(); ++i)
    {
        std::cout << path[i] << (i == path.size() - 1 ? "": ", ");
    }
    std::cout << "]";
}

int main(int argc, char* argv[])
{
    std::vector<std::vector<int>> matrix = createAdjacencyMatrixFromFile("problemData/MinPaths_data5.txt");

    //! Uncomment to generate adjacency matrix
    // int size = 6; // 6 verticles
    // double density = 0.4; //40% chance of there being an edge
    // int maxWeight = 25; //max weight is 25 -- max distance from Vi to Vj
    // std::vector<std::vector<int>> generatedMatrix = generateAdjacencyMatrix(size, density, maxWeight);

    if (matrix.empty()) return 1;

    int startVertex = 0;
    Dijkstra dijkstra(matrix);
    dijkstra.findShortestPaths(startVertex);

     // --- Krok 3: Wyświetlenie wyników ---
    std::cout << "Wyniki algorytmu Dijkstry dla wierzcholka startowego " << startVertex << ":" << std::endl;
    std::cout << "----------------------------------------------------" << std::endl;

    for (int i = 0; i < matrix.size(); ++i) {
        std::cout << "Do wierzcholka " << i << ":\n";
        
        int distance = dijkstra.getShortestDistanceTo(i);
        std::vector<int> path = dijkstra.getShortestPathTo(i);

        std::cout << "  - Dystans: " << distance << "\n";
        std::cout << "  - Sciezka: ";
        printPath(path);
        std::cout << "\n" << std::endl;
    }


    return 0;
}

