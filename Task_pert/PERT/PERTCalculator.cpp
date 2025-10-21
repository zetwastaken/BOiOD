#include "PERTCalculator.h"

#include <algorithm>
#include <cmath>

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
