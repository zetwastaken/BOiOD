#include "dijkstra_algorithm.h"
#include "dataGenerator/dataGenerator.h"

#include <limits>
#include <algorithm>

Dijkstra::Dijkstra(const std::vector<std::vector<int>> &graph) 
    : graph(graph), numVertices(graph.size()) {}

void Dijkstra::findShortestPaths(int startVertex)
{
    distances.assign(numVertices, INF); //? Distances set to infinity
    parents.assign(numVertices, -1); //? No defined parents
    std::vector<bool> visited(numVertices, false); //? No vertices visited yet

    distances[startVertex] = 0;

    for (int count = 0; count < numVertices - 1; ++count)
    {
        //* Find vertex with minimal distance and not yet visited
        int vertex = findMinDistanceVertex(visited);

        if (vertex == -1) break;
        visited[vertex] = true;

        for (int v = 0; v < numVertices; ++v)
        {
            if(!visited[v] && graph[vertex][v] != INF && distances[vertex] != INF && distances[vertex] + graph[vertex][v] < distances[v])
            {
                distances[v] = distances[vertex] + graph[vertex][v];
                parents[v] = vertex;
            }
        }
    }
}

int Dijkstra::getShortestDistanceTo(int destinationVertex) const
{
    if (destinationVertex < 0 || destinationVertex >= numVertices) return -1; // Invalid index

    int distance = distances[destinationVertex];
    return (distance == INF) ? -1 : distance; // Return -1 if vertex is not reachable
}

std::vector<int> Dijkstra::getShortestPathTo(int destinationVertex) const
{
    if (destinationVertex < 0 || destinationVertex >= numVertices || distances[destinationVertex] == INF) return {};

    std::vector<int> path;
    int currentNode = destinationVertex;

    while (currentNode != -1)
    {
        path.push_back(currentNode);
        currentNode = parents[currentNode];
    }

    std::reverse(path.begin(), path.end());
    return path;
    
}

int Dijkstra::findMinDistanceVertex(const std::vector<bool> &visited) const
{
    int minDistance = INF;
    int minIndex = -1;

    for (int v = 0; v < numVertices; ++v)
    {
    if (!visited[v] && distances[v] < minDistance) 
    {
        minDistance = distances[v];
        minIndex = v;
    }
    }
    return minIndex;
}
