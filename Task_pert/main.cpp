#include <chrono>
#include <algorithm>
#include <cstdint>
#include <cmath>
#include <exception>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
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
constexpr int kDefaultNumSimulations = 100000;

struct GeneratorOptions
{
    PertProblemGenerator::Parameters params{};
    std::optional<std::uint32_t> seed;
};

struct BenchmarkOptions
{
    bool enabled = false;
    std::vector<int> sizes{10, 30, 50, 100, 200, 400};
    std::uint32_t seed = 123;
    int pertSimulations = 1000;
};

struct ParsedArguments
{
    bool useGenerator = false;
    GeneratorOptions generator;
    std::vector<std::string> positional;
    BenchmarkOptions benchmark;
    bool skipSimulation = false;
    std::optional<int> simulationRuns;
};

bool parseArguments(int argc, char* argv[], ParsedArguments& out)
{
    auto parseNodeList = [](const std::string& csv, std::vector<int>& output) -> bool {
        std::vector<int> parsed;
        std::stringstream ss(csv);
        std::string token;
        while (std::getline(ss, token, ','))
        {
            if (token.empty())
            {
                continue;
            }
            try
            {
                const int value = std::stoi(token);
                if (value <= 0)
                {
                    return false;
                }
                parsed.push_back(value);
            }
            catch (const std::exception&)
            {
                return false;
            }
        }

        if (parsed.empty())
        {
            return false;
        }
        output = std::move(parsed);
        return true;
    };

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
        else if (argument == "--benchmark-cpm" || argument == "--benchmark")
        {
            out.benchmark.enabled = true;
            continue;
        }
        else if (argument == "--benchmark-pert-sim")
        {
            if (i + 1 >= argc)
            {
                std::cerr << "Missing value for " << argument << '\n';
                return false;
            }
            const std::string value = argv[++i];
            try
            {
                const int runs = std::stoi(value);
                if (runs < 0)
                {
                    std::cerr << "Benchmark simulation count must be non-negative: " << value << '\n';
                    return false;
                }
                out.benchmark.pertSimulations = runs;
            }
            catch (const std::exception&)
            {
                std::cerr << "Invalid integer value for " << argument << ": " << value << '\n';
                return false;
            }
            out.benchmark.enabled = true;
            continue;
        }
        else if (argument == "--benchmark-seed")
        {
            if (i + 1 >= argc)
            {
                std::cerr << "Missing value for " << argument << '\n';
                return false;
            }
            const std::string value = argv[++i];
            try
            {
                out.benchmark.seed = static_cast<std::uint32_t>(std::stoul(value));
            }
            catch (const std::exception&)
            {
                std::cerr << "Invalid seed value for benchmark: " << value << '\n';
                return false;
            }
            out.benchmark.enabled = true;
            continue;
        }
        else if (argument == "--benchmark-sizes")
        {
            if (i + 1 >= argc)
            {
                std::cerr << "Missing value for " << argument << '\n';
                return false;
            }
            const std::string value = argv[++i];
            if (!parseNodeList(value, out.benchmark.sizes))
            {
                std::cerr << "Invalid benchmark node list: " << value << '\n';
                return false;
            }
            out.benchmark.enabled = true;
            continue;
        }
        else if (argument == "--skip-simulation")
        {
            out.skipSimulation = true;
            continue;
        }
        else if (argument == "--simulations")
        {
            if (i + 1 >= argc)
            {
                std::cerr << "Missing value for " << argument << '\n';
                return false;
            }
            const std::string value = argv[++i];
            try
            {
                const int runs = std::stoi(value);
                if (runs < 0)
                {
                    std::cerr << "Simulation count must be non-negative: " << value << '\n';
                    return false;
                }
                out.simulationRuns = runs;
            }
            catch (const std::exception&)
            {
                std::cerr << "Invalid integer value for " << argument << ": " << value << '\n';
                return false;
            }
            continue;
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

double normalCDF(double z)
{
    return 0.5 * (1.0 + std::erf(z / std::sqrt(2.0)));
}

double normalInvCDF(double p)
{
    if (p > 0.999)
    {
        return 3.29;
    }
    if (p > 0.99)
    {
        return 2.33;
    }
    if (p < 0.001)
    {
        return -3.29;
    }
    if (p < 0.01)
    {
        return -2.33;
    }

    double low = -8.0;
    double high = 8.0;
    for (int i = 0; i < 100; ++i)
    {
        const double mid = (low + high) / 2.0;
        if (normalCDF(mid) < p)
        {
            low = mid;
        }
        else
        {
            high = mid;
        }
    }
    return high;
}

int runBenchmark(const ParsedArguments& args)
{
    if (args.benchmark.sizes.empty())
    {
        std::cerr << "Benchmark requires at least one node size.\n";
        return 1;
    }

    std::cout << "CPM benchmark (topological vs Bellman-Ford)\n";
    std::cout << "Seed: " << args.benchmark.seed << '\n';

    PertProblemGenerator::Parameters baseParams = args.generator.params;
    std::mt19937 seeder(args.benchmark.seed);

    struct Row
    {
        int nodes = 0;
        double topoMs = 0.0;
        std::int64_t topoUs = 0;
        double bellmanMs = 0.0;
        std::int64_t bellmanUs = 0;
        double ratio = 0.0;
    };

    std::vector<Row> results;
    results.reserve(args.benchmark.sizes.size());

    struct PertRow
    {
        int nodes = 0;
        double targetTime = 0.0;
        double targetProbability = 0.0;
        std::int64_t analyticTimeUs = 0;
        double analyticExpected = 0.0;
        double analyticStdDev = 0.0;
        double analyticProbability = 0.0;
        double analyticRequired = 0.0;
        bool hasSimulation = false;
        int simulations = 0;
        std::int64_t simulationTimeUs = 0;
        double simExpected = 0.0;
        double simStdDev = 0.0;
        double simProbability = 0.0;
        double simRequired = 0.0;
    };

    std::vector<PertRow> pertResults;
    pertResults.reserve(args.benchmark.sizes.size());

    for (int nodes : args.benchmark.sizes)
    {
        if (nodes <= 0)
        {
            std::cerr << "Skipping invalid node count: " << nodes << '\n';
            continue;
        }

        PertProblemGenerator::Parameters params = baseParams;
        params.nodeCount = nodes;
        const double densityScale = std::sqrt(static_cast<double>(nodes) /
                                              std::max(1, baseParams.nodeCount));
        params.xMax = baseParams.xMax * densityScale;
        params.yMax = baseParams.yMax * densityScale;
        params.minNodeDistance = std::max(0.5, baseParams.minNodeDistance / densityScale);
        params.minEdgeDistance = std::max(0.5, baseParams.minEdgeDistance / densityScale);
        const std::uint32_t instanceSeed = static_cast<std::uint32_t>(seeder());

        ProjectDataPert pertData = PertProblemGenerator::generate(params, instanceSeed);
        if (!pertData.success)
        {
            std::cerr << "Failed to generate benchmark instance for " << nodes << " nodes.\n";
            return 1;
        }

        ProjectData projectData = createProjectDataFromPert(pertData);
        if (!projectData.success)
        {
            std::cerr << "Failed to map PERT data to CPM tasks for " << nodes << " nodes.\n";
            return 1;
        }

        auto tasksTopo = projectData.tasks;
        auto startTopo = std::chrono::high_resolution_clock::now();
        CPMResult topoResult = CPMCalculator::analyze(tasksTopo);
        auto endTopo = std::chrono::high_resolution_clock::now();

        auto tasksBellman = projectData.tasks;
        auto startBellman = std::chrono::high_resolution_clock::now();
        CPMResult bellmanResult = CPMCalculator::analyzeBellmanFord(tasksBellman);
        auto endBellman = std::chrono::high_resolution_clock::now();

        const auto topoDuration = std::chrono::duration_cast<std::chrono::microseconds>(endTopo - startTopo);
        const auto bellmanDuration = std::chrono::duration_cast<std::chrono::microseconds>(endBellman - startBellman);

        if (topoResult.totalDuration != bellmanResult.totalDuration)
        {
            std::cerr << "Warning: total duration mismatch detected for " << nodes << " nodes.\n";
        }

        Row row;
        row.nodes = nodes;
        row.topoMs = topoDuration.count() / 1000.0;
        row.topoUs = topoDuration.count();
        row.bellmanMs = bellmanDuration.count() / 1000.0;
        row.bellmanUs = bellmanDuration.count();
        row.ratio = row.topoMs > 0.0 ? row.bellmanMs / row.topoMs : 0.0;

        results.push_back(row);

        auto pertTasksAnalytic = pertData.tasks;
        auto startPertAnalytic = std::chrono::high_resolution_clock::now();
        PERTResult pertAnalytic = PERTCalculator::analyze(pertTasksAnalytic);
        auto endPertAnalytic = std::chrono::high_resolution_clock::now();
        const auto pertAnalyticDuration = std::chrono::duration_cast<std::chrono::microseconds>(endPertAnalytic - startPertAnalytic);

        PertRow pertRow;
        pertRow.nodes = nodes;
        pertRow.targetTime = pertData.target_time;
        pertRow.targetProbability = pertData.target_probability;
        pertRow.analyticTimeUs = pertAnalyticDuration.count();
        pertRow.analyticExpected = pertAnalytic.expectedDuration;
        pertRow.analyticStdDev = pertAnalytic.standardDeviation;
        if (pertAnalytic.standardDeviation > 0.0)
        {
            const double zScore = (pertRow.targetTime - pertRow.analyticExpected) / pertRow.analyticStdDev;
            pertRow.analyticProbability = normalCDF(zScore);
            const double zForProb = normalInvCDF(pertRow.targetProbability);
            pertRow.analyticRequired = pertRow.analyticExpected + pertRow.analyticStdDev * zForProb;
        }
        else
        {
            pertRow.analyticProbability = pertRow.targetTime >= pertRow.analyticExpected ? 1.0 : 0.0;
            pertRow.analyticRequired = pertRow.analyticExpected;
        }

        if (args.benchmark.pertSimulations > 0)
        {
            auto pertTasksSim = pertData.tasks;
            auto startPertSim = std::chrono::high_resolution_clock::now();
            PERTSimulation pertSimulation = PERTCalculator::analyzeSimulation(pertTasksSim, args.benchmark.pertSimulations);
            auto endPertSim = std::chrono::high_resolution_clock::now();
            const auto pertSimDuration = std::chrono::duration_cast<std::chrono::microseconds>(endPertSim - startPertSim);
            pertRow.hasSimulation = pertSimulation.simulations > 0 && !pertSimulation.completionTimes.empty();
            pertRow.simulations = pertSimulation.simulations;
            if (pertRow.hasSimulation)
            {
                pertRow.simulationTimeUs = pertSimDuration.count();
                pertRow.simExpected = pertSimulation.meanDuration;
                pertRow.simStdDev = pertSimulation.standardDeviation;

                int onTimeCount = 0;
                for (double value : pertSimulation.completionTimes)
                {
                    if (value <= pertRow.targetTime)
                    {
                        ++onTimeCount;
                    }
                }
                pertRow.simProbability = static_cast<double>(onTimeCount) / static_cast<double>(pertSimulation.completionTimes.size());
                pertRow.simRequired = pertSimulation.getPercentile(pertRow.targetProbability * 100.0);
            }
        }

        pertResults.push_back(pertRow);
    }

    if (results.empty())
    {
        std::cerr << "Benchmark did not produce any data.\n";
        return 1;
    }

    std::cout << '\n'
              << std::left << std::setw(8) << "Nodes"
              << std::right << std::setw(16) << "Topo (ms)"
              << std::setw(14) << "Topo (µs)"
              << std::setw(16) << "Bellman (ms)"
              << std::setw(14) << "Bellman (µs)"
              << std::setw(10) << "Ratio" << '\n';
    std::cout << std::string(78, '-') << '\n';

    std::cout << std::fixed << std::setprecision(3);
    for (const Row& row : results)
    {
        std::cout << std::left << std::setw(8) << row.nodes
                  << std::right << std::setw(16) << row.topoMs
                  << std::setw(14) << row.topoUs
                  << std::setw(16) << row.bellmanMs
                  << std::setw(14) << row.bellmanUs
                  << std::setw(10) << row.ratio
                  << '\n';
    }

    std::cout << '\n';
    std::cout.unsetf(std::ios::floatfield);

    if (!pertResults.empty())
    {
        std::cout << "PERT benchmark (analytic vs simulation)\n";
        if (args.benchmark.pertSimulations > 0)
        {
            std::cout << "Simulations per instance: " << args.benchmark.pertSimulations << '\n';
        }
        else
        {
            std::cout << "Simulation disabled (use --benchmark-pert-sim N to enable).\n";
        }

        const std::string separator(192, '-');
        std::cout << std::left
                  << std::setw(8) << "Nodes"
                  << std::setw(12) << "Target"
                  << std::setw(12) << "Prob (%)"
                  << std::setw(14) << "Analytic μ"
                  << std::setw(14) << "Analytic σ"
                  << std::setw(14) << "Analytic (ms)"
                  << std::setw(18) << "Analytic P<=T"
                  << std::setw(20) << "Analytic T@Prob"
                  << std::setw(14) << "Sim μ"
                  << std::setw(14) << "Sim σ"
                  << std::setw(14) << "Sim (ms)"
                  << std::setw(18) << "Sim P<=T"
                  << std::setw(20) << "Sim T@Prob"
                  << '\n';
        std::cout << separator << '\n';

        auto formatValue = [](double value, int precision) -> std::string {
            std::ostringstream oss;
            oss.setf(std::ios::fixed);
            oss << std::setprecision(precision) << value;
            return oss.str();
        };

        auto formatOptional = [&](bool available, double value, int precision) -> std::string {
            if (!available)
            {
                return "n/a";
            }
            return formatValue(value, precision);
        };

        for (const PertRow& row : pertResults)
        {
            std::cout << std::left
                      << std::setw(8) << row.nodes
                      << std::setw(12) << formatValue(row.targetTime, 3)
                      << std::setw(12) << formatValue(row.targetProbability * 100.0, 2)
                      << std::setw(14) << formatValue(row.analyticExpected, 3)
                      << std::setw(14) << formatValue(row.analyticStdDev, 3)
                      << std::setw(14) << formatValue(static_cast<double>(row.analyticTimeUs) / 1000.0, 3)
                      << std::setw(18) << formatValue(row.analyticProbability, 4)
                      << std::setw(20) << formatValue(row.analyticRequired, 3)
                      << std::setw(14) << formatOptional(row.hasSimulation, row.simExpected, 3)
                      << std::setw(14) << formatOptional(row.hasSimulation, row.simStdDev, 3)
                      << std::setw(14) << formatOptional(row.hasSimulation,
                                                         static_cast<double>(row.simulationTimeUs) / 1000.0,
                                                         3)
                      << std::setw(18) << formatOptional(row.hasSimulation, row.simProbability, 4)
                      << std::setw(20) << formatOptional(row.hasSimulation, row.simRequired, 3)
                      << '\n';
        }

        std::cout << separator << '\n';
        std::cout << '\n';
    }

    return 0;
}
} // namespace

int main(int argc, char* argv[])
{
    ParsedArguments args;
    if (!parseArguments(argc, argv, args))
    {
        return 1;
    }

    if (args.benchmark.enabled)
    {
        return runBenchmark(args);
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
    int simulationRuns = args.simulationRuns.value_or(kDefaultNumSimulations);
    if (args.skipSimulation)
    {
        simulationRuns = 0;
    }

    std::optional<std::chrono::microseconds> durationMC;
    PERTSimulation simulationResult;
    if (simulationRuns > 0)
    {
        auto startMC = std::chrono::high_resolution_clock::now();
        simulationResult = PERTCalculator::analyzeSimulation(pertData.tasks, simulationRuns);
        auto endMC = std::chrono::high_resolution_clock::now();
        durationMC = std::chrono::duration_cast<std::chrono::microseconds>(endMC - startMC);
    }

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
    if (durationMC.has_value())
    {
        ResultPrinter::printSimulation(simulationResult, pertData.target_time, pertData.target_probability, std::cout);
    }
    else
    {
        std::cout << "\nPERT simulation skipped (use --simulations N to configure runs).\n";
    }
    ResultPrinter::printPERTSummary(pertData, pertResult, durationMC.has_value() ? &simulationResult : nullptr, std::cout);

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

    if (durationMC.has_value())
    {
        std::cout << "PERT Simulation:   " << std::setw(10) << durationMC->count() / 1000.0 << " ms";
        std::cout << " (" << durationMC->count() << " µs)\n";
    }
    else
    {
        std::cout << "PERT Simulation:           skipped\n";
    }
    std::cout << "========================================\n";

    return 0;
}
