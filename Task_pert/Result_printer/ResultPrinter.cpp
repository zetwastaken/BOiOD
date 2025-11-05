#include "ResultPrinter.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

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

void printHistogram(const PERTSimulation& simulation,
                    std::ostream& output,
                    bool useColor)
{
    if (simulation.completionTimes.empty())
    {
        return;
    }

    const double minValue = simulation.minDuration;
    const double maxValue = simulation.maxDuration;
    const double range = maxValue - minValue;
    const double epsilon = std::numeric_limits<double>::epsilon() * std::max(1.0, std::abs(minValue));

    // Handle the degenerate case where every simulation finished at the same time.
    if (range <= epsilon)
    {
        applyColor(output, useColor, LABEL_COLOR);
        output << "  All simulations completed at approximately ";
        applyColor(output, useColor, VALUE_COLOR);
        output << std::setprecision(1) << minValue;
        applyColor(output, useColor, RESET_COLOR);
        output << '\n';
        return;
    }

    constexpr int kBinCount = 20;
    constexpr int kMaxBarWidth = 40;

    std::vector<int> counts(kBinCount, 0);
    for (double time : simulation.completionTimes)
    {
        const double normalized = (time - minValue) / range;
        int index = static_cast<int>(normalized * kBinCount);
        if (index < 0)
        {
            index = 0;
        }
        else if (index >= kBinCount)
        {
            index = kBinCount - 1;
        }
        ++counts[index];
    }

    const int maxCount = *std::max_element(counts.begin(), counts.end());
    if (maxCount == 0)
    {
        return;
    }

    int firstBin = 0;
    while (firstBin < kBinCount && counts[firstBin] == 0)
    {
        ++firstBin;
    }

    int lastBin = kBinCount - 1;
    while (lastBin > firstBin && counts[lastBin] == 0)
    {
        --lastBin;
    }

    output << std::setprecision(1);
    for (int i = firstBin; i <= lastBin; ++i)
    {
        const double binStart = minValue + (range * i) / kBinCount;
        const double binEnd = (i == kBinCount - 1) ? maxValue : minValue + (range * (i + 1)) / kBinCount;

        applyColor(output, useColor, LABEL_COLOR);
    output << "  [" << std::setw(7) << binStart << ", " << std::setw(7) << binEnd
               << (i == kBinCount - 1 ? "]" : ")");
        applyColor(output, useColor, VALUE_COLOR);
        output << " | ";

        int barLength = 0;
        if (counts[i] > 0)
        {
            barLength = static_cast<int>(std::round(static_cast<double>(counts[i]) / maxCount * kMaxBarWidth));
            if (barLength == 0)
            {
                barLength = 1;
            }
        }

        for (int j = 0; j < barLength; ++j)
        {
            output << '#';
        }

        if (barLength > 0)
        {
            output << ' ';
        }

        output << counts[i] << '\n';
        applyColor(output, useColor, RESET_COLOR);
    }
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

    if (!projectData.tasks.empty())
    {
        const std::unordered_set<int> criticalIds(result.criticalPath.begin(), result.criticalPath.end());
        const std::string separator(116, '-');

        auto formatDouble = [](double value, int precision) {
            std::ostringstream oss;
            oss.setf(std::ios::fixed);
            oss << std::setprecision(precision) << value;
            return oss.str();
        };

        applyColor(output, useColor, SECTION_COLOR);
        output << "Task schedule:" << '\n';
        applyColor(output, useColor, RESET_COLOR);
        output << separator << '\n';

        applyColor(output, useColor, HEADER_COLOR);
        output << std::left
               << std::setw(6) << "ID"
               << std::setw(6) << "a"
               << std::setw(6) << "m"
               << std::setw(6) << "b"
               << std::setw(12) << "Expected"
               << std::setw(12) << "StdDev"
               << std::setw(12) << "EarlyStart"
               << std::setw(12) << "EarlyFinish"
               << std::setw(12) << "LateStart"
               << std::setw(12) << "LateFinish"
               << std::setw(10) << "Slack"
               << '\n';
        applyColor(output, useColor, RESET_COLOR);
        output << separator << '\n';

        for (const auto& [id, task] : projectData.tasks)
        {
            const bool onCriticalPath = criticalIds.count(id) > 0;
            applyColor(output, useColor, onCriticalPath ? CRITICAL_COLOR : VALUE_COLOR);
            output << std::left
                   << std::setw(6) << taskLabel(task.id)
                   << std::setw(6) << task.optimistic_time
                   << std::setw(6) << task.most_likely_time
                   << std::setw(6) << task.pessimistic_time
                   << std::setw(12) << formatDouble(task.expected_duration, 2)
                   << std::setw(12) << formatDouble(std::sqrt(task.variance), 3)
                   << std::setw(12) << formatDouble(task.ES, 2)
                   << std::setw(12) << formatDouble(task.EF, 2)
                   << std::setw(12) << formatDouble(task.LS, 2)
                   << std::setw(12) << formatDouble(task.LF, 2)
                   << std::setw(10) << formatDouble(task.slack, 2)
                   << '\n';
            applyColor(output, useColor, RESET_COLOR);
        }

        output << separator << '\n';
        applyColor(output, useColor, RESET_COLOR);
        output << '\n';
    }

    output.flags(originalFlags);
    output.precision(originalPrecision);
}

void ResultPrinter::printSimulation(const PERTSimulation& result,
                                              double targetTime,
                                              double targetProbability,
                                              std::ostream& output)
{
    const bool useColor = streamSupportsColor(output);
    const auto originalFlags = output.flags();
    const auto originalPrecision = output.precision();

    applyColor(output, useColor, TITLE_COLOR);
    output << '\n' << "=== PERT Simulation Result ===" << '\n';
    applyColor(output, useColor, RESET_COLOR);

    applyColor(output, useColor, SECTION_COLOR);
    output << "Schedule statistics (based on " << result.simulations << " simulations):" << '\n';
    applyColor(output, useColor, RESET_COLOR);

    output.setf(std::ios::fixed, std::ios::floatfield);

    applyColor(output, useColor, LABEL_COLOR);
    output << "  Expected duration: ";
    applyColor(output, useColor, VALUE_COLOR);
    output << std::setprecision(1) << result.meanDuration << '\n';

    applyColor(output, useColor, LABEL_COLOR);
    output << "  Standard deviation: ";
    applyColor(output, useColor, VALUE_COLOR);
    output << std::setprecision(2) << result.standardDeviation << '\n';

    applyColor(output, useColor, LABEL_COLOR);
    output << "  Target time: ";
    applyColor(output, useColor, VALUE_COLOR);
    output << std::setprecision(1) << targetTime << '\n';

    // Calculate probability to meet target time
    double onTimeProbability = 0.0;
    for (double time : result.completionTimes)
    {
        if (time <= targetTime)
        {
            onTimeProbability += 1.0;
        }
    }
    onTimeProbability /= result.simulations;

    applyColor(output, useColor, LABEL_COLOR);
    output << "  Probability to meet target: ";
    applyColor(output, useColor, VALUE_COLOR);
    output << std::setprecision(4) << onTimeProbability;
    applyColor(output, useColor, RESET_COLOR);
    output << '\n';

    // Calculate required time for target probability
    const double timeForProbability = result.getPercentile(targetProbability * 100.0);

    applyColor(output, useColor, LABEL_COLOR);
    output << "  Required time for target probability (" << std::setprecision(2) << targetProbability * 100.0 << "%): ";
    applyColor(output, useColor, VALUE_COLOR);
    output << std::setprecision(2) << timeForProbability << '\n';

    applyColor(output, useColor, RESET_COLOR);
    output << '\n';

    if (!result.completionTimes.empty())
    {
        applyColor(output, useColor, SECTION_COLOR);
        output << "Duration histogram:" << '\n';
        applyColor(output, useColor, RESET_COLOR);
        printHistogram(result, output, useColor);
        output << '\n';
    }

    output.flags(originalFlags);
    output.precision(originalPrecision);
}

void ResultPrinter::printPERTSummary(const ProjectDataPert& projectData,
                                     const PERTResult& analytic,
                                     const PERTSimulation* simulation,
                                     std::ostream& output)
{
    const bool useColor = streamSupportsColor(output);
    const auto originalFlags = output.flags();
    const auto originalPrecision = output.precision();

    auto analyticProbability = [&]() {
        if (analytic.standardDeviation > 0.0)
        {
            const double zScore = (projectData.target_time - analytic.expectedDuration) / analytic.standardDeviation;
            return normalCDF(zScore);
        }
        return projectData.target_time >= analytic.expectedDuration ? 1.0 : 0.0;
    }();

    auto analyticRequired = [&]() {
        if (analytic.standardDeviation > 0.0)
        {
            const double z = normalInvCDF(projectData.target_probability);
            return analytic.expectedDuration + analytic.standardDeviation * z;
        }
        return analytic.expectedDuration;
    }();

    double simulationProbability = 0.0;
    double simulationRequired = 0.0;
    bool hasSimulation = simulation != nullptr && simulation->simulations > 0 && !simulation->completionTimes.empty();
    if (hasSimulation)
    {
        for (double time : simulation->completionTimes)
        {
            if (time <= projectData.target_time)
            {
                simulationProbability += 1.0;
            }
        }
        simulationProbability /= static_cast<double>(simulation->completionTimes.size());
        simulationRequired = simulation->getPercentile(projectData.target_probability * 100.0);
    }

    applyColor(output, useColor, SECTION_COLOR);
    output << "\nPERT summary table:\n";
    applyColor(output, useColor, RESET_COLOR);

    const std::string separator(88, '-');
    output << separator << '\n';

    output.setf(std::ios::fixed, std::ios::floatfield);
    output << std::left
           << std::setw(12) << "Variant"
           << std::right
           << std::setw(14) << "Expected"
           << std::setw(14) << "StdDev"
           << std::setw(14) << "Target"
           << std::setw(16) << "P(T<=target)"
           << std::setw(18) << "Time@targetProb"
           << '\n';
    output << separator << '\n';

    auto printRow = [&](const std::string& label,
                        double expected,
                        double stddev,
                        double probability,
                        double required) {
        applyColor(output, useColor, VALUE_COLOR);
        output << std::left << std::setw(12) << label;
        output << std::right
               << std::setw(14) << std::setprecision(3) << expected
               << std::setw(14) << std::setprecision(3) << stddev
               << std::setw(14) << std::setprecision(3) << projectData.target_time
               << std::setw(16) << std::setprecision(4) << probability
               << std::setw(18) << std::setprecision(3) << required
               << '\n';
        applyColor(output, useColor, RESET_COLOR);
    };

    printRow("Analytic",
             analytic.expectedDuration,
             analytic.standardDeviation,
             analyticProbability,
             analyticRequired);

    if (hasSimulation)
    {
        printRow("Simulation",
                 simulation->meanDuration,
                 simulation->standardDeviation,
                 simulationProbability,
                 simulationRequired);
    }
    else
    {
        applyColor(output, useColor, VALUE_COLOR);
        output << std::left << std::setw(12) << "Simulation"
               << std::right << std::setw(14) << "n/a"
               << std::setw(14) << "n/a"
               << std::setw(14) << std::setprecision(3) << projectData.target_time
               << std::setw(16) << "n/a"
               << std::setw(18) << "n/a"
               << '\n';
        applyColor(output, useColor, RESET_COLOR);
    }

    output << separator << '\n';
    output << '\n';

    output.flags(originalFlags);
    output.precision(originalPrecision);
}
