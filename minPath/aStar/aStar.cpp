#include "aStar.h"
#include "../dataGenerator/dataGenerator.h"

#include <cmath>
#include <vector>
#include <queue>
#include <algorithm>

AStar::AStar(const std::vector<std::vector<int>> &graph, const std::vector<Point> &coordinates)
    : graph(graph), coordinates(coordinates), numVertices(graph.size()), distance(-1) {}

bool AStar::findShortestPath(int startVertex, int endVertex)
{
    using FScoreVertex = std::pair<int, int>; // {fScore, vertex}

    std::priority_queue<FScoreVertex, std::vector<FScoreVertex>, std::greater<FScoreVertex>> openSet;
    std::vector<int> g_scores(numVertices, INF);
    std::vector<int> parents(numVertices, -1);

    g_scores[startVertex] = 0;
    int start_f_score = heuristic(startVertex, endVertex);
    openSet.push({start_f_score, startVertex});

    while (!openSet.empty())
    {
        int current = openSet.top().second;
        openSet.pop();

        if (current == endVertex)
        {
            distance = g_scores[endVertex];
            path.clear();
            int node = endVertex;
            while(node != -1)
            {
                path.push_back(node);
                node = parents[node];
            }
            std::reverse(path.begin(), path.end());
            return true;
        }

        for (int neighbor = 0; neighbor < numVertices; ++neighbor)
        {
            int weight = graph[current][neighbor];
            if (weight == INF) continue;

            int new_g_score = g_scores[current] + weight;
            if (new_g_score < g_scores[neighbor])
            {
                parents[neighbor] = current;
                g_scores[neighbor] = new_g_score;
                int f_score = new_g_score + heuristic(neighbor, endVertex);
                openSet.push({f_score, neighbor});
            }
        }
    }

    distance = -1;
    path.clear();
    return false; // No path found
}

int AStar::getShortestDistance() const
{
    return distance;
}

std::vector<int> AStar::getShortestPath() const
{
    return path;
}

int AStar::heuristic(int from, int to) const
{
    int dx = coordinates[from].x - coordinates[to].x;
    int dy = coordinates[from].y - coordinates[to].y;
    return static_cast<int>(std::sqrt(dx * dx + dy * dy));
}

