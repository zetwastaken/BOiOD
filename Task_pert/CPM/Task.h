#ifndef CPM_TASK_H
#define CPM_TASK_H

#include <vector>

class Task
{
public:
    int id;
    int duration;

    int ES = 0; // Early Start
    int EF = 0; // Early Finish
    int LS = 0; // Late Start
    int LF = 0; // Late Finish
    int slack = 0;

    std::vector<int> predecessors; // IDs of tasks before this task
    std::vector<int> successors;   // IDs of tasks after this task

    Task() = default;

    Task(int taskID, int taskDuration)
        : id(taskID), duration(taskDuration)
    {
    }
};

#endif // CPM_TASK_H
