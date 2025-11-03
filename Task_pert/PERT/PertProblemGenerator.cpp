#include "PertProblemGenerator.h"

#include "PERTCalculator.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <limits>
#include <map>
#include <numeric>
#include <random>
#include <stdexcept>

namespace
{
constexpr double kPi = 3.14159265358979323846;
constexpr int kMaxGenerationRetries = 32;

int toPercent(double probability)
{
    return static_cast<int>(std::lround(probability * 100.0));
}
}

ProjectDataPert PertProblemGenerator::generate(const Parameters& params,
                                               std::optional<std::uint32_t> seed)
{
    std::mt19937 rng(seed.has_value() ? *seed : static_cast<std::uint32_t>(std::random_device{}()));

    for (int attempt = 0; attempt < kMaxGenerationRetries; ++attempt)
    {
        auto nodes = generateNodes(params, rng);
        assignDurations(nodes, params, rng);
        auto edges = generateEdges(nodes, params);

        if (edges.empty())
        {
            continue;
        }

        ProjectDataPert problem = buildProblem(nodes, edges);
        if (problem.success)
        {
            return problem;
        }
    }

    return {};
}

std::vector<ProjectDataPert> PertProblemGenerator::generateMany(int count,
                                                                const Parameters& params,
                                                                std::optional<std::uint32_t> seed)
{
    std::vector<ProjectDataPert> problems;
    problems.reserve(static_cast<std::size_t>(count));

    std::mt19937 seeder(seed.has_value() ? *seed : static_cast<std::uint32_t>(std::random_device{}()));
    for (int i = 0; i < count; ++i)
    {
        ProjectDataPert problem = generate(params, seeder());
        if (!problem.success)
        {
            throw std::runtime_error("Failed to generate PERT problem instance");
        }
        problems.push_back(std::move(problem));
    }

    return problems;
}

void PertProblemGenerator::writeToFile(const ProjectDataPert& problem,
                                       const std::filesystem::path& destination)
{
    std::ofstream file(destination);
    if (!file)
    {
        throw std::runtime_error("Unable to open file for writing: " + destination.string());
    }

    file << problem.N << ' ' << problem.M << '\n';

    for (int id = 1; id <= problem.N; ++id)
    {
        const Task_pert& task = problem.tasks.at(id);
        file << task.optimistic_time << ' '
             << task.most_likely_time << ' '
             << task.pessimistic_time;
        if (id != problem.N)
        {
            file << "   ";
        }
    }
    file << '\n';

    bool first = true;
    for (int id = 1; id <= problem.N; ++id)
    {
        const Task_pert& task = problem.tasks.at(id);
        for (int succ : task.successors)
        {
            if (!first)
            {
                file << "   ";
            }
            file << id << ' ' << succ;
            first = false;
        }
    }
    file << '\n';

    file << static_cast<int>(std::llround(problem.target_time)) << ' '
         << toPercent(problem.target_probability) << '\n';
}

std::vector<PertProblemGenerator::Node> PertProblemGenerator::generateNodes(const Parameters& params,
                                                                            std::mt19937& rng)
{
    std::uniform_real_distribution<double> distX(0.0, params.xMax);
    std::uniform_real_distribution<double> distY(0.0, params.yMax);

    std::vector<Node> nodes;
    nodes.reserve(static_cast<std::size_t>(params.nodeCount));

    int attempts = 0;
    while (static_cast<int>(nodes.size()) < params.nodeCount)
    {
        const double x = std::round(distX(rng));
        const double y = std::round(distY(rng));

        if (hasEnoughSpacing(nodes, x, y, params.minNodeDistance))
        {
            nodes.push_back(Node{.x = x, .y = y});
        }

        if (++attempts > params.nodeCount * 4)
        {
            attempts = 0;
            nodes.clear();
        }
    }

    if (!nodes.empty())
    {
        std::size_t minXIdx = 0;
        std::size_t minYIdx = 0;
        std::size_t maxXIdx = 0;
        std::size_t maxYIdx = 0;

        for (std::size_t i = 0; i < nodes.size(); ++i)
        {
            if (nodes[i].x < nodes[minXIdx].x)
            {
                minXIdx = i;
            }
            if (nodes[i].y < nodes[minYIdx].y)
            {
                minYIdx = i;
            }
            if (nodes[i].x > nodes[maxXIdx].x)
            {
                maxXIdx = i;
            }
            if (nodes[i].y > nodes[maxYIdx].y)
            {
                maxYIdx = i;
            }
        }

        nodes[minXIdx].x = 0.0;
        nodes[minYIdx].y = 0.0;
        nodes[maxXIdx].x = params.xMax;
        nodes[maxYIdx].y = params.yMax;
    }

    return nodes;
}

std::vector<PertProblemGenerator::Edge> PertProblemGenerator::generateEdges(const std::vector<Node>& nodes,
                                                                            const Parameters& params)
{
    struct Candidate
    {
        int a = 0;
        int b = 0;
        double distance = 0.0;
    };

    std::vector<Candidate> candidates;
    candidates.reserve(nodes.size() * nodes.size());
    for (std::size_t a = 0; a < nodes.size(); ++a)
    {
        for (std::size_t b = a + 1; b < nodes.size(); ++b)
        {
            candidates.push_back(Candidate{
                .a = static_cast<int>(a),
                .b = static_cast<int>(b),
                .distance = distance(nodes[a], nodes[b])});
        }
    }

    std::sort(candidates.begin(), candidates.end(),
              [](const Candidate& lhs, const Candidate& rhs)
              {
                  return lhs.distance < rhs.distance;
              });

    const double angleThreshold = std::cos(params.minAngleDegrees * 2.0 * kPi / 360.0);

    std::vector<Edge> edges;
    edges.reserve(candidates.size());

    for (const Candidate& candidate : candidates)
    {
        const int idA = candidate.a;
        const int idB = candidate.b;

        bool ok = true;

        for (const Edge& edge : edges)
        {
            if (segmentsIntersect(nodes[edge.from], nodes[edge.to], nodes[idA], nodes[idB]))
            {
                ok = false;
                break;
            }
        }

        if (ok)
        {
            for (std::size_t nodeId = 0; nodeId < nodes.size(); ++nodeId)
            {
                if (nodeId == static_cast<std::size_t>(idA) || nodeId == static_cast<std::size_t>(idB))
                {
                    continue;
                }

                const double dist = distancePointToSegment(nodes[idA], nodes[idB], nodes[nodeId]);
                if (dist < params.minEdgeDistance)
                {
                    ok = false;
                    break;
                }
            }
        }

        if (ok)
        {
            for (const Edge& edge : edges)
            {
                const int A = edge.from;
                const int B = edge.to;

                if (A == idA && angleCos(nodes[B], nodes[A], nodes[idB]) > angleThreshold)
                {
                    ok = false;
                    break;
                }
                if (A == idB && angleCos(nodes[B], nodes[A], nodes[idA]) > angleThreshold)
                {
                    ok = false;
                    break;
                }
                if (B == idA && angleCos(nodes[A], nodes[B], nodes[idB]) > angleThreshold)
                {
                    ok = false;
                    break;
                }
                if (B == idB && angleCos(nodes[A], nodes[B], nodes[idA]) > angleThreshold)
                {
                    ok = false;
                    break;
                }
            }
        }

        if (!ok)
        {
            continue;
        }

        if (nodes[idA].y < nodes[idB].y)
        {
            edges.push_back(Edge{.from = idA, .to = idB});
        }
        else if (nodes[idA].y > nodes[idB].y)
        {
            edges.push_back(Edge{.from = idB, .to = idA});
        }
    }

    return edges;
}

void PertProblemGenerator::assignDurations(std::vector<Node>& nodes,
                                           const Parameters& params,
                                           std::mt19937& rng)
{
    std::uniform_int_distribution<int> dist(1, params.maxDuration);

    for (Node& node : nodes)
    {
        int pmin = dist(rng);
        int pmod = dist(rng);
        int pmax = dist(rng);

        if (pmin > pmod)
        {
            std::swap(pmin, pmod);
        }
        if (pmod > pmax)
        {
            std::swap(pmod, pmax);
        }
        if (pmin > pmod)
        {
            std::swap(pmin, pmod);
        }

        node.optimistic = pmin;
        node.mostLikely = pmod;
        node.pessimistic = pmax;
    }
}

ProjectDataPert PertProblemGenerator::buildProblem(const std::vector<Node>& nodes,
                                                   const std::vector<Edge>& edges)
{
    ProjectDataPert data;
    data.N = static_cast<int>(nodes.size());

    for (int idx = 0; idx < data.N; ++idx)
    {
        const Node& node = nodes[static_cast<std::size_t>(idx)];
        const int id = idx + 1;
        data.tasks.emplace(id, Task_pert(id, node.optimistic, node.mostLikely, node.pessimistic));
    }

    int edgeCount = 0;
    for (const Edge& edge : edges)
    {
        const int fromId = edge.from + 1;
        const int toId = edge.to + 1;
        if (fromId == toId)
        {
            continue;
        }
        data.tasks.at(fromId).successors.push_back(toId);
        data.tasks.at(toId).predecessors.push_back(fromId);
        ++edgeCount;
    }
    data.M = edgeCount;

    std::map<int, Task_pert> tasksCopy = data.tasks;
    PERTResult pertResult = PERTCalculator::analyze(tasksCopy);

    if (pertResult.expectedDuration <= 0.0)
    {
        return {};
    }

    const double expectedDuration = pertResult.expectedDuration;
    const double targetTime = std::floor(expectedDuration - 0.5);
    constexpr double kTargetProbability = 0.99;

    data.target_time = std::max(0.0, targetTime);
    data.target_probability = kTargetProbability;
    data.success = true;

    return data;
}

bool PertProblemGenerator::hasEnoughSpacing(const std::vector<Node>& nodes,
                                            double x,
                                            double y,
                                            double minDistance)
{
    for (const Node& node : nodes)
    {
        const double dx = x - node.x;
        const double dy = y - node.y;
        if (dx * dx + dy * dy < minDistance * minDistance)
        {
            return false;
        }
    }
    return true;
}

double PertProblemGenerator::distance(const Node& a, const Node& b)
{
    const double dx = a.x - b.x;
    const double dy = a.y - b.y;
    return std::sqrt(dx * dx + dy * dy);
}

double PertProblemGenerator::distancePointToSegment(const Node& a, const Node& b, const Node& c)
{
    const double ab = distance(a, b);
    if (ab == 0.0)
    {
        return distance(c, a);
    }

    const double cosABC = angleCos(c, b, a);
    if (cosABC <= 0.0)
    {
        return distance(c, b);
    }

    const double cosBAC = angleCos(c, a, b);
    if (cosBAC <= 0.0)
    {
        return distance(c, a);
    }

    const double cp = crossProduct(a, b, c);
    return std::fabs(cp) / ab;
}

double PertProblemGenerator::crossProduct(const Node& a, const Node& b, const Node& c)
{
    const double ca0 = c.x - a.x;
    const double ca1 = c.y - a.y;
    const double ba0 = b.x - a.x;
    const double ba1 = b.y - a.y;
    return ca0 * ba1 - ba0 * ca1;
}

double PertProblemGenerator::angleCos(const Node& a, const Node& b, const Node& c)
{
    const double ux = a.x - b.x;
    const double uy = a.y - b.y;
    const double vx = c.x - b.x;
    const double vy = c.y - b.y;

    const double uLen = std::sqrt(ux * ux + uy * uy);
    const double vLen = std::sqrt(vx * vx + vy * vy);
    if (uLen == 0.0 || vLen == 0.0)
    {
        return 1.0;
    }

    double cosValue = (ux * vx + uy * vy) / (uLen * vLen);
    cosValue = std::clamp(cosValue, -1.0, 1.0);
    return cosValue;
}

bool PertProblemGenerator::segmentsIntersect(const Node& a,
                                             const Node& b,
                                             const Node& c,
                                             const Node& d)
{
    const double v1 = crossProduct(c, d, a);
    const double v2 = crossProduct(c, d, b);
    const double v3 = crossProduct(a, b, c);
    const double v4 = crossProduct(a, b, d);

    return (v1 * v2 < 0.0) && (v3 * v4 < 0.0);
}

