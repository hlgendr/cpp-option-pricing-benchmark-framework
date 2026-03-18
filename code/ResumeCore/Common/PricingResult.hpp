#ifndef RESUMECORE_COMMON_PRICINGRESULT_HPP
#define RESUMECORE_COMMON_PRICINGRESULT_HPP

#include <limits>
#include <string>

struct PricingResult
{
    std::string method;
    std::string optionType;
    std::string configSummary;
    std::string notes;
    double price = std::numeric_limits<double>::quiet_NaN();
    double delta = std::numeric_limits<double>::quiet_NaN();
    double gamma = std::numeric_limits<double>::quiet_NaN();
    double standardError = std::numeric_limits<double>::quiet_NaN();
    double confidence95Lower = std::numeric_limits<double>::quiet_NaN();
    double confidence95Upper = std::numeric_limits<double>::quiet_NaN();
    long sampleCount = 0;
    long pathCount = 0;
    double runtimeMs = std::numeric_limits<double>::quiet_NaN();
};

#endif
