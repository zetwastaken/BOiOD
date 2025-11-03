#include <chrono>
#include <cstdint>
#include <cmath>
#include <exception>
#include <iomanip>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "CPMCalculator.h"
#include "DataLoader.h"
#include "DataLoader_pert.h"
#include "PERTCalculator.h"
#include "PertProblemGenerator.h"
#include "ResultPrinter.h"

namespace
{
constexpr const char* kDefaultCpmFile = "problem_data/data00.txt";
constexpr const char* kDefaultPertFile = "problem_data/pert_data_3.txt";

struct GeneratorOptions
{
    PertProblemGenerator::Parameters params{};
    std::optional<std::uint32_t> seed;
};

struct ParsedArguments
{
    bool useGenerator = false;
    GeneratorOptions generator;
    std::vector<std::string> positional;
};

bool parseArguments(int argc, char* argv[], ParsedArguments& out)
{
    for (int i = 1; i < argc; ++i)
    {
        const std::string argument = argv[i];
        if (argument == "--generate" || argument == "--generate-pert")
        {
            out.useGenerator = true;
            continue;
        }

        if (argument == "--seed" || argument == "-s")
        {
            if (i + 1 >= argc)
            {
                std::cerr << "Missing value for " << argument << '\n';
                return false;
            }
            const std::string value = argv[++i];
            try
            {
                out.generator.seed = static_cast<std::uint32_t>(std::stoul(value));
            }
            catch (const std::exception&)
            {
                std::cerr << "Invalid seed value: " << value << '\n';
                return false;
            }
            out.useGenerator = true;
            continue;
        }

        auto requireInt = [&](const std::string& option, int& target) -> bool {
            if (i + 1 >= argc)
            {
                std::cerr << "Missing value for " << option << '\n';
                return false;
            }
            const std::string value = argv[++i];
            try
            {
                target = std::stoi(value);
            }
            catch (const std::exception&)
            {
                std::cerr << "Invalid integer value for " << option << ": " << value << '\n';
                return false;
            }
            out.useGenerator = true;
            return true;
        };

        auto requireDouble = [&](const std::string& option, double& target) -> bool {
            if (i + 1 >= argc)
            {
                std::cerr << "Missing value for " << option << '\n';
                return false;
            }
            const std::string value = argv[++i];
            try
            {
                target = std::stod(value);
            }
            catch (const std::exception&)
            {
                std::cerr << "Invalid numeric value for " << option << ": " << value << '\n';
                return false;
            }
            out.useGenerator = true;
            return true;
        };

        if (argument == "--nodes")
        {
            if (!requireInt(argument, out.generator.params.nodeCount))
            {
                return false;
            }
        }
        else if (argument == "--x-max")
        {
            if (!requireDouble(argument, out.generator.params.xMax))
            {
                return false;
            }
        }
        else if (argument == "--y-max")
        {
            if (!requireDouble(argument, out.generator.params.yMax))
            {
                return false;
            }
        }
        else if (argument == "--min-node-distance")
        {
            if (!requireDouble(argument, out.generator.params.minNodeDistance))
            {
                return false;
            }
        }
        else if (argument == "--min-edge-distance")
        {
            if (!requireDouble(argument, out.generator.params.minEdgeDistance))
            {
                return false;
            }
        }
        else if (argument == "--min-angle-degrees")
        {
            if (!requireDouble(argument, out.generator.params.minAngleDegrees))
            {
                return false;
            }
        }
        else if (argument == "--max-duration")
        {
            if (!requireInt(argument, out.generator.params.maxDuration))
            {
                return false;
            }
        }
        else if (!argument.empty() && argument[0] == '-')
        {
            std::cerr << "Unknown option: " << argument << '\n';
            return false;
        }
        else
        {
            out.positional.push_back(argument);
        }
    }

    return true;
}

ProjectData createProjectDataFromPert(const ProjectDataPert& pertData)
{
    ProjectData project;
    project.N = pertData.N;
    project.M = pertData.M;

    for (const auto& [id, pertTask] : pertData.tasks)
    {
        const double expected = pertTask.expected_duration;
        int duration = static_cast<int>(std::lround(expected));
        if (duration <= 0)
        {
            duration = static_cast<int>(std::ceil(expected));
        }
        if (duration <= 0)
        {
            duration = 1;
        }

        Task task(id, duration);
        task.predecessors = pertTask.predecessors;
        task.successors = pertTask.successors;
        project.tasks.emplace(id, std::move(task));
    }

    if (static_cast<int>(project.tasks.size()) != project.N)
    {
        project.success = false;
        return project;
    }

    project.success = true;
    return project;
}
} // namespace

int main(int argc, char* argv[])
{
    ParsedArguments args;
    if (!parseArguments(argc, argv, args))
    {
        return 1;
    }

    const std::string cpmFile = args.positional.size() > 0 ? args.positional[0] : kDefaultCpmFile;
    const std::string pertFile = args.positional.size() > 1 ? args.positional[1] : kDefaultPertFile;

    ProjectData projectData;
    ProjectDataPert pertData;

    if (args.useGenerator)
    {
        pertData = PertProblemGenerator::generate(args.generator.params, args.generator.seed);
        if (!pertData.success)
        {
            std::cerr << "Error: Failed to generate PERT problem instance\n";
            return 1;
        }

        projectData = createProjectDataFromPert(pertData);
        if (!projectData.success)
        {
            std::cerr << "Error: Failed to derive CPM data from generated PERT instance\n";
            return 1;
        }
    }
    else
    {
        projectData = DataLoader::read_data(cpmFile);
        if (!projectData.success)
        {
            std::cerr << "Error while reading project data: " << cpmFile << '\n';
            return 1;
        }

        pertData = DataLoader_pert::read_data(pertFile);
        if (!pertData.success)
        {
            std::cerr << "Error while reading pert data: " << pertFile << '\n';
            return 1;
        }
    }

    // CPM Analysis with timing
    auto startCPM = std::chrono::high_resolution_clock::now();
    CPMResult cpmResult = CPMCalculator::analyze(projectData.tasks);
    auto endCPM = std::chrono::high_resolution_clock::now();
    auto durationCPM = std::chrono::duration_cast<std::chrono::microseconds>(endCPM - startCPM);

    // CPM Bellman-Ford with timing
    auto startCPMBF = std::chrono::high_resolution_clock::now();
    CPMResult cpmResultBF = CPMCalculator::analyzeBellmanFord(projectData.tasks);
    auto endCPMBF = std::chrono::high_resolution_clock::now();
    auto durationCPMBF = std::chrono::duration_cast<std::chrono::microseconds>(endCPMBF - startCPMBF);

    // PERT Analysis with timing
    auto startPERT = std::chrono::high_resolution_clock::now();
    PERTResult pertResult = PERTCalculator::analyze(pertData.tasks);
    auto endPERT = std::chrono::high_resolution_clock::now();
    auto durationPERT = std::chrono::duration_cast<std::chrono::microseconds>(endPERT - startPERT);

    // PERT Simulation with timing
    constexpr int kNumSimulations = 100000;
    auto startMC = std::chrono::high_resolution_clock::now();
    PERTSimulation simulationResult = PERTCalculator::analyzeSimulation(pertData.tasks, kNumSimulations);
    auto endMC = std::chrono::high_resolution_clock::now();
    auto durationMC = std::chrono::duration_cast<std::chrono::microseconds>(endMC - startMC);

    const std::string cpmSource = args.useGenerator ? "generated from PERT instance" : cpmFile;
    std::string pertSource = args.useGenerator ? "generated instance" : pertFile;
    if (args.useGenerator && args.generator.seed.has_value())
    {
        pertSource += " (seed " + std::to_string(*args.generator.seed) + ')';
    }

    std::cout << "Data sources:\n";
    std::cout << "  CPM data: " << cpmSource << '\n';
    std::cout << "  PERT data: " << pertSource << '\n';
    if (args.useGenerator)
    {
        const auto& p = args.generator.params;
        std::cout << "  Generator parameters: nodes=" << p.nodeCount
                  << ", xMax=" << p.xMax
                  << ", yMax=" << p.yMax
                  << ", minNodeDist=" << p.minNodeDistance
                  << ", minEdgeDist=" << p.minEdgeDistance
                  << ", minAngle=" << p.minAngleDegrees
                  << ", maxDuration=" << p.maxDuration << '\n';
    }

    ResultPrinter::printCPM(projectData, cpmResult, std::cout);
    ResultPrinter::printCPM(projectData, cpmResultBF, std::cout);

    ResultPrinter::printPERT(pertData, pertResult, std::cout);
    ResultPrinter::printSimulation(simulationResult, pertData.target_time, pertData.target_probability, std::cout);

    // Display execution times
    std::cout << "\n========================================\n";
    std::cout << "Execution Times:\n";
    std::cout << "========================================\n";
    std::cout << std::fixed << std::setprecision(3);

    std::cout << "CPM Analysis:             " << std::setw(10) << durationCPM.count() / 1000.0 << " ms";
    std::cout << " (" << durationCPM.count() << " µs)\n";

    std::cout << "CPM Bellman-Ford:         " << std::setw(10) << durationCPMBF.count() / 1000.0 << " ms";
    std::cout << " (" << durationCPMBF.count() << " µs)\n";

    std::cout << "-----------------------------------------\n";

    std::cout << "PERT Analysis:            " << std::setw(10) << durationPERT.count() / 1000.0 << " ms";
    std::cout << " (" << durationPERT.count() << " µs)\n";

    std::cout << "PERT Simulation:   " << std::setw(10) << durationMC.count() / 1000.0 << " ms";
    std::cout << " (" << durationMC.count() << " µs)\n";
    std::cout << "========================================\n";

    return 0;
}
