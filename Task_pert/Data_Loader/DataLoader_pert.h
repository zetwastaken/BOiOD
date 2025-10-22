#ifndef DATALOADER_PERT_H
#define DATALOADER_PERT_H

#include "Task_pert.h"

#include <map>
#include <string>

struct ProjectDataPert
{
	int N = 0; // number of tasks
	int M = 0; // number of dependencies
	std::map<int, Task_pert> tasks;
	bool success = false; // reading status

	double target_time = 0.0; // target project completion time
	double target_probability = 0.0; // target probability of completion
};

class DataLoader_pert
{
public:
	static ProjectDataPert read_data(const std::string& filename);
};
#endif // !DATALOADER_PERT_H
