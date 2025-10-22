#ifndef PERT_CALCULATOR_H
#define PERT_CALCULATOR_H

#include <map>
#include <vector>

#include "Task_pert.h"

struct PERTResult
{
    double expectedDuration = 0.0;
    double variance = 0.0;
    double standardDeviation = 0.0;
    std::vector<int> criticalPath;
};

struct PERTSimulation
{
    int simulations = 0;
    double meanDuration = 0.0;
    double minDuration = 0.0;
    double maxDuration = 0.0;
    double standardDeviation = 0.0;
    std::vector<double> completionTimes; // All simulation results
    
    // Percentile calculations
    double getPercentile(double percentile) const;
};

class PERTCalculator
{
public:
    static PERTResult analyze(std::map<int, Task_pert>& tasks);
    static PERTSimulation analyzeSimulation(std::map<int, Task_pert>& tasks, int numSimulations);
};

#endif // PERT_CALCULATOR_H
