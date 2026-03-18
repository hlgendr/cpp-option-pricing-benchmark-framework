#ifndef RESUMECORE_EXPERIMENTS_EXPERIMENTRUNNER_HPP
#define RESUMECORE_EXPERIMENTS_EXPERIMENTRUNNER_HPP

#include "BenchmarkCore/Common/BenchmarkRow.hpp"
#include "BenchmarkCore/Experiments/ExperimentPack.hpp"

#include <filesystem>
#include <vector>

struct BinomialConvergenceRow
{
    std::string caseName;
    std::string optionType;
    int steps;
    double exactPrice;
    PricingResult result;
    double absoluteError;
    double relativeError;
};

struct MCVarianceStudyRow
{
    std::string caseName;
    std::string optionType;
    std::string variant;
    long simulations;
    long timeSteps;
    unsigned long seed;
    long sampleCount;
    long pathCount;
    double exactPrice;
    PricingResult result;
    double absoluteError;
    double relativeError;
};

struct FDMGridStudyRow
{
    std::string caseName;
    std::string optionType;
    int spaceSteps;
    long timeSteps;
    double smaxMultiplier;
    double exactPrice;
    PricingResult result;
    double absoluteError;
    double relativeError;
};

std::vector<BenchmarkRow> RunPrimaryBenchmark(const std::vector<BenchmarkCase>& benchmarkCases, const MethodConfig& config);
std::vector<BinomialConvergenceRow> RunBinomialConvergenceStudy(const std::vector<BenchmarkCase>& benchmarkCases, const MethodConfig& baseConfig, const std::vector<int>& stepCounts);
std::vector<FDMGridStudyRow> RunFDMGridStudy(const std::vector<BenchmarkCase>& benchmarkCases, const MethodConfig& baseConfig, const std::vector<FDMGridPoint>& gridPoints);
std::vector<MCVarianceStudyRow> RunMCVarianceStudy(
    const std::vector<BenchmarkCase>& benchmarkCases,
    const MethodConfig& baseConfig,
    const std::vector<long>& simulationCounts,
    const std::vector<MonteCarloVariant>& variants);

std::filesystem::path EnsureOutputDirectory(const std::filesystem::path& executablePath);
void WritePrimaryBenchmarkCsv(const std::filesystem::path& csvPath, const std::vector<BenchmarkRow>& rows);
void WriteBinomialConvergenceCsv(const std::filesystem::path& csvPath, const std::vector<BinomialConvergenceRow>& rows);
void WriteMCVarianceStudyCsv(const std::filesystem::path& csvPath, const std::vector<MCVarianceStudyRow>& rows);
void WriteFDMGridStudyCsv(const std::filesystem::path& csvPath, const std::vector<FDMGridStudyRow>& rows);

void PrintPrimaryBenchmark(const std::vector<BenchmarkCase>& benchmarkCases, const std::vector<BenchmarkRow>& rows);
void PrintExperimentPackSummary(const ExperimentPack& pack);
void PrintBinomialConvergenceSummary(const std::vector<BinomialConvergenceRow>& rows);
void PrintMCVarianceSummary(const std::vector<MCVarianceStudyRow>& rows);
void PrintFDMGridStudySummary(const std::vector<FDMGridStudyRow>& rows);

#endif
