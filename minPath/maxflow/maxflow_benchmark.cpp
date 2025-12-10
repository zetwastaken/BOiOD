#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

#include "Edmonds-Karp/EdmondsKarp.h"
#include "Ford-Fulkerson/FordFulkerson.h"
#include "../dataGenerator/dataGenerator.h"

using Clock = std::chrono::high_resolution_clock;
using Microseconds = std::chrono::microseconds;

struct BenchConfig {
    std::vector<int> sizes{25, 50, 100, 200, 400, 800};
    std::vector<double> densities{0.10, 0.30, 0.50};
    int repeats = 5;
    int maxCapacity = 100;
    std::string output = "maxflow/benchmark_maxflow.csv";
    std::string aggregateOutput = "";
    int source = 0; // sink = n-1
};

struct RunStats {
    long long us;
    long long flow;
};

static void writeCsvHeaderIfNeeded(const std::string& filename) {
    namespace fs = std::filesystem;
    if (!fs::exists(filename)) {
        std::ofstream out(filename, std::ios::out);
        out << "algorithm,graph_size,density,max_capacity,run_id,source,sink,execution_time_us,max_flow,edges,actual_density"
            << '\n';
    }
}

static void writeAggHeaderIfNeeded(const std::string& filename) {
    namespace fs = std::filesystem;
    if (!fs::exists(filename)) {
        std::ofstream out(filename, std::ios::out);
        out << "algorithm,graph_size,density,max_capacity,runs,mean_us,stddev_us,median_us,min_us,max_us,"
               "mean_flow,median_flow,edges_mean,edges_std,actual_density_mean"
            << '\n';
    }
}

static void appendCsvRow(const std::string& filename,
                         const std::string& algorithm,
                         int n,
                         double density,
                         int maxCapacity,
                         int runId,
                         int source,
                         int sink,
                         long long exec_us,
                         long long flow,
                         long long edges,
                         double actual_density) {
    std::ofstream out(filename, std::ios::app);
    out << algorithm << ","
        << n << ","
        << std::fixed << std::setprecision(2) << density << ","
        << maxCapacity << ","
        << runId << ","
        << source << ","
        << sink << ","
        << exec_us << ","
        << flow << ","
        << edges << ","
        << std::setprecision(6) << actual_density
        << '\n';
}

static double mean(const std::vector<long long>& v) {
    if (v.empty()) return 0.0;
    long long sum = std::accumulate(v.begin(), v.end(), 0LL);
    return static_cast<double>(sum) / static_cast<double>(v.size());
}

static double stdev(const std::vector<long long>& v, double m) {
    if (v.size() < 2) return 0.0;
    double acc = 0.0;
    for (auto x : v) {
        double d = static_cast<double>(x) - m;
        acc += d * d;
    }
    return std::sqrt(acc / static_cast<double>(v.size() - 1));
}

static long long median(std::vector<long long> v) {
    if (v.empty()) return 0;
    std::nth_element(v.begin(), v.begin() + v.size() / 2, v.end());
    return v[v.size() / 2];
}

static BenchConfig parseArgs(int argc, char* argv[]) {
    BenchConfig cfg;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        auto next = [&](int& idx) {
            return (idx + 1 < argc) ? std::string(argv[++idx]) : std::string();
        };

        if (arg == "--sizes") {
            std::string v = next(i);
            cfg.sizes.clear();
            std::stringstream ss(v);
            std::string tok;
            while (std::getline(ss, tok, ',')) cfg.sizes.push_back(std::stoi(tok));
        } else if (arg == "--densities") {
            std::string v = next(i);
            cfg.densities.clear();
            std::stringstream ss(v);
            std::string tok;
            while (std::getline(ss, tok, ',')) cfg.densities.push_back(std::stod(tok));
        } else if (arg == "--repeats") {
            cfg.repeats = std::stoi(next(i));
        } else if (arg == "--max-capacity") {
            cfg.maxCapacity = std::stoi(next(i));
        } else if (arg == "--output") {
            cfg.output = next(i);
        } else if (arg == "--aggregate-output") {
            cfg.aggregateOutput = next(i);
        } else if (arg == "--start") {
            cfg.source = std::stoi(next(i));
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: maxflow_benchmark [--sizes 25,50,100] [--densities 0.1,0.3] [--repeats 5]\n"
                         "       [--max-capacity 100] [--output raw.csv] [--aggregate-output agg.csv] [--start 0]\n";
            std::exit(0);
        }
    }
    return cfg;
}

int main(int argc, char* argv[]) {
    BenchConfig cfg = parseArgs(argc, argv);
    writeCsvHeaderIfNeeded(cfg.output);

    if (cfg.aggregateOutput.empty()) {
        auto pos = cfg.output.find_last_of('.');
        if (pos == std::string::npos) cfg.aggregateOutput = cfg.output + "_agg.csv";
        else cfg.aggregateOutput = cfg.output.substr(0, pos) + "_agg" + cfg.output.substr(pos);
    }
    writeAggHeaderIfNeeded(cfg.aggregateOutput);

    for (int n : cfg.sizes) {
        for (double density : cfg.densities) {
            std::cout << "Benchmarking n=" << n << " density=" << density << " (" << cfg.repeats << " runs)\n";

            std::vector<RunStats> ffRuns, ekRuns;
            std::vector<long long> edgesRuns;

            for (int run = 1; run <= cfg.repeats; ++run) {
                auto capacityMatrix = generateFlowNetwork(n, density, cfg.maxCapacity);
                if (capacityMatrix.empty()) {
                    std::cerr << "Graph generation failed for n=" << n << " density=" << density << '\n';
                    continue;
                }

                long long edges = 0;
                for (int i = 0; i < n; ++i) {
                    for (int j = 0; j < n; ++j) {
                        if (i != j && capacityMatrix[i][j] > 0) ++edges;
                    }
                }
                double actualDensity = (n > 1) ? static_cast<double>(edges) / static_cast<double>(n * (n - 1)) : 0.0;
                edgesRuns.push_back(edges);

                int source = std::min(cfg.source, n - 1);
                int sink = n - 1;

                // Ford–Fulkerson (DFS)
                {
                    FordFulkerson ff(capacityMatrix);
                    auto t0 = Clock::now();
                    int flow = ff.findMaxFlow(source, sink);
                    auto t1 = Clock::now();
                    auto us = std::chrono::duration_cast<Microseconds>(t1 - t0).count();
                    appendCsvRow(cfg.output, "FordFulkerson", n, density, cfg.maxCapacity, run, source, sink, us, flow, edges, actualDensity);
                    ffRuns.push_back({us, flow});
                }

                // Edmonds–Karp (BFS)
                {
                    EdmondsKarp ek(capacityMatrix);
                    auto t0 = Clock::now();
                    int flow = ek.findMaxFlow(source, sink);
                    auto t1 = Clock::now();
                    auto us = std::chrono::duration_cast<Microseconds>(t1 - t0).count();
                    appendCsvRow(cfg.output, "EdmondsKarp", n, density, cfg.maxCapacity, run, source, sink, us, flow, edges, actualDensity);
                    ekRuns.push_back({us, flow});
                }
            }

            auto edgesMean = mean(edgesRuns);
            double edgesStd = 0.0;
            if (edgesRuns.size() > 1) {
                double m = edgesMean;
                double acc = 0.0;
                for (auto e : edgesRuns) {
                    double d = static_cast<double>(e) - m;
                    acc += d * d;
                }
                edgesStd = std::sqrt(acc / static_cast<double>(edgesRuns.size() - 1));
            }
            double actualDensityMean = (n > 1) ? edgesMean / static_cast<double>(n * (n - 1)) : 0.0;

            auto aggWrite = [&](const std::string& algo, const std::vector<RunStats>& runs) {
                std::vector<long long> times;
                times.reserve(runs.size());
                std::vector<long long> flows;
                flows.reserve(runs.size());
                for (auto& r : runs) {
                    times.push_back(r.us);
                    flows.push_back(r.flow);
                }

                double meanUs = mean(times);
                double stdUs = stdev(times, meanUs);
                long long medUs = median(times);
                long long minUs = times.empty() ? 0 : *std::min_element(times.begin(), times.end());
                long long maxUs = times.empty() ? 0 : *std::max_element(times.begin(), times.end());
                double meanFlow = flows.empty() ? 0.0 : static_cast<double>(std::accumulate(flows.begin(), flows.end(), 0LL)) / static_cast<double>(flows.size());
                long long medFlow = flows.empty() ? 0 : median(flows);

                std::ofstream out(cfg.aggregateOutput, std::ios::app);
                out << algo << ","
                    << n << ","
                    << std::fixed << std::setprecision(2) << density << ","
                    << cfg.maxCapacity << ","
                    << runs.size() << ","
                    << std::setprecision(3) << meanUs << ","
                    << std::setprecision(3) << stdUs << ","
                    << medUs << ","
                    << minUs << ","
                    << maxUs << ","
                    << std::setprecision(3) << meanFlow << ","
                    << medFlow << ","
                    << std::setprecision(3) << edgesMean << ","
                    << std::setprecision(3) << edgesStd << ","
                    << std::setprecision(6) << actualDensityMean
                    << '\n';
            };

            aggWrite("FordFulkerson", ffRuns);
            aggWrite("EdmondsKarp", ekRuns);
        }
    }

    std::cout << "Benchmark finished. Results appended to: " << cfg.output << '\n';
    std::cout << "Aggregates appended to: " << cfg.aggregateOutput << '\n';
    return 0;
}
