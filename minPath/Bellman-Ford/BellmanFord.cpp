#include "BellmanFord.h"
#include "../dataGenerator/dataGenerator.h"
#include <algorithm>

BellmanFord::BellmanFord(const std::vector<std::vector<int>> &graph)
    : graph(graph), numVertices(graph.size()), negativeCycleDetected(false) {}

bool BellmanFord::findShortestPaths(int startVertex)
{
    distances.assign(numVertices, INF);
    parents.assign(numVertices, -1);
    negativeCycleDetected = false;

    distances[startVertex] = 0;

    //* Edge relaxation
    for (int i = 0; i < numVertices -1; ++i)
    {
        for (int u = 0; u < numVertices; ++u)
        {
            for (int v = 0; v < numVertices; ++v)
            {
                int weight  = graph[u][v];
                if (weight != INF)
                {
                    if (distances[u] != INF && distances[u] + weight < distances[v])
                    {
                        distances[v] = distances[u] + weight;
                        parents[v] = u;
                    }
                }
            }
        }
    }

    //* Checking if negative cycle exists
    for (int u = 0; u < numVertices; ++u)
    {
        for (int v = 0; v < numVertices; ++v)
        {
            int weight = graph[u][v];
            if (weight != INF)
            {
                if (distances[u] != INF && distances[u] + weight < distances[v])
                    {
                        negativeCycleDetected = true;
                        return false;
                    }
            }
        }
    }

    return true;
}

int BellmanFord::getShortestDistanceTo(int destinationVertex) const
{
    if (destinationVertex < 0 || destinationVertex >= numVertices) return-1;

    int distance = distances[destinationVertex];
    return (distance == INF) ? -1 : distance;
}

std::vector<int> BellmanFord::getShortestPathTo(int destinationVertex) const
{
    if (destinationVertex < 0 || destinationVertex >= numVertices || distances[destinationVertex] == INF || negativeCycleDetected) return {};

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

bool BellmanFord::hasNegativeCycle() const
{
    return negativeCycleDetected;
}
