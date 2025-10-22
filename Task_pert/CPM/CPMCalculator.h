#ifndef CPM_CALCULATOR_H
#define CPM_CALCULATOR_H

#include <map>
#include <vector>

#include "Task.h"

struct CPMResult
{
    int totalDuration = 0;
    std::vector<int> criticalPath;
};

class CPMCalculator
{
public:
    static CPMResult analyze(std::map<int, Task>& tasks);
    static CPMResult analyzeBellmanFord(std::map<int, Task>& tasks);
};

#endif // CPM_CALCULATOR_H
