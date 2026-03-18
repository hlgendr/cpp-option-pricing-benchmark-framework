#include "ResumeCore/Experiments/ExperimentPack.hpp"
#include "ResumeCore/Experiments/ExperimentRunner.hpp"
#include "ResumeCore/Validation/ValidationHarness.hpp"

#include <cstdlib>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>

namespace
{
enum class RunMode
{
    Benchmark,
    MCVariance,
    Validation,
    All
};

void PrintUsage()
{
    std::cout << "Usage: OptionPricingBench [benchmark|mc_variance|validation|all]\n";
}

RunMode ParseMode(int argc, char* argv[])
{
    if (argc < 2)
    {
        return RunMode::All;
    }

    const std::string mode = argv[1];
    if (mode == "benchmark")
    {
        return RunMode::Benchmark;
    }
    if (mode == "mc_variance")
    {
        return RunMode::MCVariance;
    }
    if (mode == "validation")
    {
        return RunMode::Validation;
    }
    if (mode == "all")
    {
        return RunMode::All;
    }

    if (mode == "--help" || mode == "-h" || mode == "help")
    {
        PrintUsage();
        std::exit(0);
    }

    throw std::invalid_argument("Unsupported run mode: " + mode);
}

bool ShouldRunBenchmark(RunMode mode)
{
    return mode == RunMode::Benchmark || mode == RunMode::All;
}

bool ShouldRunMCVariance(RunMode mode)
{
    return mode == RunMode::MCVariance || mode == RunMode::All;
}

bool ShouldRunValidation(RunMode mode)
{
    return mode == RunMode::Validation || mode == RunMode::All;
}
}

int main(int argc, char* argv[])
{
    try
    {
        const RunMode mode = ParseMode(argc, argv);
        const std::filesystem::path executablePath = std::filesystem::absolute((argc > 0) ? std::filesystem::path(argv[0]) : std::filesystem::current_path());
        const std::filesystem::path outputDir = EnsureOutputDirectory(executablePath);
        const std::filesystem::path legacyMcCsv = outputDir / "mc_convergence.csv";
        if (std::filesystem::exists(legacyMcCsv))
        {
            std::filesystem::remove(legacyMcCsv);
        }

        const ExperimentPack pack = CreateStandardExperimentPack();

        std::cout << std::fixed << std::setprecision(6);
        std::cout << "OptionPricingBench V2\n";
        std::cout << "Mode: " << ((mode == RunMode::Benchmark) ? "benchmark"
                                   : (mode == RunMode::MCVariance) ? "mc_variance"
                                   : (mode == RunMode::Validation) ? "validation"
                                   : "all")
                  << "\n\n";
        PrintExperimentPackSummary(pack);

        if (ShouldRunBenchmark(mode))
        {
            const std::vector<BenchmarkRow> benchmarkRows = RunPrimaryBenchmark(pack.benchmarkCases, pack.baselineConfig);
            const std::vector<BinomialConvergenceRow> binomialRows = RunBinomialConvergenceStudy(pack.binomialConvergenceCases, pack.baselineConfig, pack.binomialConvergenceSteps);
            const std::vector<FDMGridStudyRow> fdmRows = RunFDMGridStudy(pack.fdmGridStudyCases, pack.baselineConfig, pack.fdmGridPoints);

            PrintPrimaryBenchmark(pack.benchmarkCases, benchmarkRows);
            PrintBinomialConvergenceSummary(binomialRows);
            PrintFDMGridStudySummary(fdmRows);

            WritePrimaryBenchmarkCsv(outputDir / "benchmark_results.csv", benchmarkRows);
            WriteBinomialConvergenceCsv(outputDir / "binomial_convergence.csv", binomialRows);
            WriteFDMGridStudyCsv(outputDir / "fdm_grid_study.csv", fdmRows);
        }

        if (ShouldRunMCVariance(mode))
        {
            const std::vector<MCVarianceStudyRow> mcVarianceRows = RunMCVarianceStudy(
                pack.mcVarianceCases,
                pack.baselineConfig,
                pack.mcVarianceSimulationCounts,
                pack.mcVarianceVariants);

            PrintMCVarianceSummary(mcVarianceRows);
            WriteMCVarianceStudyCsv(outputDir / "mc_variance_study.csv", mcVarianceRows);
        }

        if (ShouldRunValidation(mode))
        {
            const std::vector<ValidationCheckResult> validationRows = RunValidationSuite(pack);
            PrintValidationSummary(validationRows);
            WriteValidationSummaryCsv(outputDir / "validation_summary.csv", validationRows);
            WriteValidationSummaryMarkdown(outputDir / "validation_summary.md", validationRows);
        }

        std::cout << "Outputs written under: " << outputDir.filename().string() << "\n";
        return 0;
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Error: " << ex.what() << "\n";
        PrintUsage();
        return 1;
    }
}
