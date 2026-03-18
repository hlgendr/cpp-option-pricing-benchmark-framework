#ifndef RESUMECORE_ADAPTERS_EXACTPRICER_HPP
#define RESUMECORE_ADAPTERS_EXACTPRICER_HPP

#include "BenchmarkCore/Common/MethodConfig.hpp"
#include "BenchmarkCore/Common/PricingResult.hpp"
#include "BenchmarkCore/Common/VanillaOptionSpec.hpp"

class ExactPricer
{
public:
    PricingResult Price(const VanillaOptionSpec& spec, const MethodConfig& config) const;
};

#endif