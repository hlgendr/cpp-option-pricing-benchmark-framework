#include "BenchmarkCore/Experiments/ExperimentPack.hpp"

ExperimentPack CreateStandardExperimentPack()
{
    ExperimentPack pack;
    pack.baselineConfig.binomialSteps = 400;
    pack.baselineConfig.binomialScheme = BinomialScheme::CRR;
    pack.baselineConfig.mcTimeSteps = 100;
    pack.baselineConfig.mcSimulations = 30000;
    pack.baselineConfig.mcSeed = 42UL;
    pack.baselineConfig.mcVariant = MonteCarloVariant::ControlVariate;
    pack.baselineConfig.fdmSpaceSteps = 120;
    pack.baselineConfig.fdmTimeSteps = 80000;
    pack.baselineConfig.fdmSmaxMultiplier = 4.0;

    pack.benchmarkCases = {
        {"atm_call_1y", {100.0, 100.0, 1.0, 0.05, 0.20, 0.05, VanillaOptionType::Call}},
        {"atm_put_1y", {100.0, 100.0, 1.0, 0.05, 0.20, 0.05, VanillaOptionType::Put}},
        {"itm_call_2y", {120.0, 100.0, 2.0, 0.04, 0.18, 0.04, VanillaOptionType::Call}},
        {"itm_put_9m", {85.0, 100.0, 0.75, 0.02, 0.22, 0.02, VanillaOptionType::Put}},
        {"otm_call_18m", {100.0, 110.0, 1.5, 0.03, 0.25, 0.03, VanillaOptionType::Call}},
        {"otm_put_2y", {115.0, 100.0, 2.0, 0.04, 0.30, 0.04, VanillaOptionType::Put}}
    };

    pack.binomialConvergenceCases = {
        pack.benchmarkCases[0],
        pack.benchmarkCases[1]
    };
    pack.binomialConvergenceSteps = {25, 50, 100, 200, 400, 800, 1600};

    pack.mcVarianceCases = {
        pack.benchmarkCases[0],
        pack.benchmarkCases[1]
    };
    pack.mcVarianceSimulationCounts = {1000, 5000, 10000, 30000, 60000};
    pack.mcVarianceVariants = {
        MonteCarloVariant::Crude,
        MonteCarloVariant::Antithetic,
        MonteCarloVariant::ControlVariate
    };

    pack.fdmGridStudyCases = {
        pack.benchmarkCases[0]
    };
    pack.fdmGridPoints = {
        {40, 8000},
        {60, 18000},
        {80, 32000},
        {100, 50000},
        {120, 72000}
    };

    return pack;
}
