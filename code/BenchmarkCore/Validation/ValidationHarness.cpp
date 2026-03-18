#include "BenchmarkCore/Validation/ValidationHarness.hpp"

#include "BenchmarkCore/Adapters/BinomialPricer.hpp"
#include "BenchmarkCore/Adapters/ExactPricer.hpp"
#include "BenchmarkCore/Adapters/FDMPricer.hpp"
#include "BenchmarkCore/Adapters/MCPricer.hpp"
#include "BenchmarkCore/Experiments/ExperimentRunner.hpp"

#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace
{
const BenchmarkCase& RequireCase(const std::vector<BenchmarkCase>& cases, const std::string& name)
{
    for (const BenchmarkCase& benchmarkCase : cases)
    {
        if (benchmarkCase.name == name)
        {
            return benchmarkCase;
        }
    }

    throw std::invalid_argument("Validation case not found: " + name);
}

ValidationCheckResult MakeCheck(
    const std::string& suite,
    const std::string& checkName,
    bool passed,
    double observed,
    double expected,
    double tolerance,
    const std::string& notes)
{
    ValidationCheckResult result;
    result.suite = suite;
    result.checkName = checkName;
    result.passed = passed;
    result.observed = observed;
    result.expected = expected;
    result.tolerance = tolerance;
    result.notes = notes;
    return result;
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

std::string FormatCsvDouble(double value)
{
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(6) << value;
    return stream.str();
}
}

std::vector<ValidationCheckResult> RunValidationSuite(const ExperimentPack& pack)
{
    ExactPricer exactPricer;
    BinomialPricer binomialPricer;
    MCPricer mcPricer;
    FDMPricer fdmPricer;

    const BenchmarkCase& atmCall = RequireCase(pack.benchmarkCases, "atm_call_1y");
    const BenchmarkCase& atmPut = RequireCase(pack.benchmarkCases, "atm_put_1y");

    const PricingResult exactCallFirst = exactPricer.Price(atmCall.spec, pack.baselineConfig);
    const PricingResult exactCallSecond = exactPricer.Price(atmCall.spec, pack.baselineConfig);
    const PricingResult exactPut = exactPricer.Price(atmPut.spec, pack.baselineConfig);

    const double exactKnownCall = 10.450584;
    const double exactKnownPut = 5.573526;
    const double parityRightHandSide = (atmCall.spec.spot * std::exp((atmCall.spec.costOfCarry - atmCall.spec.rate) * atmCall.spec.maturity))
                                      - (atmCall.spec.strike * std::exp(-atmCall.spec.rate * atmCall.spec.maturity));
    const double parityObserved = exactCallFirst.price - exactPut.price;

    const PricingResult binomial = binomialPricer.Price(atmCall.spec, pack.baselineConfig);
    const PricingResult fdm = fdmPricer.Price(atmCall.spec, pack.baselineConfig);

    MethodConfig crudeConfig = pack.baselineConfig;
    crudeConfig.mcVariant = MonteCarloVariant::Crude;
    const PricingResult mcCrude = mcPricer.Price(atmCall.spec, crudeConfig);

    MethodConfig controlConfig = pack.baselineConfig;
    controlConfig.mcVariant = MonteCarloVariant::ControlVariate;
    const PricingResult mcControl = mcPricer.Price(atmCall.spec, controlConfig);

    const std::vector<BenchmarkRow> benchmarkRows = RunPrimaryBenchmark(pack.benchmarkCases, pack.baselineConfig);
    int invalidRowCount = 0;
    for (const BenchmarkRow& row : benchmarkRows)
    {
        if (std::isnan(row.result.price) || std::isnan(row.result.runtimeMs))
        {
            ++invalidRowCount;
        }
    }

    std::vector<ValidationCheckResult> rows;
    rows.push_back(MakeCheck(
        "exact",
        "exact_repeatability_atm_call_1y",
        std::fabs(exactCallFirst.price - exactCallSecond.price) <= 1.0e-12,
        std::fabs(exactCallFirst.price - exactCallSecond.price),
        0.0,
        1.0e-12,
        "Repricing the same exact case twice should be stable."));

    rows.push_back(MakeCheck(
        "exact",
        "exact_known_call_value_atm_call_1y",
        std::fabs(exactCallFirst.price - exactKnownCall) <= 1.0e-4,
        exactCallFirst.price,
        exactKnownCall,
        1.0e-4,
        "Regression anchor for the closed-form call price."));

    rows.push_back(MakeCheck(
        "exact",
        "exact_known_put_value_atm_put_1y",
        std::fabs(exactPut.price - exactKnownPut) <= 1.0e-4,
        exactPut.price,
        exactKnownPut,
        1.0e-4,
        "Regression anchor for the closed-form put price."));

    rows.push_back(MakeCheck(
        "parity",
        "put_call_parity_atm_1y",
        std::fabs(parityObserved - parityRightHandSide) <= 1.0e-6,
        parityObserved,
        parityRightHandSide,
        1.0e-6,
        "Exact call and put prices should satisfy Black-Scholes put-call parity."));

    rows.push_back(MakeCheck(
        "benchmark",
        "benchmark_row_count_and_finite_outputs",
        invalidRowCount == 0 && benchmarkRows.size() == (pack.benchmarkCases.size() * 4U),
        static_cast<double>(invalidRowCount),
        0.0,
        0.0,
        "Primary benchmark should emit 4 methods per case with finite price/runtime values."));

    rows.push_back(MakeCheck(
        "approximation",
        "binomial_accuracy_atm_call_1y",
        std::fabs(binomial.price - exactCallFirst.price) <= 0.02,
        std::fabs(binomial.price - exactCallFirst.price),
        0.0,
        0.02,
        "Baseline lattice price should stay within a practical tolerance of the exact price."));

    rows.push_back(MakeCheck(
        "approximation",
        "fdm_accuracy_atm_call_1y",
        std::fabs(fdm.price - exactCallFirst.price) <= 0.05,
        std::fabs(fdm.price - exactCallFirst.price),
        0.0,
        0.05,
        "Baseline explicit-Euler FDM price should stay within a practical tolerance."));

    rows.push_back(MakeCheck(
        "mc_v2",
        "mc_control_error_within_three_sigma_atm_call_1y",
        std::fabs(mcControl.price - exactCallFirst.price) <= (3.0 * mcControl.standardError),
        std::fabs(mcControl.price - exactCallFirst.price),
        0.0,
        3.0 * mcControl.standardError,
        "Control-variate MC error should stay within three standard errors on the ATM call anchor case."));

    rows.push_back(MakeCheck(
        "mc_v2",
        "mc_control_reduces_standard_error_vs_crude_atm_call_1y",
        mcControl.standardError < mcCrude.standardError,
        mcControl.standardError,
        mcCrude.standardError,
        0.0,
        "Control variate should reduce standard error relative to crude MC at the same sample budget."));

    return rows;
}

bool AllValidationChecksPassed(const std::vector<ValidationCheckResult>& rows)
{
    for (const ValidationCheckResult& row : rows)
    {
        if (!row.passed)
        {
            return false;
        }
    }

    return true;
}

void WriteValidationSummaryCsv(const std::filesystem::path& csvPath, const std::vector<ValidationCheckResult>& rows)
{
    std::ofstream output(csvPath);
    output << "suite,check_name,status,observed,expected,tolerance,notes\n";

    for (const ValidationCheckResult& row : rows)
    {
        output << QuoteCsv(row.suite) << ','
               << QuoteCsv(row.checkName) << ','
               << QuoteCsv(row.passed ? "PASS" : "FAIL") << ','
               << FormatCsvDouble(row.observed) << ','
               << FormatCsvDouble(row.expected) << ','
               << FormatCsvDouble(row.tolerance) << ','
               << QuoteCsv(row.notes) << '\n';
    }
}

void WriteValidationSummaryMarkdown(const std::filesystem::path& markdownPath, const std::vector<ValidationCheckResult>& rows)
{
    std::ofstream output(markdownPath);
    int passedCount = 0;
    for (const ValidationCheckResult& row : rows)
    {
        if (row.passed)
        {
            ++passedCount;
        }
    }

    output << "# Validation Summary\n\n";
    output << "- Passed: " << passedCount << "\n";
    output << "- Failed: " << (static_cast<int>(rows.size()) - passedCount) << "\n\n";
    output << "| Suite | Check | Status | Observed | Expected | Tolerance | Notes |\n";
    output << "| --- | --- | --- | ---: | ---: | ---: | --- |\n";

    for (const ValidationCheckResult& row : rows)
    {
        output << "| " << row.suite
               << " | " << row.checkName
               << " | " << (row.passed ? "PASS" : "FAIL")
               << " | " << FormatCsvDouble(row.observed)
               << " | " << FormatCsvDouble(row.expected)
               << " | " << FormatCsvDouble(row.tolerance)
               << " | " << row.notes << " |\n";
    }
}

void PrintValidationSummary(const std::vector<ValidationCheckResult>& rows)
{
    int passedCount = 0;
    for (const ValidationCheckResult& row : rows)
    {
        if (row.passed)
        {
            ++passedCount;
        }
    }

    std::cout << "Validation summary\n";
    for (const ValidationCheckResult& row : rows)
    {
        std::cout << "  [" << (row.passed ? "PASS" : "FAIL") << "] "
                  << row.checkName
                  << " | observed=" << FormatCsvDouble(row.observed)
                  << " expected=" << FormatCsvDouble(row.expected)
                  << " tolerance=" << FormatCsvDouble(row.tolerance) << "\n";
    }
    std::cout << "  Passed " << passedCount << " / " << rows.size() << " checks\n\n";
}
