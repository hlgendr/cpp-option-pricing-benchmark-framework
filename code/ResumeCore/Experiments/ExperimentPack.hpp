#ifndef RESUMECORE_EXPERIMENTS_EXPERIMENTPACK_HPP
#define RESUMECORE_EXPERIMENTS_EXPERIMENTPACK_HPP

#include "ResumeCore/Common/BenchmarkCase.hpp"
#include "ResumeCore/Common/MethodConfig.hpp"

#include <vector>

struct FDMGridPoint
{
    int spaceSteps;
    long timeSteps;
};

struct ExperimentPack
{
    MethodConfig baselineConfig;
    std::vector<BenchmarkCase> benchmarkCases;
    std::vector<BenchmarkCase> binomialConvergenceCases;
    std::vector<int> binomialConvergenceSteps;
    std::vector<BenchmarkCase> mcVarianceCases;
    std::vector<long> mcVarianceSimulationCounts;
    std::vector<MonteCarloVariant> mcVarianceVariants;
    std::vector<BenchmarkCase> fdmGridStudyCases;
    std::vector<FDMGridPoint> fdmGridPoints;
};

ExperimentPack CreateStandardExperimentPack();

#endif
