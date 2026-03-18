#ifndef RESUMECORE_COMMON_BENCHMARKROW_HPP
#define RESUMECORE_COMMON_BENCHMARKROW_HPP

#include "BenchmarkCore/Common/PricingResult.hpp"

#include <limits>
#include <string>

struct BenchmarkRow
{
    std::string caseName;
    PricingResult result;
    double absoluteError = std::numeric_limits<double>::quiet_NaN();
    double relativeError = std::numeric_limits<double>::quiet_NaN();
};

#endif