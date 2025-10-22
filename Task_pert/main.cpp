#include <iostream>
#include <string>
#include <chrono>
#include <iomanip>

#include "CPMCalculator.h"
#include "DataLoader.h"
#include "DataLoader_pert.h"
#include "PERTCalculator.h"
#include "ResultPrinter.h"

namespace
{
constexpr const char* kDefaultCpmFile = "problem_data/data00.txt";
constexpr const char* kDefaultPertFile = "problem_data/pert_data.txt";
}

int main(int argc, char* argv[])
{
    const std::string cpmFile = argc > 1 ? argv[1] : kDefaultCpmFile;
    const std::string pertFile = argc > 2 ? argv[2] : kDefaultPertFile;

    ProjectData projectData = DataLoader::read_data(cpmFile);
    if (!projectData.success)
    {
        std::cerr << "Error while reading project data: " << cpmFile << '\n';
        return 1;
    }

    ProjectDataPert pertData = DataLoader_pert::read_data(pertFile);
    if (!pertData.success)
    {
        std::cerr << "Error while reading pert data: " << pertFile << '\n';
        return 1;
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
    constexpr int kNumSimulations = 1000000;
    auto startMC = std::chrono::high_resolution_clock::now();
    PERTSimulation simulationResult = PERTCalculator::analyzeSimulation(pertData.tasks, kNumSimulations);
    auto endMC = std::chrono::high_resolution_clock::now();
    auto durationMC = std::chrono::duration_cast<std::chrono::microseconds>(endMC - startMC);

    std::cout << "File paths:\n";
    std::cout << "  CPM data file: " << cpmFile << '\n';

    // ResultPrinter::printCPM(projectData, cpmResult, std::cout);
    // ResultPrinter::printCPM(projectData, cpmResultBF, std::cout);

    std::cout << "  PERT data file: " << pertFile << '\n';
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