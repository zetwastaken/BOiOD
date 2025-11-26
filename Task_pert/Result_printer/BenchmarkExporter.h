#ifndef BENCHMARK_EXPORTER_H
#define BENCHMARK_EXPORTER_H

#include <string>
#include <vector>

struct CPMBenchmarkResult
{
    int nodes = 0;
    int edges = 0;
    double topoTimeMs = 0.0;
    int64_t topoTimeUs = 0;
    double bellmanTimeMs = 0.0;
    int64_t bellmanTimeUs = 0;
    double ratio = 0.0;
    int totalDuration = 0;
    int criticalPathLength = 0;
};

struct PERTBenchmarkResult
{
    int nodes = 0;
    int edges = 0;
    double targetTime = 0.0;
    double targetProbability = 0.0;
    
    // Analytic results
    int64_t analyticTimeUs = 0;
    double analyticExpected = 0.0;
    double analyticStdDev = 0.0;
    double analyticProbability = 0.0;
    double analyticRequired = 0.0;
    
    // Simulation results
    bool hasSimulation = false;
    int simulations = 0;
    int64_t simulationTimeUs = 0;
    double simExpected = 0.0;
    double simStdDev = 0.0;
    double simProbability = 0.0;
    double simRequired = 0.0;
    
    // Comparison
    double expectedDiff = 0.0;
    double stdDevDiff = 0.0;
    double probabilityDiff = 0.0;
    double requiredDiff = 0.0;
};

class BenchmarkExporter
{
public:
    // Export CPM benchmark results to CSV
    static bool exportCPMBenchmark(
        const std::string& filename,
        const std::vector<CPMBenchmarkResult>& results,
        const std::string& header = ""
    );
    
    // Export PERT benchmark results to CSV
    static bool exportPERTBenchmark(
        const std::string& filename,
        const std::vector<PERTBenchmarkResult>& results,
        const std::string& header = ""
    );
    
    // Export combined benchmark results
    static bool exportCombinedBenchmark(
        const std::string& filename,
        const std::vector<CPMBenchmarkResult>& cpmResults,
        const std::vector<PERTBenchmarkResult>& pertResults,
        const std::string& header = ""
    );
    
    // Append results to existing file (for multiple runs)
    static bool appendCPMBenchmark(
        const std::string& filename,
        const std::vector<CPMBenchmarkResult>& results
    );
    
    static bool appendPERTBenchmark(
        const std::string& filename,
        const std::vector<PERTBenchmarkResult>& results
    );
};

#endif // BENCHMARK_EXPORTER_H
