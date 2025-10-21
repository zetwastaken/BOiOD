#include "ResultPrinter.h"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <unordered_set>

#if !defined(_WIN32)
#include <unistd.h>
#endif

namespace
{
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

constexpr char RESET_COLOR[] = "\033[0m";
constexpr char TITLE_COLOR[] = "\033[1;36m";
constexpr char SECTION_COLOR[] = "\033[1;33m";
constexpr char HEADER_COLOR[] = "\033[1;37m";
constexpr char LABEL_COLOR[] = "\033[0;36m";
constexpr char VALUE_COLOR[] = "\033[0;37m";
constexpr char MATCH_COLOR[] = "\033[1;32m";
constexpr char CRITICAL_COLOR[] = "\033[1;31m";

bool streamSupportsColor(std::ostream& output)
{
#if defined(_WIN32)
    return (&output == &std::cout) || (&output == &std::cerr);
#else
    if ((&output != &std::cout) && (&output != &std::cerr))
    {
        return false;
    }

    const int fd = (&output == &std::cout) ? STDOUT_FILENO : STDERR_FILENO;
    return ::isatty(fd) != 0;
#endif
}

void applyColor(std::ostream& output, bool enabled, const char* color)
{
    if (enabled)
    {
        output << color;
    }
}

std::string taskLabel(int id)
{
    if (id <= 0)
    {
        return std::to_string(id);
    }

    std::string label;
    int value = id - 1;
    while (value >= 0)
    {
        const char letter = static_cast<char>('A' + (value % 26));
        label.insert(label.begin(), letter);
        value = value / 26 - 1;
    }
    return label;
}
}

void ResultPrinter::printCPM(const ProjectData& projectData,
                             const CPMResult& result,
                             std::ostream& output)
{
    const bool useColor = streamSupportsColor(output);
    const std::string separator(74, '-');
    const auto& tasks = projectData.tasks;
    std::unordered_set<int> criticalIds(result.criticalPath.begin(), result.criticalPath.end());

    applyColor(output, useColor, TITLE_COLOR);
    output << '\n' << "=== CPM Analysis Result ===" << '\n';
    applyColor(output, useColor, RESET_COLOR);

    applyColor(output, useColor, LABEL_COLOR);
    output << "Total duration (calculated): ";
    applyColor(output, useColor, VALUE_COLOR);
    output << result.totalDuration << '\n';
    applyColor(output, useColor, RESET_COLOR);

    if (projectData.hasExpectedProcessTime)
    {
        const bool matches = projectData.expectedProcessTime == result.totalDuration;
        const int delta = result.totalDuration - projectData.expectedProcessTime;

        applyColor(output, useColor, LABEL_COLOR);
        output << "Expected duration (file): ";
        applyColor(output, useColor, matches ? MATCH_COLOR : CRITICAL_COLOR);
        output << projectData.expectedProcessTime;
        applyColor(output, useColor, RESET_COLOR);

        if (!matches)
        {
            output << ' ';
            applyColor(output, useColor, CRITICAL_COLOR);
            output << "(delta " << (delta > 0 ? "+" : "") << delta << ')';
            applyColor(output, useColor, RESET_COLOR);
        }

        output << '\n';
    }

    output << '\n';
    applyColor(output, useColor, SECTION_COLOR);
    output << "Task schedule:" << '\n';
    applyColor(output, useColor, RESET_COLOR);
    output << separator << '\n';

    applyColor(output, useColor, HEADER_COLOR);
    output << std::left
           << std::setw(6) << "ID"
           << std::setw(8) << "Dur"
           << std::setw(12) << "EarlyStart"
           << std::setw(12) << "EarlyFinish"
           << std::setw(12) << "LateStart"
           << std::setw(12) << "LateFinish"
           << std::setw(8) << "Slack"
           << '\n';
    applyColor(output, useColor, RESET_COLOR);
    output << separator << '\n';

    for (const auto& [id, task] : tasks)
    {
        const bool onCriticalPath = criticalIds.count(id) > 0;
        applyColor(output, useColor, onCriticalPath ? CRITICAL_COLOR : VALUE_COLOR);
        const std::string idLabel = taskLabel(task.id);
        output << std::left
               << std::setw(6) << idLabel
               << std::setw(8) << task.duration
               << std::setw(12) << task.ES
               << std::setw(12) << task.EF
               << std::setw(12) << task.LS
               << std::setw(12) << task.LF
               << std::setw(8) << task.slack
               << '\n';
        applyColor(output, useColor, RESET_COLOR);
    }

    output << separator << '\n';

    applyColor(output, useColor, SECTION_COLOR);
    output << "Critical path: ";
    applyColor(output, useColor, RESET_COLOR);

    for (std::size_t i = 0; i < result.criticalPath.size(); ++i)
    {
        applyColor(output, useColor, CRITICAL_COLOR);
        output << taskLabel(result.criticalPath[i]);
        applyColor(output, useColor, RESET_COLOR);
        if (i + 1 < result.criticalPath.size())
        {
            output << " -> ";
        }
    }
    output << '\n';

    if (!result.criticalPath.empty())
    {
        applyColor(output, useColor, SECTION_COLOR);
        output << "Timeline (ES - EF):" << '\n';
        applyColor(output, useColor, RESET_COLOR);
        for (int taskId : result.criticalPath)
        {
            const Task& task = tasks.at(taskId);
            applyColor(output, useColor, CRITICAL_COLOR);
            output << "  Task " << taskLabel(task.id) << ": ";
            applyColor(output, useColor, VALUE_COLOR);
            output << task.ES << " - " << task.EF << '\n';
            applyColor(output, useColor, RESET_COLOR);
        }
    }

    output << '\n';
}

void ResultPrinter::printPERT(const ProjectDataPert& projectData,
                              const PERTResult& result,
                              std::ostream& output)
{
    const bool useColor = streamSupportsColor(output);
    const auto originalFlags = output.flags();
    const auto originalPrecision = output.precision();

    applyColor(output, useColor, TITLE_COLOR);
    output << '\n' << "=== PERT Analysis Result ===" << '\n';
    applyColor(output, useColor, RESET_COLOR);

    applyColor(output, useColor, SECTION_COLOR);
    output << "Critical path (" << result.criticalPath.size() << " tasks): ";
    applyColor(output, useColor, RESET_COLOR);
    for (std::size_t i = 0; i < result.criticalPath.size(); ++i)
    {
        applyColor(output, useColor, CRITICAL_COLOR);
        output << taskLabel(result.criticalPath[i]);
        applyColor(output, useColor, RESET_COLOR);
        if (i + 1 < result.criticalPath.size())
        {
            output << " -> ";
        }
    }
    output << '\n';

    applyColor(output, useColor, SECTION_COLOR);
    output << "Schedule statistics:" << '\n';
    applyColor(output, useColor, RESET_COLOR);

    output.setf(std::ios::fixed, std::ios::floatfield);

    applyColor(output, useColor, LABEL_COLOR);
    output << "  Expected duration: ";
    applyColor(output, useColor, VALUE_COLOR);
    output << std::setprecision(1) << result.expectedDuration << '\n';

    applyColor(output, useColor, LABEL_COLOR);
    output << "  Standard deviation: ";
    applyColor(output, useColor, VALUE_COLOR);
    output << std::setprecision(2) << result.standardDeviation << '\n';

    applyColor(output, useColor, LABEL_COLOR);
    output << "  Target time: ";
    applyColor(output, useColor, VALUE_COLOR);
    output << std::setprecision(1) << projectData.target_time << '\n';

    double onTimeProbability = 0.0;
    if (result.standardDeviation > 0.0)
    {
        const double zScore = (projectData.target_time - result.expectedDuration) / result.standardDeviation;
        onTimeProbability = normalCDF(zScore);
    }
    else
    {
        onTimeProbability = projectData.target_time >= result.expectedDuration ? 1.0 : 0.0;
    }

    applyColor(output, useColor, LABEL_COLOR);
    output << "  Probability to meet target: ";
    applyColor(output, useColor, VALUE_COLOR);
    output << std::setprecision(4) << onTimeProbability;
    applyColor(output, useColor, RESET_COLOR);
    output << '\n';

    const double zForProbability = normalInvCDF(projectData.target_probability);
    const double timeForProbability = result.expectedDuration + result.standardDeviation * zForProbability;

    applyColor(output, useColor, LABEL_COLOR);
    output << "  Required time for target probability (" << std::setprecision(2) << projectData.target_probability * 100.0 << "%): ";
    applyColor(output, useColor, VALUE_COLOR);
    output << std::setprecision(2) << timeForProbability << '\n';

    applyColor(output, useColor, RESET_COLOR);
    output << '\n';

    output.flags(originalFlags);
    output.precision(originalPrecision);
}
