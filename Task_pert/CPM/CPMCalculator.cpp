#include "CPMCalculator.h"

#include <algorithm>
#include <limits>

namespace
{
void initializeStartTimes(std::map<int, Task>& tasks,
                          std::map<int, int>& inDegree,
                          std::vector<int>& topoOrder)
{
    topoOrder.clear();
    inDegree.clear();

    for (auto& [id, task] : tasks)
    {
        inDegree[id] = static_cast<int>(task.predecessors.size());
        task.ES = 0;
        task.EF = 0;
        task.LS = 0;
        task.LF = 0;
        task.slack = 0;

        if (inDegree[id] == 0)
        {
            task.ES = 0;
            task.EF = task.duration;
            topoOrder.push_back(id);
        }
    }
}
}

CPMResult CPMCalculator::analyze(std::map<int, Task>& tasks)
{
    CPMResult result;
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
        Task& currentTask = tasks.at(currentId);
        currentTask.EF = currentTask.ES + currentTask.duration;

        for (int successorId : currentTask.successors)
        {
            Task& successor = tasks.at(successorId);
            successor.ES = std::max(successor.ES, currentTask.EF);

            auto it = inDegree.find(successorId);
            if (it != inDegree.end() && --(it->second) == 0)
            {
                topoOrder.push_back(successorId);
            }
        }
    }

    // Determine total project duration.
    for (const auto& [id, task] : tasks)
    {
        if (task.successors.empty())
        {
            result.totalDuration = std::max(result.totalDuration, task.EF);
        }
    }

    // Backward pass.
    for (auto& [id, task] : tasks)
    {
        if (task.successors.empty())
        {
            task.LF = result.totalDuration;
        }
    }

    for (int i = static_cast<int>(topoOrder.size()) - 1; i >= 0; --i)
    {
        const int currentId = topoOrder[i];
        Task& currentTask = tasks.at(currentId);

        if (!currentTask.successors.empty())
        {
            int minLateStart = result.totalDuration;
            bool first = true;
            for (int successorId : currentTask.successors)
            {
                const Task& successor = tasks.at(successorId);
                if (first || successor.LS < minLateStart)
                {
                    minLateStart = successor.LS;
                    first = false;
                }
            }
            currentTask.LF = minLateStart;
        }

        currentTask.LS = currentTask.LF - currentTask.duration;
        currentTask.slack = currentTask.LF - currentTask.EF;
    }

    std::vector<int> criticalCandidates;
    criticalCandidates.reserve(tasks.size());
    for (const auto& [id, task] : tasks)
    {
        if (task.slack == 0)
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
        for (std::size_t idx = 1; idx < criticalCandidates.size(); ++idx)
        {
            const int currentId = criticalCandidates[idx];
            const int previousId = result.criticalPath.back();
            const Task& previousTask = tasks.at(previousId);
            const auto successorIt = std::find(previousTask.successors.begin(), previousTask.successors.end(), currentId);
            if (successorIt != previousTask.successors.end())
            {
                result.criticalPath.push_back(currentId);
            }
        }
    }

    return result;
}

CPMResult CPMCalculator::analyzeBellmanFord(std::map<int, Task>& tasks)
{
    CPMResult result;
    if (tasks.empty())
    {
        return result;
    }

    constexpr int negativeInfinity = std::numeric_limits<int>::min() / 4;

    for (auto& [id, task] : tasks)
    {
        if (task.predecessors.empty())
        {
            task.ES = 0;
            task.EF = task.duration;
        }
        else
        {
            task.ES = negativeInfinity;
            task.EF = negativeInfinity;
        }
        task.LS = 0;
        task.LF = 0;
        task.slack = 0;
    }

    const int vertices = static_cast<int>(tasks.size());

    for (int iteration = 0; iteration < vertices - 1; ++iteration)
    {
        bool updated = false;
        for (auto& [id, task] : tasks)
        {
            if (task.ES == negativeInfinity)
            {
                continue;
            }

            const int finish = task.ES + task.duration;
            for (int successorId : task.successors)
            {
                Task& successor = tasks.at(successorId);
                if (finish > successor.ES)
                {
                    successor.ES = finish;
                    successor.EF = successor.ES + successor.duration;
                    updated = true;
                }
            }
        }

        if (!updated)
        {
            break;
        }
    }

    for (auto& [id, task] : tasks)
    {
        if (task.ES == negativeInfinity)
        {
            task.ES = 0;
        }
        task.EF = task.ES + task.duration;
    }

    for (const auto& [id, task] : tasks)
    {
        if (task.EF > result.totalDuration)
        {
            result.totalDuration = task.EF;
        }
    }

    for (auto& [id, task] : tasks)
    {
        task.LF = result.totalDuration;
        task.LS = task.LF - task.duration;
    }

    for (int iteration = 0; iteration < vertices - 1; ++iteration)
    {
        bool updated = false;
        for (auto& [id, task] : tasks)
        {
            for (int successorId : task.successors)
            {
                Task& successor = tasks.at(successorId);
                const int candidateLF = successor.LS;
                if (candidateLF < task.LF)
                {
                    task.LF = candidateLF;
                    task.LS = task.LF - task.duration;
                    updated = true;
                }
            }
        }

        if (!updated)
        {
            break;
        }
    }

    std::vector<int> criticalCandidates;
    criticalCandidates.reserve(tasks.size());
    for (auto& [id, task] : tasks)
    {
        task.slack = task.LF - task.EF;
        if (task.slack == 0)
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
        for (std::size_t idx = 1; idx < criticalCandidates.size(); ++idx)
        {
            const int currentId = criticalCandidates[idx];
            const int previousId = result.criticalPath.back();
            const Task& previousTask = tasks.at(previousId);
            const auto successorIt = std::find(previousTask.successors.begin(),
                                               previousTask.successors.end(),
                                               currentId);
            if (successorIt != previousTask.successors.end())
            {
                result.criticalPath.push_back(currentId);
            }
        }
    }

    return result;
}