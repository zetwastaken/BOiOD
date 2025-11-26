#ifndef NEARESTNEIGHBOR_H
#define NEARESTNEIGHBOR_H

#include <vector>

using namespace std;

class NearestNeighbor {
private:
    vector<vector<int>> distanceMatrix;
    int numCities;
    
public:
    // Constructor
    NearestNeighbor(const vector<vector<int>>& matrix);
    
    // Solve TSP using Nearest Neighbor algorithm starting from a given city
    vector<int> solve(int startCity = 0);
    
    // Calculate total distance of a given tour
    int calculateTourDistance(const vector<int>& tour) const;
    
    // Get tour with distance (returns pair of tour and its total distance)
    pair<vector<int>, int> getTour(int startCity = 0);
    
    // Get the number of cities
    int getNumCities() const;
};

#endif //! NEARESTNEIGHBOR_H
