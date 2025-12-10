#ifndef FORD_FULKERSON_H
#define FORD_FULKERSON_H

#include <vector>
#include <limits>

const int INF_FLOW = std::numeric_limits<int>::max();

class FordFulkerson {
private:
    std::vector<std::vector<int>> capacity;  // Macierz przepustowości
    std::vector<std::vector<int>> flow;      // Macierz przepływów
    int numVertices;
    
    // Pomocnicza funkcja DFS do znajdowania ścieżki powiększającej
    bool dfs(int source, int sink, std::vector<int>& parent, std::vector<bool>& visited);
    
public:
    FordFulkerson(const std::vector<std::vector<int>>& capacityMatrix);
    
    // Oblicza maksymalny przepływ od source do sink
    int findMaxFlow(int source, int sink);
    
    // Zwraca macierz przepływów
    std::vector<std::vector<int>> getFlowMatrix() const;
    
    // Zwraca wartość przepływu przez krawędź
    int getFlow(int from, int to) const;
};

#endif // FORD_FULKERSON_H
