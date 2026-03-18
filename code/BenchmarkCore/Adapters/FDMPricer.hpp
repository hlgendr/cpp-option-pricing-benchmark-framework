#ifndef RESUMECORE_ADAPTERS_FDMPRICER_HPP
#define RESUMECORE_ADAPTERS_FDMPRICER_HPP

#include "BenchmarkCore/Common/MethodConfig.hpp"
#include "BenchmarkCore/Common/PricingResult.hpp"
#include "BenchmarkCore/Common/VanillaOptionSpec.hpp"

class FDMPricer
{
public:
    PricingResult Price(const VanillaOptionSpec& spec, const MethodConfig& config) const;
};

#endif