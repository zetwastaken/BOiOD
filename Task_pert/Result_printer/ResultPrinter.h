#ifndef RESULT_PRINTER_H
#define RESULT_PRINTER_H

#include <iosfwd>
#include <map>

#include "CPMCalculator.h"
#include "DataLoader.h"
#include "DataLoader_pert.h"
#include "PERTCalculator.h"

class ResultPrinter
{
public:
    static void printCPM(const ProjectData& projectData,
                         const CPMResult& result,
                         std::ostream& output);

    static void printPERT(const ProjectDataPert& projectData,
                          const PERTResult& result,
                          std::ostream& output);
};

#endif // RESULT_PRINTER_H
