#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <iomanip>
#include <algorithm>
#include <numeric>
#include <cmath>

#include "dataGenerator/dataGenerator.h"
#include "Dijkstra/dijkstra_algorithm.h"
#include "Bellman-Ford/BellmanFord.h"
#include "aStar/aStar.h"

using Clock = std::chrono::high_resolution_clock;
using Microseconds = std::chrono::microseconds;

struct BenchConfig {
    std::vector<int> sizes{100, 250};
    std::vector<double> densities{0.25, 0.5};
    int repeats = 3;
    std::string generator = "euclidean"; // or "random"
    int maxWeight = 100; // used only when generator=="random"
    std::string output = "benchmark_results.csv";
    int startVertex = 0; // endVertex = n-1
    // aggregated output (if empty, will default to output with _agg.csv suffix)
    std::string aggregateOutput = "";
};

static void writeCsvHeaderIfNeeded(const std::string& filename) {
    namespace fs = std::filesystem;
    if (!fs::exists(filename)) {
        std::ofstream out(filename, std::ios::out);
        out << "algorithm,graph_size,density,generator,max_weight,run_id,start,end,execution_time_us,success,path_cost,path_length,edges,actual_density" << '\n';
    }
}

static void appendCsvRow(const std::string& filename,
                         const std::string& algorithm,
                         int n, double density,
                         const std::string& generator, int maxWeight,
                         int runId, int start, int end,
                         long long exec_us, bool success, long long cost, int path_len,
                         long long edges, double actual_density) {
    std::ofstream out(filename, std::ios::app);
    out << algorithm << ","
        << n << ","
        << std::fixed << std::setprecision(2) << density << ","
        << generator << ","
        << maxWeight << ","
        << runId << ","
        << start << ","
        << end << ","
        << exec_us << ","
        << (success ? 1 : 0) << ","
        << (success ? std::to_string(cost) : "") << ","
        << (success ? std::to_string(path_len) : "") << ","
        << edges << ","
        << std::setprecision(6) << actual_density
        << '\n';
}

static void writeAggHeaderIfNeeded(const std::string& filename) {
    namespace fs = std::filesystem;
    if (!fs::exists(filename)) {
        std::ofstream out(filename, std::ios::out);
        out << "algorithm,graph_size,density,generator,max_weight,runs,success_rate,mean_us,stddev_us,median_us,min_us,max_us,mean_cost,median_cost,mean_path_len,edges_mean,edges_std,actual_density_mean,matches_dijkstra_cost" << '\n';
    }
}

struct RunStats { long long us; bool ok; long long cost; int path_len; };

static double mean(const std::vector<long long>& v) {
    if (v.empty()) return 0.0;
    long long sum = std::accumulate(v.begin(), v.end(), 0LL);
    return double(sum) / double(v.size());
}
static double stdev(const std::vector<long long>& v, double m) {
    if (v.size() < 2) return 0.0;
    double acc = 0.0;
    for (auto x : v) { double d = double(x) - m; acc += d * d; }
    return std::sqrt(acc / double(v.size() - 1));
}
static long long median(std::vector<long long> v) {
    if (v.empty()) return 0;
    std::nth_element(v.begin(), v.begin() + v.size()/2, v.end());
    return v[v.size()/2];
}

static BenchConfig parseArgs(int argc, char* argv[]) {
    BenchConfig cfg;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        auto next = [&](int& i){ return (i + 1 < argc) ? std::string(argv[++i]) : std::string(); };

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
        } else if (arg == "--generator") {
            cfg.generator = next(i); // euclidean|random
        } else if (arg == "--max-weight") {
            cfg.maxWeight = std::stoi(next(i));
        } else if (arg == "--output") {
            cfg.output = next(i);
        } else if (arg == "--aggregate-output") {
            cfg.aggregateOutput = next(i);
        } else if (arg == "--start") {
            cfg.startVertex = std::stoi(next(i));
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: minPath_benchmark [--sizes 100,250,500] [--densities 0.25,0.5] [--repeats 5]\n"
                      << "       [--generator euclidean|random] [--max-weight 100] [--output raw.csv] [--aggregate-output agg.csv] [--start 0]\n";
            std::exit(0);
        }
    }
    return cfg;
}

int main(int argc, char* argv[]) {
    BenchConfig cfg = parseArgs(argc, argv);
    writeCsvHeaderIfNeeded(cfg.output);
    // decide aggregate output name
    if (cfg.aggregateOutput.empty()) {
        // derive from raw output
        auto pos = cfg.output.find_last_of('.');
        if (pos == std::string::npos) cfg.aggregateOutput = cfg.output + "_agg.csv";
        else cfg.aggregateOutput = cfg.output.substr(0, pos) + "_agg" + cfg.output.substr(pos);
    }
    writeAggHeaderIfNeeded(cfg.aggregateOutput);

    for (int n : cfg.sizes) {
        for (double density : cfg.densities) {
            // Per-(n,density) accumulators for aggregated stats
            std::vector<RunStats> djRuns, bfRuns, asRuns;
            std::vector<long long> edgesRuns;
            std::vector<long long> costsDijkstra; // for cross-check

            for (int run = 1; run <= cfg.repeats; ++run) {
                // Generate graph
                std::vector<std::vector<int>> matrix;
                std::vector<Point> coords;
                if (cfg.generator == "euclidean") {
                    matrix = generateAdjacencyMatrixWithCoordinates(n, density, coords);
                } else {
                    matrix = generateAdjacencyMatrix(n, density, cfg.maxWeight);
                    // fabricate coordinates for A*, but we'll disable heuristic in this mode
                    coords.resize(n, {0,0});
                }
                if (matrix.empty()) {
                    std::cerr << "Graph generation failed for n=" << n << " density=" << density << "\n";
                    continue;
                }

                // compute edges and actual density for this graph
                long long edges = 0;
                for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j) if (i != j && matrix[i][j] != INF) ++edges;
                double actual_density = n > 1 ? double(edges) / double(n * (long long)(n - 1)) : 0.0;
                edgesRuns.push_back(edges);

                int start = std::min(cfg.startVertex, n - 1);
                int end = n - 1;

                // Dijkstra
                {
                    Dijkstra dj(matrix);
                    auto t0 = Clock::now();
                    dj.findShortestPaths(start);
                    auto t1 = Clock::now();
                    auto us = std::chrono::duration_cast<Microseconds>(t1 - t0).count();
                    long long cost = dj.getShortestDistanceTo(end);
                    auto path = dj.getShortestPathTo(end);
                    bool ok = (cost != INF) && !path.empty();
                    appendCsvRow(cfg.output, "Dijkstra", n, density, cfg.generator, cfg.maxWeight, run, start, end, us, ok, ok ? cost : -1, ok ? (int)path.size() : 0, edges, actual_density);
                    djRuns.push_back({us, ok, ok ? cost : -1, ok ? (int)path.size() : 0});
                    if (ok) costsDijkstra.push_back(cost);
                }

                // Bellman-Ford
                {
                    BellmanFord bf(matrix);
                    auto t0 = Clock::now();
                    bool okCall = bf.findShortestPaths(start);
                    auto t1 = Clock::now();
                    auto us = std::chrono::duration_cast<Microseconds>(t1 - t0).count();
                    long long cost = bf.getShortestDistanceTo(end);
                    auto path = bf.getShortestPathTo(end);
                    bool ok = okCall && (cost != INF) && !path.empty();
                    appendCsvRow(cfg.output, "BellmanFord", n, density, cfg.generator, cfg.maxWeight, run, start, end, us, ok, ok ? cost : -1, ok ? (int)path.size() : 0, edges, actual_density);
                    bfRuns.push_back({us, ok, ok ? cost : -1, ok ? (int)path.size() : 0});
                }

                // A*
                {
                    // If generator is random (weights unrelated to geometry), disable heuristic to ensure optimality
                    bool useHeuristic = (cfg.generator == "euclidean");
                    AStar astar(matrix, coords, useHeuristic);
                    auto t0 = Clock::now();
                    bool okCall = astar.findShortestPath(start, end);
                    auto t1 = Clock::now();
                    auto us = std::chrono::duration_cast<Microseconds>(t1 - t0).count();
                    long long cost = okCall ? astar.getShortestDistance() : -1;
                    auto path = okCall ? astar.getShortestPath() : std::vector<int>{};
                    appendCsvRow(cfg.output, "AStar", n, density, cfg.generator, cfg.maxWeight, run, start, end, us, okCall, okCall ? cost : -1, okCall ? (int)path.size() : 0, edges, actual_density);
                    asRuns.push_back({us, okCall, okCall ? cost : -1, okCall ? (int)path.size() : 0});
                }
            }

            // aggregated stats per algorithm
            auto edgesMean = mean(edgesRuns);
            double edgesStd = 0.0; {
                if (edgesRuns.size() > 1) {
                    double m = edgesMean;
                    double acc = 0.0; for (auto e : edgesRuns) { double d = double(e) - m; acc += d*d; }
                    edgesStd = std::sqrt(acc / double(edgesRuns.size() - 1));
                }
            }
            double actualDensityMean = (n > 1) ? (edgesMean / double(n * (long long)(n - 1))) : 0.0;

            auto aggWrite = [&](const std::string& algo, const std::vector<RunStats>& runs, long long dijkstraMedianCost){
                std::vector<long long> times; times.reserve(runs.size());
                std::vector<long long> costs; costs.reserve(runs.size());
                std::vector<long long> lens; lens.reserve(runs.size());
                int okCount = 0;
                for (auto& r : runs) {
                    times.push_back(r.us);
                    if (r.ok) { ++okCount; costs.push_back(r.cost); lens.push_back(r.path_len); }
                }
                double meanUs = mean(times);
                double stdUs = stdev(times, meanUs);
                long long medUs = median(times);
                long long minUs = times.empty() ? 0 : *std::min_element(times.begin(), times.end());
                long long maxUs = times.empty() ? 0 : *std::max_element(times.begin(), times.end());
                double meanCost = costs.empty() ? 0.0 : (double)std::accumulate(costs.begin(), costs.end(), 0LL) / (double)costs.size();
                long long medCost = costs.empty() ? 0 : median(costs);
                double meanLen = lens.empty() ? 0.0 : (double)std::accumulate(lens.begin(), lens.end(), 0LL) / (double)lens.size();
                double successRate = runs.empty() ? 0.0 : double(okCount) / double(runs.size());
                int matchesDijkstra = (algo == "Dijkstra") ? 1 : ((dijkstraMedianCost > 0 && medCost == dijkstraMedianCost) ? 1 : 0);

                std::ofstream out(cfg.aggregateOutput, std::ios::app);
                out << algo << ","
                    << n << ","
                    << std::fixed << std::setprecision(2) << density << ","
                    << cfg.generator << ","
                    << cfg.maxWeight << ","
                    << runs.size() << ","
                    << std::setprecision(6) << successRate << ","
                    << std::setprecision(3) << meanUs << ","
                    << std::setprecision(3) << stdUs << ","
                    << medUs << ","
                    << minUs << ","
                    << maxUs << ","
                    << std::setprecision(3) << meanCost << ","
                    << medCost << ","
                    << std::setprecision(3) << meanLen << ","
                    << std::setprecision(3) << edgesMean << ","
                    << std::setprecision(3) << edgesStd << ","
                    << std::setprecision(6) << actualDensityMean << ","
                    << matchesDijkstra
                    << '\n';
            };

            long long djMedCost = 0; {
                std::vector<long long> djCosts; djCosts.reserve(djRuns.size());
                for (auto& r : djRuns) if (r.ok) djCosts.push_back(r.cost);
                djMedCost = djCosts.empty() ? 0 : median(djCosts);
            }

            aggWrite("Dijkstra", djRuns, djMedCost);
            aggWrite("BellmanFord", bfRuns, djMedCost);
            aggWrite("AStar", asRuns, djMedCost);
        }
    }

    std::cout << "Benchmark finished. Results appended to: " << cfg.output << "\n";
    std::cout << "Aggregates appended to: " << cfg.aggregateOutput << "\n";
    return 0;
}
