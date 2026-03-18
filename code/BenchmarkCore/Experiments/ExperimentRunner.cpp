#include "BenchmarkCore/Experiments/ExperimentRunner.hpp"

#include "BenchmarkCore/Adapters/BinomialPricer.hpp"
#include "BenchmarkCore/Adapters/ExactPricer.hpp"
#include "BenchmarkCore/Adapters/FDMPricer.hpp"
#include "BenchmarkCore/Adapters/MCPricer.hpp"

#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

namespace
{
PricingResult FailureResult(const std::string& methodName, const VanillaOptionSpec& spec, const std::string& notes)
{
    PricingResult result;
    result.method = methodName;
    result.optionType = ToString(spec.type);
    result.notes = notes;
    return result;
}

template <typename TPricer>
PricingResult RunSafely(const TPricer& pricer, const VanillaOptionSpec& spec, const MethodConfig& config, const std::string& methodName)
{
    try
    {
        return pricer.Price(spec, config);
    }
    catch (const std::exception& ex)
    {
        return FailureResult(methodName, spec, ex.what());
    }
}

double ComputeAbsoluteError(double observed, double exactPrice)
{
    if (std::isnan(observed) || std::isnan(exactPrice))
    {
        return std::numeric_limits<double>::quiet_NaN();
    }

    return std::fabs(observed - exactPrice);
}

double ComputeRelativeError(double absoluteError, double exactPrice)
{
    if (std::isnan(absoluteError) || std::isnan(exactPrice) || std::fabs(exactPrice) <= 1.0e-12)
    {
        return std::numeric_limits<double>::quiet_NaN();
    }

    return absoluteError / std::fabs(exactPrice);
}

BenchmarkRow MakeBenchmarkRow(const std::string& caseName, const PricingResult& result, double exactPrice)
{
    BenchmarkRow row;
    row.caseName = caseName;
    row.result = result;
    row.absoluteError = ComputeAbsoluteError(result.price, exactPrice);
    row.relativeError = ComputeRelativeError(row.absoluteError, exactPrice);
    return row;
}

std::string FormatDouble(double value, int precision = 6)
{
    if (std::isnan(value))
    {
        return "n/a";
    }

    std::ostringstream stream;
    stream << std::fixed << std::setprecision(precision) << value;
    return stream.str();
}

std::string FormatCsvDouble(double value)
{
    if (std::isnan(value))
    {
        return "";
    }

    std::ostringstream stream;
    stream << std::fixed << std::setprecision(6) << value;
    return stream.str();
}

std::string QuoteCsv(const std::string& value)
{
    std::string escaped = "\"";
    for (char ch : value)
    {
        if (ch == '\"')
        {
            escaped += "\"\"";
        }
        else
        {
            escaped += ch;
        }
    }
    escaped += "\"";
    return escaped;
}

std::vector<BenchmarkRow> FilterBenchmarkRows(const std::vector<BenchmarkRow>& rows, const std::string& caseName)
{
    std::vector<BenchmarkRow> filtered;
    for (const BenchmarkRow& row : rows)
    {
        if (row.caseName == caseName)
        {
            filtered.push_back(row);
        }
    }

    return filtered;
}
}

std::vector<BenchmarkRow> RunPrimaryBenchmark(const std::vector<BenchmarkCase>& benchmarkCases, const MethodConfig& config)
{
    ExactPricer exactPricer;
    BinomialPricer binomialPricer;
    MCPricer mcPricer;
    FDMPricer fdmPricer;

    std::vector<BenchmarkRow> allRows;

    for (const BenchmarkCase& benchmarkCase : benchmarkCases)
    {
        const PricingResult exact = RunSafely(exactPricer, benchmarkCase.spec, config, "Exact");
        const double exactPrice = exact.price;
        allRows.push_back(MakeBenchmarkRow(benchmarkCase.name, exact, exactPrice));

        const PricingResult binomial = RunSafely(binomialPricer, benchmarkCase.spec, config, "Binomial");
        allRows.push_back(MakeBenchmarkRow(benchmarkCase.name, binomial, exactPrice));

        const PricingResult mc = RunSafely(mcPricer, benchmarkCase.spec, config, "MC");
        allRows.push_back(MakeBenchmarkRow(benchmarkCase.name, mc, exactPrice));

        const PricingResult fdm = RunSafely(fdmPricer, benchmarkCase.spec, config, "FDM");
        allRows.push_back(MakeBenchmarkRow(benchmarkCase.name, fdm, exactPrice));
    }

    return allRows;
}

std::vector<BinomialConvergenceRow> RunBinomialConvergenceStudy(const std::vector<BenchmarkCase>& benchmarkCases, const MethodConfig& baseConfig, const std::vector<int>& stepCounts)
{
    ExactPricer exactPricer;
    BinomialPricer binomialPricer;
    std::vector<BinomialConvergenceRow> rows;

    for (const BenchmarkCase& benchmarkCase : benchmarkCases)
    {
        const double exactPrice = exactPricer.Price(benchmarkCase.spec, baseConfig).price;
        for (int steps : stepCounts)
        {
            MethodConfig config = baseConfig;
            config.binomialSteps = steps;

            const PricingResult result = RunSafely(binomialPricer, benchmarkCase.spec, config, "Binomial");
            BinomialConvergenceRow row;
            row.caseName = benchmarkCase.name;
            row.optionType = ToString(benchmarkCase.spec.type);
            row.steps = steps;
            row.exactPrice = exactPrice;
            row.result = result;
            row.absoluteError = ComputeAbsoluteError(result.price, exactPrice);
            row.relativeError = ComputeRelativeError(row.absoluteError, exactPrice);
            rows.push_back(row);
        }
    }

    return rows;
}

std::vector<MCVarianceStudyRow> RunMCVarianceStudy(
    const std::vector<BenchmarkCase>& benchmarkCases,
    const MethodConfig& baseConfig,
    const std::vector<long>& simulationCounts,
    const std::vector<MonteCarloVariant>& variants)
{
    ExactPricer exactPricer;
    MCPricer mcPricer;
    std::vector<MCVarianceStudyRow> rows;

    for (const BenchmarkCase& benchmarkCase : benchmarkCases)
    {
        const double exactPrice = exactPricer.Price(benchmarkCase.spec, baseConfig).price;

        for (MonteCarloVariant variant : variants)
        {
            for (long simulationCount : simulationCounts)
            {
                MethodConfig config = baseConfig;
                config.mcVariant = variant;
                config.mcSimulations = simulationCount;

                const PricingResult result = RunSafely(mcPricer, benchmarkCase.spec, config, "MC");
                MCVarianceStudyRow row;
                row.caseName = benchmarkCase.name;
                row.optionType = ToString(benchmarkCase.spec.type);
                row.variant = ToString(variant);
                row.simulations = simulationCount;
                row.timeSteps = config.mcTimeSteps;
                row.seed = config.mcSeed;
                row.sampleCount = result.sampleCount;
                row.pathCount = result.pathCount;
                row.exactPrice = exactPrice;
                row.result = result;
                row.absoluteError = ComputeAbsoluteError(result.price, exactPrice);
                row.relativeError = ComputeRelativeError(row.absoluteError, exactPrice);
                rows.push_back(row);
            }
        }
    }

    return rows;
}

std::vector<FDMGridStudyRow> RunFDMGridStudy(const std::vector<BenchmarkCase>& benchmarkCases, const MethodConfig& baseConfig, const std::vector<FDMGridPoint>& gridPoints)
{
    ExactPricer exactPricer;
    FDMPricer fdmPricer;
    std::vector<FDMGridStudyRow> rows;

    for (const BenchmarkCase& benchmarkCase : benchmarkCases)
    {
        const double exactPrice = exactPricer.Price(benchmarkCase.spec, baseConfig).price;
        for (const FDMGridPoint& gridPoint : gridPoints)
        {
            MethodConfig config = baseConfig;
            config.fdmSpaceSteps = gridPoint.spaceSteps;
            config.fdmTimeSteps = gridPoint.timeSteps;

            const PricingResult result = RunSafely(fdmPricer, benchmarkCase.spec, config, "FDM");
            FDMGridStudyRow row;
            row.caseName = benchmarkCase.name;
            row.optionType = ToString(benchmarkCase.spec.type);
            row.spaceSteps = gridPoint.spaceSteps;
            row.timeSteps = gridPoint.timeSteps;
            row.smaxMultiplier = config.fdmSmaxMultiplier;
            row.exactPrice = exactPrice;
            row.result = result;
            row.absoluteError = ComputeAbsoluteError(result.price, exactPrice);
            row.relativeError = ComputeRelativeError(row.absoluteError, exactPrice);
            rows.push_back(row);
        }
    }

    return rows;
}

std::filesystem::path EnsureOutputDirectory(const std::filesystem::path& executablePath)
{
    const std::filesystem::path outputDir = executablePath.parent_path().parent_path() / "output";
    std::filesystem::create_directories(outputDir);
    return outputDir;
}

void WritePrimaryBenchmarkCsv(const std::filesystem::path& csvPath, const std::vector<BenchmarkRow>& rows)
{
    std::ofstream output(csvPath);
    output << "case_name,method,option_type,price,runtime_ms,absolute_error,relative_error,standard_error,ci95_lower,ci95_upper,sample_count,path_count,config,notes\n";

    for (const BenchmarkRow& row : rows)
    {
        output << QuoteCsv(row.caseName) << ','
               << QuoteCsv(row.result.method) << ','
               << QuoteCsv(row.result.optionType) << ','
               << FormatCsvDouble(row.result.price) << ','
               << FormatCsvDouble(row.result.runtimeMs) << ','
               << FormatCsvDouble(row.absoluteError) << ','
               << FormatCsvDouble(row.relativeError) << ','
               << FormatCsvDouble(row.result.standardError) << ','
               << FormatCsvDouble(row.result.confidence95Lower) << ','
               << FormatCsvDouble(row.result.confidence95Upper) << ','
               << row.result.sampleCount << ','
               << row.result.pathCount << ','
               << QuoteCsv(row.result.configSummary) << ','
               << QuoteCsv(row.result.notes) << '\n';
    }
}

void WriteBinomialConvergenceCsv(const std::filesystem::path& csvPath, const std::vector<BinomialConvergenceRow>& rows)
{
    std::ofstream output(csvPath);
    output << "case_name,option_type,steps,exact_price,price,runtime_ms,absolute_error,relative_error,config,notes\n";

    for (const BinomialConvergenceRow& row : rows)
    {
        output << QuoteCsv(row.caseName) << ','
               << QuoteCsv(row.optionType) << ','
               << row.steps << ','
               << FormatCsvDouble(row.exactPrice) << ','
               << FormatCsvDouble(row.result.price) << ','
               << FormatCsvDouble(row.result.runtimeMs) << ','
               << FormatCsvDouble(row.absoluteError) << ','
               << FormatCsvDouble(row.relativeError) << ','
               << QuoteCsv(row.result.configSummary) << ','
               << QuoteCsv(row.result.notes) << '\n';
    }
}

void WriteMCVarianceStudyCsv(const std::filesystem::path& csvPath, const std::vector<MCVarianceStudyRow>& rows)
{
    std::ofstream output(csvPath);
    output << "case_name,option_type,variant,simulations,time_steps,seed,sample_count,path_count,exact_price,price,runtime_ms,absolute_error,relative_error,standard_error,ci95_lower,ci95_upper,config,notes\n";

    for (const MCVarianceStudyRow& row : rows)
    {
        output << QuoteCsv(row.caseName) << ','
               << QuoteCsv(row.optionType) << ','
               << QuoteCsv(row.variant) << ','
               << row.simulations << ','
               << row.timeSteps << ','
               << row.seed << ','
               << row.sampleCount << ','
               << row.pathCount << ','
               << FormatCsvDouble(row.exactPrice) << ','
               << FormatCsvDouble(row.result.price) << ','
               << FormatCsvDouble(row.result.runtimeMs) << ','
               << FormatCsvDouble(row.absoluteError) << ','
               << FormatCsvDouble(row.relativeError) << ','
               << FormatCsvDouble(row.result.standardError) << ','
               << FormatCsvDouble(row.result.confidence95Lower) << ','
               << FormatCsvDouble(row.result.confidence95Upper) << ','
               << QuoteCsv(row.result.configSummary) << ','
               << QuoteCsv(row.result.notes) << '\n';
    }
}

void WriteFDMGridStudyCsv(const std::filesystem::path& csvPath, const std::vector<FDMGridStudyRow>& rows)
{
    std::ofstream output(csvPath);
    output << "case_name,option_type,space_steps,time_steps,smax_multiplier,exact_price,price,runtime_ms,absolute_error,relative_error,config,notes\n";

    for (const FDMGridStudyRow& row : rows)
    {
        output << QuoteCsv(row.caseName) << ','
               << QuoteCsv(row.optionType) << ','
               << row.spaceSteps << ','
               << row.timeSteps << ','
               << FormatCsvDouble(row.smaxMultiplier) << ','
               << FormatCsvDouble(row.exactPrice) << ','
               << FormatCsvDouble(row.result.price) << ','
               << FormatCsvDouble(row.result.runtimeMs) << ','
               << FormatCsvDouble(row.absoluteError) << ','
               << FormatCsvDouble(row.relativeError) << ','
               << QuoteCsv(row.result.configSummary) << ','
               << QuoteCsv(row.result.notes) << '\n';
    }
}

void PrintPrimaryBenchmark(const std::vector<BenchmarkCase>& benchmarkCases, const std::vector<BenchmarkRow>& rows)
{
    std::cout << std::fixed << std::setprecision(6);
    for (const BenchmarkCase& benchmarkCase : benchmarkCases)
    {
        const std::vector<BenchmarkRow> caseRows = FilterBenchmarkRows(rows, benchmarkCase.name);
        std::cout << "Case: " << benchmarkCase.name
                  << " | " << ToString(benchmarkCase.spec.type)
                  << " | S=" << benchmarkCase.spec.spot
                  << " K=" << benchmarkCase.spec.strike
                  << " T=" << benchmarkCase.spec.maturity
                  << " r=" << benchmarkCase.spec.rate
                  << " sig=" << benchmarkCase.spec.volatility << "\n";
        std::cout << std::left
                  << std::setw(12) << "Method"
                  << std::setw(16) << "Price"
                  << std::setw(16) << "RuntimeMs"
                  << std::setw(16) << "AbsError"
                  << std::setw(16) << "RelError"
                  << std::setw(16) << "StdError"
                  << "Config" << "\n";

        for (const BenchmarkRow& row : caseRows)
        {
            std::cout << std::left
                      << std::setw(12) << row.result.method
                      << std::setw(16) << FormatDouble(row.result.price)
                      << std::setw(16) << FormatDouble(row.result.runtimeMs)
                      << std::setw(16) << FormatDouble(row.absoluteError)
                      << std::setw(16) << FormatDouble(row.relativeError)
                      << std::setw(16) << FormatDouble(row.result.standardError)
                      << row.result.configSummary << "\n";

            if (!std::isnan(row.result.confidence95Lower) && !std::isnan(row.result.confidence95Upper))
            {
                std::cout << "  95% CI: [" << FormatDouble(row.result.confidence95Lower)
                          << ", " << FormatDouble(row.result.confidence95Upper)
                          << "] using " << row.result.sampleCount
                          << " samples / " << row.result.pathCount << " paths\n";
            }

            if (!row.result.notes.empty())
            {
                std::cout << "  note: " << row.result.notes << "\n";
            }
        }

        std::cout << "\n";
    }
}

void PrintExperimentPackSummary(const ExperimentPack& pack)
{
    std::cout << "Config: binomial=" << ToString(pack.baselineConfig.binomialScheme) << "(" << pack.baselineConfig.binomialSteps << ")"
              << ", mcVariant=" << ToString(pack.baselineConfig.mcVariant)
              << ", mcSteps=" << pack.baselineConfig.mcTimeSteps
              << ", mcSims=" << pack.baselineConfig.mcSimulations
              << ", mcSeed=" << pack.baselineConfig.mcSeed
              << ", fdmJ=" << pack.baselineConfig.fdmSpaceSteps
              << ", fdmN=" << pack.baselineConfig.fdmTimeSteps
              << ", fdmSmaxMultiplier=" << pack.baselineConfig.fdmSmaxMultiplier << "\n";
    std::cout << "Case pack: " << pack.benchmarkCases.size() << " benchmark cases"
              << ", " << pack.binomialConvergenceCases.size() << " binomial study cases"
              << ", " << pack.mcVarianceCases.size() << " MC variance study cases"
              << ", " << pack.fdmGridStudyCases.size() << " FDM study cases\n\n";
}

void PrintBinomialConvergenceSummary(const std::vector<BinomialConvergenceRow>& rows)
{
    std::cout << "Binomial convergence summary\n";
    std::string currentCase;
    bool first = true;
    BinomialConvergenceRow firstRow{};
    BinomialConvergenceRow lastRow{};

    for (const BinomialConvergenceRow& row : rows)
    {
        if (first || row.caseName != currentCase)
        {
            if (!first)
            {
                std::cout << "  " << currentCase << ": steps " << firstRow.steps << " err=" << FormatDouble(firstRow.absoluteError)
                          << " -> steps " << lastRow.steps << " err=" << FormatDouble(lastRow.absoluteError) << "\n";
            }
            currentCase = row.caseName;
            firstRow = row;
            first = false;
        }
        lastRow = row;
    }

    if (!first)
    {
        std::cout << "  " << currentCase << ": steps " << firstRow.steps << " err=" << FormatDouble(firstRow.absoluteError)
                  << " -> steps " << lastRow.steps << " err=" << FormatDouble(lastRow.absoluteError) << "\n";
    }

    std::cout << "\n";
}

void PrintMCVarianceSummary(const std::vector<MCVarianceStudyRow>& rows)
{
    std::cout << "MC variance-reduction summary\n";

    std::string currentKey;
    bool first = true;
    MCVarianceStudyRow firstRow{};
    MCVarianceStudyRow lastRow{};

    for (const MCVarianceStudyRow& row : rows)
    {
        const std::string key = row.caseName + "|" + row.variant;
        if (first || key != currentKey)
        {
            if (!first)
            {
                std::cout << "  " << firstRow.caseName << " | " << firstRow.variant
                          << ": sims " << firstRow.simulations
                          << " err=" << FormatDouble(firstRow.absoluteError)
                          << ", se=" << FormatDouble(firstRow.result.standardError)
                          << " -> sims " << lastRow.simulations
                          << " err=" << FormatDouble(lastRow.absoluteError)
                          << ", se=" << FormatDouble(lastRow.result.standardError) << "\n";
            }

            currentKey = key;
            firstRow = row;
            first = false;
        }
        lastRow = row;
    }

    if (!first)
    {
        std::cout << "  " << firstRow.caseName << " | " << firstRow.variant
                  << ": sims " << firstRow.simulations
                  << " err=" << FormatDouble(firstRow.absoluteError)
                  << ", se=" << FormatDouble(firstRow.result.standardError)
                  << " -> sims " << lastRow.simulations
                  << " err=" << FormatDouble(lastRow.absoluteError)
                  << ", se=" << FormatDouble(lastRow.result.standardError) << "\n";
    }

    std::cout << "\n";
}

void PrintFDMGridStudySummary(const std::vector<FDMGridStudyRow>& rows)
{
    std::cout << "FDM grid/runtime summary\n";
    std::string currentCase;
    bool first = true;
    FDMGridStudyRow firstRow{};
    FDMGridStudyRow lastRow{};

    for (const FDMGridStudyRow& row : rows)
    {
        if (first || row.caseName != currentCase)
        {
            if (!first)
            {
                std::cout << "  " << currentCase << ": J=" << firstRow.spaceSteps << "/N=" << firstRow.timeSteps
                          << " err=" << FormatDouble(firstRow.absoluteError)
                          << " -> J=" << lastRow.spaceSteps << "/N=" << lastRow.timeSteps
                          << " err=" << FormatDouble(lastRow.absoluteError)
                          << ", runtime=" << FormatDouble(lastRow.result.runtimeMs) << " ms\n";
            }
            currentCase = row.caseName;
            firstRow = row;
            first = false;
        }
        lastRow = row;
    }

    if (!first)
    {
        std::cout << "  " << currentCase << ": J=" << firstRow.spaceSteps << "/N=" << firstRow.timeSteps
                  << " err=" << FormatDouble(firstRow.absoluteError)
                  << " -> J=" << lastRow.spaceSteps << "/N=" << lastRow.timeSteps
                  << " err=" << FormatDouble(lastRow.absoluteError)
                  << ", runtime=" << FormatDouble(lastRow.result.runtimeMs) << " ms\n";
    }

    std::cout << "\n";
}
