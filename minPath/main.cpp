#include <iostream>
#include <vector>
#include <string>
#include <chrono> 

#include "dataGenerator/dataGenerator.h"
#include "Dijkstra/dijkstra_algorithm.h"
#include "Bellman-Ford/BellmanFord.h"
#include "aStar/aStar.h"


void printPath(const std::vector<int>& path) {
    if (path.empty()) {
        std::cout << "[]";
        return;
    }

    std::cout << "[";
    for (size_t i = 0; i < path.size(); ++i) {
        std::cout << path[i]; 
        if (i < path.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "]"; 
}

void printMatrix(const std::vector<std::vector<int>>& matrix) {
    if (matrix.empty()) {
        std::cout << "Matrix is empty." << std::endl;
        return;
    }
    for (const auto& row : matrix) {
        for (int val : row) {
            if (val == INF) {
                std::cout << "INF\t";
            } else {
                std::cout << val << "\t";
            }
        }
        std::cout << std::endl;
    }
}


int main(int argc, char* argv[]) {
    // data preparation

    std::vector<std::vector<int>> matrix;
    std::vector<Point> coordinates;
    int size = 0;

    //! Uncomment to load from file
    // const std::string filename = "problemData/MinPaths_data10.txt";
    // matrix = createAdjacencyMatrixFromFile(filename);
    // if (!matrix.empty()) {
    //     size = matrix.size();
    // }

    //Generate a random adjacency matrix with coordinates
    // Edge weights will be based on Euclidean distances between coordinates
    size = 500;            // Number of vertices
    double density = 0.6;  // Graph density (%)
    int maxWeight = 100;   // Maximum edge weight
    // matrix = generateAdjacencyMatrix(size, density, maxWeight);
    matrix = generateAdjacencyMatrixWithCoordinates(size, density, coordinates);

    if (matrix.empty()) {
        std::cerr << "Error: Failed to create graph." << std::endl;
        return 1;
    }
    
    //! Uncomment to print generated matrix
    // std::cout << "Generated matrix:" << std::endl;
    // printMatrix(matrix);
    // std::cout << "\n";

    // Define start and end vertices
    int startVertex = 0;
    int endVertex = size - 1;


    // Run algorithms and measure time

    // --- Dijkstra's Algorithm ---
    Dijkstra dijkstra(matrix);
    auto time_start_dijkstra = std::chrono::high_resolution_clock::now();
    dijkstra.findShortestPaths(startVertex);
    auto time_stop_dijkstra = std::chrono::high_resolution_clock::now();
    auto duration_dijkstra_micro = std::chrono::duration_cast<std::chrono::microseconds>(time_stop_dijkstra - time_start_dijkstra);
    auto duration_dijkstra_milli = std::chrono::duration_cast<std::chrono::milliseconds>(time_stop_dijkstra - time_start_dijkstra);

    // --- Bellman-Ford Algorithm ---
    BellmanFord bellmanFord(matrix);
    auto time_start_bf = std::chrono::high_resolution_clock::now();
    bool bellmanFordSuccess = bellmanFord.findShortestPaths(startVertex);
    auto time_stop_bf = std::chrono::high_resolution_clock::now();
    auto duration_bf_micro = std::chrono::duration_cast<std::chrono::microseconds>(time_stop_bf - time_start_bf);
    auto duration_bf_milli = std::chrono::duration_cast<std::chrono::milliseconds>(time_stop_bf - time_start_bf);

    // --- A* Algorithm ---
    AStar aStar(matrix, coordinates);
    auto time_start_astar = std::chrono::high_resolution_clock::now();
    bool aStarSuccess = aStar.findShortestPath(startVertex, endVertex);
    auto time_stop_astar = std::chrono::high_resolution_clock::now();
    auto duration_astar_micro = std::chrono::duration_cast<std::chrono::microseconds>(time_stop_astar - time_start_astar);
    auto duration_astar_milli = std::chrono::duration_cast<std::chrono::milliseconds>(time_stop_astar - time_start_astar);


    // Display results

    std::cout << "\n===== DIJKSTRA'S ALGORITHM RESULTS =====\n";
    std::cout << "Execution time: " << duration_dijkstra_micro.count() << " microseconds -> " << duration_dijkstra_milli.count() << " milliseconds.\n";
    std::cout << "Shortest path from " << startVertex << " to " << endVertex << ":\n";
    std::cout << "Cost: " << dijkstra.getShortestDistanceTo(endVertex) << "\nPath: ";
    printPath(dijkstra.getShortestPathTo(endVertex));
    std::cout << std::endl;
    std::cout << "--------------------------------------\n";
    //! Uncomment to print all paths
    // for (int i = 0; i < size; ++i) {
    //     std::cout << "To " << i << ":\tCost: " << dijkstra.getShortestDistanceTo(i) << "\tPath: ";
    //     printPath(dijkstra.getShortestPathTo(i));
    //     std::cout << std::endl;
    // }

    std::cout << "\n===== BELLMAN-FORD ALGORITHM RESULTS =====\n";
    std::cout << "Execution time: " << duration_bf_micro.count() << " microseconds -> " << duration_bf_milli.count() << " milliseconds.\n";
    if (bellmanFordSuccess) {
        std::cout << "Shortest path from " << startVertex << " to " << endVertex << ":\n";
        std::cout << "Cost: " << bellmanFord.getShortestDistanceTo(endVertex) << "\nPath: ";
        printPath(bellmanFord.getShortestPathTo(endVertex));
        std::cout << std::endl;
        std::cout << "-------------------------------------------\n";
        //! Uncomment to print all paths
        // for (int i = 0; i < size; ++i) {
        //     std::cout << "To " << i << ":\tCost: " << bellmanFord.getShortestDistanceTo(i) << "\tPath: ";
        //     printPath(bellmanFord.getShortestPathTo(i));
        //     std::cout << std::endl;
        // }
    } else {
        std::cout << "Negative weight cycle detected! Results are undefined.\n";
    }

    std::cout << "\n===== A* ALGORITHM RESULTS =====\n";
    std::cout << "Execution time: " << duration_astar_micro.count() << " microseconds -> " << duration_astar_milli.count() << " milliseconds.\n";
    std::cout << "Searching path from " << startVertex << " to " << endVertex << ":\n";
    std::cout << "---------------------------------\n";
    if (aStarSuccess) {
        std::cout << "Path found!\n";
        std::cout << "Cost: " << aStar.getShortestDistance() << "\nPath: ";
        printPath(aStar.getShortestPath());
        std::cout << std::endl;
    } else {
        std::cout << "Path does not exist.\n";
    }

    return 0;
}