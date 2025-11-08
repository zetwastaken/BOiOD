#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include <vector>
#include <string>

class Dijkstra
{
public:
    //* Constructor takes a graph as a adjacency matrix
    Dijkstra(const std::vector<std::vector<int>>& graph);
    //* function to find shortest paths from given vertex
    void findShortestPaths(int startVertex);
    //* Function returns the length of the shortest path to destination vertex
    int getShortestDistanceTo(int destinationVertex) const;
    //* returns the shortest path as a sequence
    std::vector<int> getShortestPathTo(int destinationVertex)const;

private:
    //* find the closest known, not visited vertex
    int findMinDistanceVertex(const std::vector<bool>& visited) const;

    std::vector<std::vector<int>> graph;

    int numVertices;

    //* Result vectors
    std::vector<int> distances; //* shortest paths from startVertex
    std::vector<int> parents; //* predecesors on the shortest path
};

#endif //! DIJKSTRA_H