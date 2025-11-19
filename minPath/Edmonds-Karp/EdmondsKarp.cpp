#include "EdmondsKarp.h"
#include <algorithm>
#include <queue>

EdmondsKarp::EdmondsKarp(const std::vector<std::vector<int>>& capacityMatrix) 
    : capacity(capacityMatrix), numVertices(capacityMatrix.size()) {
    // Inicjalizacja macierzy przepływów zerami
    flow.assign(numVertices, std::vector<int>(numVertices, 0));
}

bool EdmondsKarp::bfs(int source, int sink, std::vector<int>& parent) {
    std::vector<bool> visited(numVertices, false);
    std::queue<int> queue;
    
    queue.push(source);
    visited[source] = true;
    parent[source] = -1;
    
    while (!queue.empty()) {
        int u = queue.front();
        queue.pop();
        
        for (int v = 0; v < numVertices; ++v) {
            // Sprawdź czy wierzchołek nie był odwiedzony i jest przepustowość residualna
            if (!visited[v] && capacity[u][v] - flow[u][v] > 0) {
                parent[v] = u;
                visited[v] = true;
                
                if (v == sink) {
                    return true;  // Znaleziono ścieżkę do ujścia
                }
                
                queue.push(v);
            }
        }
    }
    
    return false;  // Nie znaleziono ścieżki
}

int EdmondsKarp::findMaxFlow(int source, int sink) {
    // Resetuj przepływy
    flow.assign(numVertices, std::vector<int>(numVertices, 0));
    
    int maxFlow = 0;
    std::vector<int> parent(numVertices);
    
    // Dopóki istnieje ścieżka powiększająca (znajdowana przez BFS)
    while (bfs(source, sink, parent)) {
        // Znajdź minimalną przepustowość residualną na ścieżce
        int pathFlow = INF_EK;
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

std::vector<std::vector<int>> EdmondsKarp::getFlowMatrix() const {
    return flow;
}

int EdmondsKarp::getFlow(int from, int to) const {
    if (from < 0 || from >= numVertices || to < 0 || to >= numVertices) {
        return -1;
    }
    return flow[from][to];
}
