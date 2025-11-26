#include "nearestNeighbor.h"
#include <limits>
#include <algorithm>

// Constructor
NearestNeighbor::NearestNeighbor(const vector<vector<int>>& matrix) 
    : distanceMatrix(matrix), numCities(matrix.size()) {}

// Solve TSP using Nearest Neighbor algorithm
vector<int> NearestNeighbor::solve(int startCity) {
    vector<int> tour;
    vector<bool> visited(numCities, false);
    
    int currentCity = startCity;
    tour.push_back(currentCity);
    visited[currentCity] = true;
    
    // Visit all cities
    for (int i = 1; i < numCities; i++) {
        int nearestCity = -1;
        int minDistance = numeric_limits<int>::max();
        
        // Find the nearest unvisited city
        for (int j = 0; j < numCities; j++) {
            if (!visited[j] && distanceMatrix[currentCity][j] < minDistance) {
                minDistance = distanceMatrix[currentCity][j];
                nearestCity = j;
            }
        }
        
        // Move to the nearest city
        currentCity = nearestCity;
        tour.push_back(currentCity);
        visited[currentCity] = true;
    }
    
    // Return to the starting city to complete the tour
    tour.push_back(startCity);
    
    return tour;
}

// Calculate total distance of a given tour
int NearestNeighbor::calculateTourDistance(const vector<int>& tour) const {
    int totalDistance = 0;
    
    for (size_t i = 0; i < tour.size() - 1; i++) {
        totalDistance += distanceMatrix[tour[i]][tour[i + 1]];
    }
    
    return totalDistance;
}

// Get tour with distance
pair<vector<int>, int> NearestNeighbor::getTour(int startCity) {
    vector<int> tour = solve(startCity);
    int distance = calculateTourDistance(tour);
    return make_pair(tour, distance);
}

// Get the number of cities
int NearestNeighbor::getNumCities() const {
    return numCities;
}
