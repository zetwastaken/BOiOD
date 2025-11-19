#include "FordFulkerson.h"
#include <algorithm>
#include <queue>

FordFulkerson::FordFulkerson(const std::vector<std::vector<int>>& capacityMatrix) 
    : capacity(capacityMatrix), numVertices(capacityMatrix.size()) {
    // Inicjalizacja macierzy przepływów zerami
    flow.assign(numVertices, std::vector<int>(numVertices, 0));
}

bool FordFulkerson::dfs(int source, int sink, std::vector<int>& parent, std::vector<bool>& visited) {
    if (source == sink) {
        return true;
    }
    
    visited[source] = true;
    
    for (int v = 0; v < numVertices; ++v) {
        // Sprawdź czy jest przepustowość residualna i wierzchołek nie był odwiedzony
        if (!visited[v] && capacity[source][v] - flow[source][v] > 0) {
            parent[v] = source;
            if (dfs(v, sink, parent, visited)) {
                return true;
            }
        }
    }
    
    return false;
}

int FordFulkerson::findMaxFlow(int source, int sink) {
    // Resetuj przepływy
    flow.assign(numVertices, std::vector<int>(numVertices, 0));
    
    int maxFlow = 0;
    std::vector<int> parent(numVertices);
    
    // Dopóki istnieje ścieżka powiększająca
    while (true) {
        std::vector<bool> visited(numVertices, false);
        parent.assign(numVertices, -1);
        
        // Szukaj ścieżki powiększającej używając DFS
        if (!dfs(source, sink, parent, visited)) {
            break;  // Nie ma więcej ścieżek powiększających
        }
        
        // Znajdź minimalną przepustowość residualną na ścieżce
        int pathFlow = INF_FLOW;
        for (int v = sink; v != source; v = parent[v]) {
            int u = parent[v];
            pathFlow = std::min(pathFlow, capacity[u][v] - flow[u][v]);
        }
        
        // Aktualizuj przepływy wzdłuż ścieżki
        for (int v = sink; v != source; v = parent[v]) {
            int u = parent[v];
            flow[u][v] += pathFlow;
            flow[v][u] -= pathFlow;
        }
        
        maxFlow += pathFlow;
    }
    
    return maxFlow;
}

std::vector<std::vector<int>> FordFulkerson::getFlowMatrix() const {
    return flow;
}

int FordFulkerson::getFlow(int from, int to) const {
    if (from < 0 || from >= numVertices || to < 0 || to >= numVertices) {
        return -1;
    }
    return flow[from][to];
}
