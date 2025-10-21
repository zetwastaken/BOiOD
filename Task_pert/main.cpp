#include <iostream>
#include <string>

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

    CPMResult cpmResult = CPMCalculator::analyze(projectData.tasks);
    CPMResult cpmResultBF = CPMCalculator::analyzeBellmanFord(projectData.tasks);

    PERTResult pertResult = PERTCalculator::analyze(pertData.tasks);

    std::cout << "File paths:\n";
    std::cout << "  CPM data file: " << cpmFile << '\n';

    ResultPrinter::printCPM(projectData, cpmResult, std::cout);
    ResultPrinter::printCPM(projectData, cpmResultBF, std::cout);

    // std::cout << "  PERT data file: " << pertFile << '\n';
    // std::cout << std::endl;
    // if (!pertResult.criticalPath.empty())
    // {
    //     ResultPrinter::printPERT(pertData, pertResult, std::cout);
    // }


    return 0;
}