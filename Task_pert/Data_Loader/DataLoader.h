#ifndef DATALOADER_H
#define DATALOADER_H

#include "Task.h"

#include <map>
#include <string>

struct ProjectData
{
	int N = 0; // number of tasks
	int M = 0; // number of dependencies
	std::map<int, Task> tasks;
	bool success = false; // reading status
	int expectedProcessTime = 0;
	bool hasExpectedProcessTime = false;
};

class DataLoader
{
public:
	static ProjectData read_data(const std::string& filename);
};
#endif // !DATALOADER_H
