#ifndef PERT_PROBLEM_GENERATOR_H
#define PERT_PROBLEM_GENERATOR_H

#include "../Data_Loader/DataLoader_pert.h"

#include <cstdint>
#include <filesystem>
#include <optional>
#include <random>
#include <vector>

/**
 * @brief Utility for generating random PERT problems based on the original
 *        JavaScript demo logic.
 *
 * The generator reproduces the behaviour of the JS `Graph.SetRand` pipeline:
 *  - sample node positions inside a bounding box while enforcing minimum
 *    spacing,
 *  - create aesthetically pleasing edges (no crossings, wide angles, minimum
 *    distance from other nodes) directed from lower to higher Y,
 *  - assign optimistic/most-likely/pessimistic durations with the same
 *    swapping scheme,
 *  - compute the default target time/probability used by the demo.
 */
class PertProblemGenerator
{
public:
    struct Parameters
    {
        int nodeCount = 10;
        double xMax = 16.0;
        double yMax = 10.0;
        double minNodeDistance = 3.8;
        double minEdgeDistance = 2.0;
        double minAngleDegrees = 20.0;
        int maxDuration = 9;
    };

    /**
     * @brief Generate a single random problem.
     *
     * @param params generation parameters mirroring the JS defaults.
     * @param seed   optional seed for reproducible output (std::random_device
     *               used when no seed is supplied).
     */
    static ProjectDataPert generate(const Parameters& params,
                                    std::optional<std::uint32_t> seed = std::nullopt);

    static ProjectDataPert generate(std::optional<std::uint32_t> seed = std::nullopt)
    {
        return generate(Parameters{}, seed);
    }

    /**
     * @brief Generate a batch of random problems.
     *
     * @param count  number of instances to create.
     * @param params generator parameters.
     * @param seed   optional seed; when provided each instance advances the
     *               same generator sequence.
     */
    static std::vector<ProjectDataPert> generateMany(int count,
                                                     const Parameters& params,
                                                     std::optional<std::uint32_t> seed = std::nullopt);

    static std::vector<ProjectDataPert> generateMany(int count,
                                                     std::optional<std::uint32_t> seed = std::nullopt)
    {
        return generateMany(count, Parameters{}, seed);
    }

    /**
     * @brief Persist a problem in the same format that DataLoader_pert expects.
     */
    static void writeToFile(const ProjectDataPert& problem,
                            const std::filesystem::path& destination);

private:
    struct Node
    {
        double x = 0.0;
        double y = 0.0;
        int optimistic = 0;
        int mostLikely = 0;
        int pessimistic = 0;
    };

    struct Edge
    {
        int from = 0; // zero-based indices while generating
        int to = 0;
    };

    static std::vector<Node> generateNodes(const Parameters& params, std::mt19937& rng);
    static std::vector<Edge> generateEdges(const std::vector<Node>& nodes,
                                           const Parameters& params);
    static void assignDurations(std::vector<Node>& nodes, const Parameters& params, std::mt19937& rng);
    static ProjectDataPert buildProblem(const std::vector<Node>& nodes,
                                        const std::vector<Edge>& edges);

    static bool hasEnoughSpacing(const std::vector<Node>& nodes,
                                 double x,
                                 double y,
                                 double minDistance);
    static double distance(const Node& a, const Node& b);
    static double distancePointToSegment(const Node& a, const Node& b, const Node& c);
    static double crossProduct(const Node& a, const Node& b, const Node& c);
    static double angleCos(const Node& a, const Node& b, const Node& c);
    static bool segmentsIntersect(const Node& a, const Node& b, const Node& c, const Node& d);
};

#endif // PERT_PROBLEM_GENERATOR_H
