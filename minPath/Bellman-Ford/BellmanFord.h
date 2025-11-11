#ifndef BELLMANFORD_H
#define BELLMANFORD_H

#include <vector>
#include <string>

class BellmanFord
{
public:
    //* Constructor
    BellmanFord(const std::vector<std::vector<int>>& graph);

    //* 'true' if paths were find, 'false' if negative cycle detected
    bool findShortestPaths(int startVertex);
    //* return length of the shortest path
    int getShortestDistanceTo(int destinationVertex) const;

    //* returns the shortest path as a vector
    std::vector<int> getShortestPathTo(int destinationVertex) const;

    //* returns true if negative cycle is detected
    bool hasNegativeCycle() const;

private:
    std::vector<std::vector<int>> graph;
    int numVertices;
    std::vector<int> distances;
    std::vector<int> parents;
    bool negativeCycleDetected;
};


#endif //! BELLMANFORD_H