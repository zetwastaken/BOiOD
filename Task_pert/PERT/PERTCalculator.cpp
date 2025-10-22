#include "PERTCalculator.h"
#include "../CPM/CPMCalculator.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>
#include <random>
#include <numeric>

namespace
{
constexpr double kSlackTolerance = 1e-6;

void initializeStartTimes(std::map<int, Task_pert>& tasks,
                          std::map<int, int>& inDegree,
                          std::vector<int>& topoOrder)
{
    topoOrder.clear();
    inDegree.clear();

    for (auto& [id, task] : tasks)
    {
        inDegree[id] = static_cast<int>(task.predecessors.size());
        task.ES = 0.0;
        task.EF = 0.0;
        task.LS = 0.0;
        task.LF = 0.0;
        task.slack = 0.0;

        if (inDegree[id] == 0)
        {
            task.ES = 0.0;
            task.EF = task.expected_duration;
            topoOrder.push_back(id);
        }
    }
}
}

PERTResult PERTCalculator::analyze(std::map<int, Task_pert>& tasks)
{
    PERTResult result;
    if (tasks.empty())
    {
        return result;
    }

    std::map<int, int> inDegree;
    std::vector<int> topoOrder;
    initializeStartTimes(tasks, inDegree, topoOrder);

    // Forward pass.
    std::size_t head = 0;
    while (head < topoOrder.size())
    {
        const int currentId = topoOrder[head++];
        Task_pert& currentTask = tasks.at(currentId);
        currentTask.EF = currentTask.ES + currentTask.expected_duration;

        for (int successorId : currentTask.successors)
        {
            Task_pert& successor = tasks.at(successorId);
            successor.ES = std::max(successor.ES, currentTask.EF);

            auto it = inDegree.find(successorId);
            if (it != inDegree.end() && --(it->second) == 0)
            {
                topoOrder.push_back(successorId);
            }
        }
    }

    // Determine expected project duration (mu).
    for (const auto& [id, task] : tasks)
    {
        if (task.successors.empty())
        {
            result.expectedDuration = std::max(result.expectedDuration, task.EF);
        }
    }

    // Backward pass.
    for (auto& [id, task] : tasks)
    {
        if (task.successors.empty())
        {
            task.LF = result.expectedDuration;
        }
    }

    for (int i = static_cast<int>(topoOrder.size()) - 1; i >= 0; --i)
    {
        const int currentId = topoOrder[i];
        Task_pert& currentTask = tasks.at(currentId);

        if (!currentTask.successors.empty())
        {
            double minLateStart = result.expectedDuration;
            bool first = true;
            for (int successorId : currentTask.successors)
            {
                const Task_pert& successor = tasks.at(successorId);
                if (first || successor.LS < minLateStart)
                {
                    minLateStart = successor.LS;
                    first = false;
                }
            }
            currentTask.LF = minLateStart;
        }

        currentTask.LS = currentTask.LF - currentTask.expected_duration;
        currentTask.slack = currentTask.LF - currentTask.EF;
    }

    std::vector<int> criticalCandidates;
    criticalCandidates.reserve(tasks.size());
    for (const auto& [id, task] : tasks)
    {
        if (std::abs(task.slack) < kSlackTolerance)
        {
            criticalCandidates.push_back(id);
        }
    }

    std::sort(criticalCandidates.begin(), criticalCandidates.end(),
              [&tasks](int lhs, int rhs)
              {
                  return tasks.at(lhs).ES < tasks.at(rhs).ES;
              });

    if (!criticalCandidates.empty())
    {
        result.criticalPath.push_back(criticalCandidates.front());
        result.variance += tasks.at(criticalCandidates.front()).variance;

        for (std::size_t idx = 1; idx < criticalCandidates.size(); ++idx)
        {
            const int currentId = criticalCandidates[idx];
            const int previousId = result.criticalPath.back();
            const Task_pert& previousTask = tasks.at(previousId);
            const auto successorIt = std::find(previousTask.successors.begin(), previousTask.successors.end(), currentId);
            if (successorIt != previousTask.successors.end())
            {
                result.criticalPath.push_back(currentId);
                result.variance += tasks.at(currentId).variance;
            }
        }
    }

    result.standardDeviation = std::sqrt(result.variance);
    return result;
}

double PERTSimulation::getPercentile(double percentile) const
{
    if (completionTimes.empty() || percentile < 0.0 || percentile > 100.0)
    {
        return 0.0;
    }

    std::vector<double> sortedTimes = completionTimes;
    std::sort(sortedTimes.begin(), sortedTimes.end());

    const double index = (percentile / 100.0) * (sortedTimes.size() - 1);
    const std::size_t lowerIndex = static_cast<std::size_t>(std::floor(index));
    const std::size_t upperIndex = static_cast<std::size_t>(std::ceil(index));

    if (lowerIndex == upperIndex)
    {
        return sortedTimes[lowerIndex];
    }

    const double weight = index - lowerIndex;
    return sortedTimes[lowerIndex] * (1.0 - weight) + sortedTimes[upperIndex] * weight;
}

PERTSimulation PERTCalculator::analyzeSimulation(std::map<int, Task_pert>& tasks, int numSimulations)
{
    PERTSimulation result;
    if (tasks.empty() || numSimulations <= 0)
    {
        return result;
    }

    result.simulations = numSimulations;
    result.completionTimes.reserve(numSimulations);

    // Setup random number generation
    std::random_device rd;
    std::mt19937 gen(rd());

    // Run simulations
    for (int sim = 0; sim < numSimulations; ++sim)
    {
        // Create a CPM task map with randomized durations
        std::map<int, Task> cpmTasks;
        
        for (const auto& [id, pertTask] : tasks)
        {
            // Generate random duration using Beta distribution (PERT-style)
            // Beta distribution parameters for PERT: alpha=4, beta=4
            // This gives a bell-shaped distribution centered around most_likely_time
            
            const double a = static_cast<double>(pertTask.optimistic_time);
            const double b = static_cast<double>(pertTask.pessimistic_time);
                        
            // Use uniform distribution between optimistic and pessimistic times
            std::uniform_real_distribution<double> uniformDist(a, b);
            double randomDuration = uniformDist(gen);
            
            // Create CPM task with random duration (rounded to integer)
            Task cpmTask(id, static_cast<int>(std::round(randomDuration)));
            cpmTask.predecessors = pertTask.predecessors;
            cpmTask.successors = pertTask.successors;
            
            cpmTasks[id] = cpmTask;
        }
        
        // Run CPM analysis on the randomized tasks
        CPMResult cpmResult = CPMCalculator::analyze(cpmTasks);
        
        // Store the completion time
        const double completionTime = static_cast<double>(cpmResult.totalDuration);
        result.completionTimes.push_back(completionTime);
        
    }

    // Calculate statistics
    result.minDuration = *std::min_element(result.completionTimes.begin(), result.completionTimes.end());
    result.maxDuration = *std::max_element(result.completionTimes.begin(), result.completionTimes.end());
    
    const double sum = std::accumulate(result.completionTimes.begin(), result.completionTimes.end(), 0.0);
    result.meanDuration = sum / numSimulations;

    // Calculate standard deviation
    double sumSquaredDiff = 0.0;
    for (double time : result.completionTimes)
    {
        const double diff = time - result.meanDuration;
        sumSquaredDiff += diff * diff;
    }
    result.standardDeviation = std::sqrt(sumSquaredDiff / numSimulations);

    return result;
}