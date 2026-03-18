#ifndef RESUMECORE_ADAPTERS_MCPRICER_HPP
#define RESUMECORE_ADAPTERS_MCPRICER_HPP

#include "BenchmarkCore/Common/MethodConfig.hpp"
#include "BenchmarkCore/Common/PricingResult.hpp"
#include "BenchmarkCore/Common/VanillaOptionSpec.hpp"

class MCPricer
{
public:
    PricingResult Price(const VanillaOptionSpec& spec, const MethodConfig& config) const;
};

#endif