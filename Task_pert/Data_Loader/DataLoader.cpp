#include "DataLoader.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <string>

namespace
{
void strip_bom(std::string& line)
{
    if (line.size() >= 3 && static_cast<unsigned char>(line[0]) == 0xEF &&
        static_cast<unsigned char>(line[1]) == 0xBB &&
        static_cast<unsigned char>(line[2]) == 0xBF)
    {
        line.erase(0, 3);
    }
}

std::string trim(const std::string& value)
{
    const auto first = value.find_first_not_of("\t\r\n ");
    if (first == std::string::npos)
    {
        return {};
    }
    const auto last = value.find_last_not_of("\t\r\n ");
    return value.substr(first, last - first + 1);
}

bool is_metadata_line(const std::string& line)
{
    return line.find("in:") != std::string::npos ||
           line.find("out:") != std::string::npos ||
           line.find("process time:") != std::string::npos ||
           line.find("earlyStart") != std::string::npos ||
           line.find("critical path:") != std::string::npos;
}
}

ProjectData DataLoader::read_data(const std::string& filename)
{
	ProjectData data;

	std::ifstream file(filename);
	if (!file.is_open())
	{
		std::cerr << "Error: Could not open file: " << filename << std::endl;
		data.success = false;
		return data;
	}

	std::string line;
	int data_line_index = 0;

	while (std::getline(file, line))
	{
		strip_bom(line);
		line = trim(line);

	if (line.rfind("process time", 0) == 0 || line.rfind("Process time", 0) == 0)
		{
			std::string valueLine;
			while (std::getline(file, valueLine))
			{
				strip_bom(valueLine);
				valueLine = trim(valueLine);
				if (valueLine.empty())
				{
					continue;
				}

				std::stringstream valueStream(valueLine);
				int expected = 0;
				if (valueStream >> expected)
				{
					data.expectedProcessTime = expected;
					data.hasExpectedProcessTime = true;
				}
				else
				{
					std::cerr << "Warning: Failed to parse expected process time from file." << std::endl;
				}
				break;
			}
			continue;
		}

		if (line.empty() || is_metadata_line(line)) 
		{
			continue; 
		}

		data_line_index++;
		std::stringstream ss(line);

		if (data_line_index == 1)
		{
			// fisrt line: N and M
			if (!(ss >> data.N >> data.M))
			{
				std::cerr << "Error reading N and M from file." << std::endl;
				data.success = false;
				return data;
			}
		}
		else if (data_line_index == 2)
		{
			// second line: task durations
			int duration;
			for (int i = 1; i <= data.N; ++i)
			{
				if (!(ss >> duration))
				{
					std::cerr << "Error reading task durations from file. (Task: " << i << ")" << std::endl;
					data.success = false;
					return data;
				}
				data.tasks.emplace(i, Task(i, duration));
			}
		}
		else if (data_line_index == 3)
		{
			// third line: dependencies
			int predID, succID;
			while (ss >> predID >> succID)
			{
				if (data.tasks.count(predID) && data.tasks.count(succID))
				{
					data.tasks.at(predID).successors.push_back(succID);
					data.tasks.at(succID).predecessors.push_back(predID);
				}
			}
		}
	}

	if (data.tasks.empty() || data.N == 0)
	{
		std::cerr << "Error: No Tasks found in file." << std::endl;
		data.success = false;
		return data;
	}

	data.success = true;
	return data;	
}
