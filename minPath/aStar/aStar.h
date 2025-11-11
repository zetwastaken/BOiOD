#ifndef ASTAR_H
#define ASTAR_H

#include <vector>
#include <string>

struct Point
{
    int x, y;
};

class AStar
{
public:
    AStar(const std::vector<std::vector<int>>& graph, const std::vector<Point>& coordinates);

    //* finds shortest path from 'startVertex' to 'endVertex'
    bool findShortestPath(int startVertex, int endVertex);
    //* return final distance
    int getShortestDistance() const;
    //* returns final-shortest path
    std::vector<int> getShortestPath() const;

private:
    int heuristic(int from, int to) const;

    std::vector<std::vector<int>> graph;
    std::vector<Point> coordinates;
    int numVertices;

    //* Results
    std::vector<int> path;
    int distance;
};

#endif //! ASTAR_H