#ifndef RESUMECORE_VALIDATION_VALIDATIONHARNESS_HPP
#define RESUMECORE_VALIDATION_VALIDATIONHARNESS_HPP

#include "BenchmarkCore/Experiments/ExperimentPack.hpp"

#include <filesystem>
#include <string>
#include <vector>

struct ValidationCheckResult
{
    std::string suite;
    std::string checkName;
    bool passed = false;
    double observed = 0.0;
    double expected = 0.0;
    double tolerance = 0.0;
    std::string notes;
};

std::vector<ValidationCheckResult> RunValidationSuite(const ExperimentPack& pack);
bool AllValidationChecksPassed(const std::vector<ValidationCheckResult>& rows);
void WriteValidationSummaryCsv(const std::filesystem::path& csvPath, const std::vector<ValidationCheckResult>& rows);
void WriteValidationSummaryMarkdown(const std::filesystem::path& markdownPath, const std::vector<ValidationCheckResult>& rows);
void PrintValidationSummary(const std::vector<ValidationCheckResult>& rows);

#endif
