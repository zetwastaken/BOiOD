#include <iostream>
#include <vector>
#include <chrono>

#include "dataGenerator/dataGenerator.h"
#include "Ford-Fulkerson/FordFulkerson.h"
#include "Edmonds-Karp/EdmondsKarp.h"


void printFlowMatrix(const std::vector<std::vector<int>>& matrix) {
    if (matrix.empty()) {
        std::cout << "Matrix is empty." << std::endl;
        return;
    }
    for (const auto& row : matrix) {
        for (int val : row) {
            std::cout << val << "\t";
        }
        std::cout << std::endl;
    }
}


int main(int argc, char* argv[]) {
    // Przygotowanie danych
    std::vector<std::vector<int>> capacityMatrix;
    int size = 0;

    //! Uncomment to load from file
    // const std::string filename = "problemData/FlowNetwork_data10.txt";
    // capacityMatrix = createAdjacencyMatrixFromFile(filename);
    // if (!capacityMatrix.empty()) {
    //     size = capacityMatrix.size();
    // }

    // Generuj losową sieć przepływową
    size = 100;            // Liczba wierzchołków
    double density = 0.3;  // Gęstość grafu (%)
    int maxCapacity = 100; // Maksymalna przepustowość krawędzi
    capacityMatrix = generateFlowNetwork(size, density, maxCapacity);

    if (capacityMatrix.empty()) {
        std::cerr << "Error: Failed to create flow network." << std::endl;
        return 1;
    }
    
    //! Uncomment to print generated capacity matrix
    // std::cout << "Generated capacity matrix:" << std::endl;
    // printFlowMatrix(capacityMatrix);
    // std::cout << "\n";

    // Zdefiniuj źródło i ujście
    int source = 0;
    int sink = size - 1;


    // Uruchom algorytmy i zmierz czas

    // --- Ford-Fulkerson Algorithm ---
    FordFulkerson fordFulkerson(capacityMatrix);
    auto time_start_ff = std::chrono::high_resolution_clock::now();
    int maxFlow_ff = fordFulkerson.findMaxFlow(source, sink);
    auto time_stop_ff = std::chrono::high_resolution_clock::now();
    auto duration_ff_micro = std::chrono::duration_cast<std::chrono::microseconds>(time_stop_ff - time_start_ff);
    auto duration_ff_milli = std::chrono::duration_cast<std::chrono::milliseconds>(time_stop_ff - time_start_ff);

    // --- Edmonds-Karp Algorithm ---
    EdmondsKarp edmondsKarp(capacityMatrix);
    auto time_start_ek = std::chrono::high_resolution_clock::now();
    int maxFlow_ek = edmondsKarp.findMaxFlow(source, sink);
    auto time_stop_ek = std::chrono::high_resolution_clock::now();
    auto duration_ek_micro = std::chrono::duration_cast<std::chrono::microseconds>(time_stop_ek - time_start_ek);
    auto duration_ek_milli = std::chrono::duration_cast<std::chrono::milliseconds>(time_stop_ek - time_start_ek);


    // Wyświetl wyniki

    std::cout << "\n===== FORD-FULKERSON ALGORITHM RESULTS =====\n";
    std::cout << "Execution time: " << duration_ff_micro.count() << " microseconds -> " << duration_ff_milli.count() << " milliseconds.\n";
    std::cout << "Maximum flow from " << source << " to " << sink << ": " << maxFlow_ff << std::endl;
    //! Uncomment to print flow matrix
    // std::cout << "\nFlow matrix:" << std::endl;
    // printFlowMatrix(fordFulkerson.getFlowMatrix());

    std::cout << "\n===== EDMONDS-KARP ALGORITHM RESULTS =====\n";
    std::cout << "Execution time: " << duration_ek_micro.count() << " microseconds -> " << duration_ek_milli.count() << " milliseconds.\n";
    std::cout << "Maximum flow from " << source << " to " << sink << ": " << maxFlow_ek << std::endl;
    //! Uncomment to print flow matrix
    // std::cout << "\nFlow matrix:" << std::endl;
    // printFlowMatrix(edmondsKarp.getFlowMatrix());

    // Weryfikacja
    if (maxFlow_ff == maxFlow_ek) {
        std::cout << "\n✓ Both algorithms found the same maximum flow: " << maxFlow_ff << std::endl;
    } else {
        std::cout << "\n✗ Warning: Algorithms found different maximum flows!" << std::endl;
        std::cout << "Ford-Fulkerson: " << maxFlow_ff << std::endl;
        std::cout << "Edmonds-Karp: " << maxFlow_ek << std::endl;
    }

    return 0;
}
