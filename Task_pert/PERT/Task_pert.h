#ifndef TASK_PERT_H
#define TASK_PERT_H

#include <vector>
#include <cmath>

class Task_pert
{
public:
	int id; 
	
	int optimistic_time = 0;
	int most_likely_time = 0;
	int pessimistic_time = 0;

	double expected_duration = 0.0;
	double variance = 0.0;

	double ES = 0; // Early Start
	double EF = 0; // Early Finish
	double LS = 0; // Late Start
	double LF = 0; // Late Finish
	double slack = 0;

	std::vector<int> predecessors; // IDs of tasks before this task
	std::vector<int> successors;   // IDs of tasks after this task

	Task_pert() = default;

	Task_pert(int taskID, int optimistic, int most_likley, int pessimistic)
		: id(taskID), optimistic_time(optimistic), most_likely_time(most_likley), pessimistic_time(pessimistic)
	{
		expected_duration = static_cast<double>(optimistic_time + 4 * most_likely_time + pessimistic_time) / 6.0;

		variance = std::pow(static_cast<double>(pessimistic_time - optimistic_time) / 6.0, 2);
	}

};

#endif // !TASK_PERT_H
