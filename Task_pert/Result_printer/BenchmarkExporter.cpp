#include "BenchmarkExporter.h"

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace
{
std::string getCurrentTimestamp()
{
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf;
    
#if defined(_WIN32)
    localtime_s(&tm_buf, &time);
#else
    localtime_r(&time, &tm_buf);
#endif
    
    std::ostringstream oss;
    oss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

bool fileExists(const std::string& filename)
{
    std::ifstream file(filename);
    return file.good();
}
}

bool BenchmarkExporter::exportCPMBenchmark(
    const std::string& filename,
    const std::vector<CPMBenchmarkResult>& results,
    const std::string& header)
{
    std::ofstream file(filename);
    if (!file)
    {
        std::cerr << "Error: Could not open file for writing: " << filename << '\n';
        return false;
    }
    
    // Write header comment
    if (!header.empty())
    {
        file << "# " << header << '\n';
    }
    file << "# Generated: " << getCurrentTimestamp() << '\n';
    file << "# CPM Benchmark: Topological Sort vs Bellman-Ford\n";
    file << '\n';
    
    // Write CSV header
    file << "Nodes,Edges,TopoTimeMs,TopoTimeUs,BellmanTimeMs,BellmanTimeUs,Ratio,"
         << "TotalDuration,CriticalPathLength\n";
    
    // Write data
    file << std::fixed << std::setprecision(6);
    for (const auto& result : results)
    {
        file << result.nodes << ','
             << result.edges << ','
             << result.topoTimeMs << ','
             << result.topoTimeUs << ','
             << result.bellmanTimeMs << ','
             << result.bellmanTimeUs << ','
             << result.ratio << ','
             << result.totalDuration << ','
             << result.criticalPathLength << '\n';
    }
    
    file.close();
    std::cout << "CPM benchmark results exported to: " << filename << '\n';
    return true;
}

bool BenchmarkExporter::exportPERTBenchmark(
    const std::string& filename,
    const std::vector<PERTBenchmarkResult>& results,
    const std::string& header)
{
    std::ofstream file(filename);
    if (!file)
    {
        std::cerr << "Error: Could not open file for writing: " << filename << '\n';
        return false;
    }
    
    // Write header comment
    if (!header.empty())
    {
        file << "# " << header << '\n';
    }
    file << "# Generated: " << getCurrentTimestamp() << '\n';
    file << "# PERT Benchmark: Analytic vs Monte Carlo Simulation\n";
    file << '\n';
    
    // Write CSV header
    file << "Nodes,Edges,TargetTime,TargetProbability,"
         << "AnalyticTimeUs,AnalyticExpected,AnalyticStdDev,AnalyticProbability,AnalyticRequired,"
         << "HasSimulation,Simulations,SimulationTimeUs,SimExpected,SimStdDev,SimProbability,SimRequired,"
         << "ExpectedDiff,StdDevDiff,ProbabilityDiff,RequiredDiff\n";
    
    // Write data
    file << std::fixed << std::setprecision(6);
    for (const auto& result : results)
    {
        file << result.nodes << ','
             << result.edges << ','
             << result.targetTime << ','
             << result.targetProbability << ','
             << result.analyticTimeUs << ','
             << result.analyticExpected << ','
             << result.analyticStdDev << ','
             << result.analyticProbability << ','
             << result.analyticRequired << ','
             << (result.hasSimulation ? 1 : 0) << ','
             << result.simulations << ','
             << result.simulationTimeUs << ','
             << result.simExpected << ','
             << result.simStdDev << ','
             << result.simProbability << ','
             << result.simRequired << ','
             << result.expectedDiff << ','
             << result.stdDevDiff << ','
             << result.probabilityDiff << ','
             << result.requiredDiff << '\n';
    }
    
    file.close();
    std::cout << "PERT benchmark results exported to: " << filename << '\n';
    return true;
}

bool BenchmarkExporter::exportCombinedBenchmark(
    const std::string& filename,
    const std::vector<CPMBenchmarkResult>& cpmResults,
    const std::vector<PERTBenchmarkResult>& pertResults,
    const std::string& header)
{
    std::ofstream file(filename);
    if (!file)
    {
        std::cerr << "Error: Could not open file for writing: " << filename << '\n';
        return false;
    }
    
    // Write header comment
    if (!header.empty())
    {
        file << "# " << header << '\n';
    }
    file << "# Generated: " << getCurrentTimestamp() << '\n';
    file << "# Combined CPM and PERT Benchmark Results\n";
    file << '\n';
    
    // CPM Section
    file << "## CPM Results (Topological vs Bellman-Ford)\n";
    file << "Nodes,Edges,TopoTimeMs,TopoTimeUs,BellmanTimeMs,BellmanTimeUs,Ratio,"
         << "TotalDuration,CriticalPathLength\n";
    
    file << std::fixed << std::setprecision(6);
    for (const auto& result : cpmResults)
    {
        file << result.nodes << ','
             << result.edges << ','
             << result.topoTimeMs << ','
             << result.topoTimeUs << ','
             << result.bellmanTimeMs << ','
             << result.bellmanTimeUs << ','
             << result.ratio << ','
             << result.totalDuration << ','
             << result.criticalPathLength << '\n';
    }
    
    file << '\n';
    
    // PERT Section
    file << "## PERT Results (Analytic vs Simulation)\n";
    file << "Nodes,Edges,TargetTime,TargetProbability,"
         << "AnalyticTimeUs,AnalyticExpected,AnalyticStdDev,AnalyticProbability,AnalyticRequired,"
         << "HasSimulation,Simulations,SimulationTimeUs,SimExpected,SimStdDev,SimProbability,SimRequired,"
         << "ExpectedDiff,StdDevDiff,ProbabilityDiff,RequiredDiff\n";
    
    for (const auto& result : pertResults)
    {
        file << result.nodes << ','
             << result.edges << ','
             << result.targetTime << ','
             << result.targetProbability << ','
             << result.analyticTimeUs << ','
             << result.analyticExpected << ','
             << result.analyticStdDev << ','
             << result.analyticProbability << ','
             << result.analyticRequired << ','
             << (result.hasSimulation ? 1 : 0) << ','
             << result.simulations << ','
             << result.simulationTimeUs << ','
             << result.simExpected << ','
             << result.simStdDev << ','
             << result.simProbability << ','
             << result.simRequired << ','
             << result.expectedDiff << ','
             << result.stdDevDiff << ','
             << result.probabilityDiff << ','
             << result.requiredDiff << '\n';
    }
    
    file.close();
    std::cout << "Combined benchmark results exported to: " << filename << '\n';
    return true;
}

bool BenchmarkExporter::appendCPMBenchmark(
    const std::string& filename,
    const std::vector<CPMBenchmarkResult>& results)
{
    const bool exists = fileExists(filename);
    std::ofstream file(filename, std::ios::app);
    
    if (!file)
    {
        std::cerr << "Error: Could not open file for appending: " << filename << '\n';
        return false;
    }
    
    // Write header if file is new
    if (!exists)
    {
        file << "# Generated: " << getCurrentTimestamp() << '\n';
        file << "# CPM Benchmark: Topological Sort vs Bellman-Ford\n";
        file << '\n';
        file << "Timestamp,Nodes,Edges,TopoTimeMs,TopoTimeUs,BellmanTimeMs,BellmanTimeUs,Ratio,"
             << "TotalDuration,CriticalPathLength\n";
    }
    
    // Write data with timestamp
    const std::string timestamp = getCurrentTimestamp();
    file << std::fixed << std::setprecision(6);
    for (const auto& result : results)
    {
        file << timestamp << ','
             << result.nodes << ','
             << result.edges << ','
             << result.topoTimeMs << ','
             << result.topoTimeUs << ','
             << result.bellmanTimeMs << ','
             << result.bellmanTimeUs << ','
             << result.ratio << ','
             << result.totalDuration << ','
             << result.criticalPathLength << '\n';
    }
    
    file.close();
    std::cout << "CPM benchmark results appended to: " << filename << '\n';
    return true;
}

bool BenchmarkExporter::appendPERTBenchmark(
    const std::string& filename,
    const std::vector<PERTBenchmarkResult>& results)
{
    const bool exists = fileExists(filename);
    std::ofstream file(filename, std::ios::app);
    
    if (!file)
    {
        std::cerr << "Error: Could not open file for appending: " << filename << '\n';
        return false;
    }
    
    // Write header if file is new
    if (!exists)
    {
        file << "# Generated: " << getCurrentTimestamp() << '\n';
        file << "# PERT Benchmark: Analytic vs Monte Carlo Simulation\n";
        file << '\n';
        file << "Timestamp,Nodes,Edges,TargetTime,TargetProbability,"
             << "AnalyticTimeUs,AnalyticExpected,AnalyticStdDev,AnalyticProbability,AnalyticRequired,"
             << "HasSimulation,Simulations,SimulationTimeUs,SimExpected,SimStdDev,SimProbability,SimRequired,"
             << "ExpectedDiff,StdDevDiff,ProbabilityDiff,RequiredDiff\n";
    }
    
    // Write data with timestamp
    const std::string timestamp = getCurrentTimestamp();
    file << std::fixed << std::setprecision(6);
    for (const auto& result : results)
    {
        file << timestamp << ','
             << result.nodes << ','
             << result.edges << ','
             << result.targetTime << ','
             << result.targetProbability << ','
             << result.analyticTimeUs << ','
             << result.analyticExpected << ','
             << result.analyticStdDev << ','
             << result.analyticProbability << ','
             << result.analyticRequired << ','
             << (result.hasSimulation ? 1 : 0) << ','
             << result.simulations << ','
             << result.simulationTimeUs << ','
             << result.simExpected << ','
             << result.simStdDev << ','
             << result.simProbability << ','
             << result.simRequired << ','
             << result.expectedDiff << ','
             << result.stdDevDiff << ','
             << result.probabilityDiff << ','
             << result.requiredDiff << '\n';
    }
    
    file.close();
    std::cout << "PERT benchmark results appended to: " << filename << '\n';
    return true;
}
