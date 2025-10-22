#include "DataLoader_pert.h"

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
    const auto first = value.find_first_not_of(" \t\n\r");
    if (first == std::string::npos)
    {
        return {};
    }
    const auto last = value.find_last_not_of(" \t\n\r");
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

ProjectDataPert DataLoader_pert::read_data(const std::string& filename)
{
	ProjectDataPert data;

    std::ifstream file(filename);
    if (!file.is_open()) 
    {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        data.success = false;
        return data;
    }

    std::string line;
    int data_line_index = 0;

    while (std::getline(file, line))
    {
        strip_bom(line);
        line = trim(line);

        if (line.empty() || is_metadata_line(line))
        {
            continue;
        }

        data_line_index++;
        std::stringstream ss(line);

        if (data_line_index == 1)
        {
            // Linia 1: N i M
            if (!(ss >> data.N >> data.M)) 
            {
                std::cerr << "Error reading N and M from file." << std::endl;
                data.success = false;
                return data;
            }
        }
        else if (data_line_index == 2) // optimistic, most likely, pessimistic times
        {
            int opt, likely, pess;
            for (int i = 1; i <= data.N; ++i)
            {
                if (!(ss >> opt >> likely >> pess))
                {
                    std::cerr << "Error reading task times." << std::endl;
                    data.success = false;
                    return data;
                }
                data.tasks.emplace(i, Task_pert(i, opt, likely, pess));
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
        else if (data_line_index == 4)
        {
            // target time and target probability
            if (!(ss >> data.target_time >> data.target_probability))
            {
                std::cerr << "Error reading target time and probability." << std::endl;
                data.success = false;
                return data;
            }
            data.target_probability /= 100.0; // convert percentage to decimal
        }
    }

    if (!data.tasks.empty() && data.N > 0 && data_line_index >= 4)
    {
		data.success = true;
    }
    else
    {
        std::cerr << "Error: No Tasks found in file." << std::endl;
		data.success = false;
    }
    return data;
}
