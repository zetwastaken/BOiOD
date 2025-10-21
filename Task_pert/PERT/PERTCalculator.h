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

class PERTCalculator
{
public:
    static PERTResult analyze(std::map<int, Task_pert>& tasks);
};

#endif // PERT_CALCULATOR_H
