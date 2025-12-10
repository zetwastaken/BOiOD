#ifndef EDMONDS_KARP_H
#define EDMONDS_KARP_H

#include <vector>
#include <limits>

const int INF_EK = std::numeric_limits<int>::max();

class EdmondsKarp {
private:
    std::vector<std::vector<int>> capacity;  // Macierz przepustowości
    std::vector<std::vector<int>> flow;      // Macierz przepływów
    int numVertices;
    
    // Pomocnicza funkcja BFS do znajdowania ścieżki powiększającej
    bool bfs(int source, int sink, std::vector<int>& parent);
    
public:
    EdmondsKarp(const std::vector<std::vector<int>>& capacityMatrix);
    
    // Oblicza maksymalny przepływ od source do sink
    int findMaxFlow(int source, int sink);
    
    // Zwraca macierz przepływów
    std::vector<std::vector<int>> getFlowMatrix() const;
    
    // Zwraca wartość przepływu przez krawędź
    int getFlow(int from, int to) const;
};

#endif // EDMONDS_KARP_H
