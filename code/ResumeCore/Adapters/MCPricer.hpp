#ifndef RESUMECORE_ADAPTERS_MCPRICER_HPP
#define RESUMECORE_ADAPTERS_MCPRICER_HPP

#include "ResumeCore/Common/MethodConfig.hpp"
#include "ResumeCore/Common/PricingResult.hpp"
#include "ResumeCore/Common/VanillaOptionSpec.hpp"

class MCPricer
{
public:
    PricingResult Price(const VanillaOptionSpec& spec, const MethodConfig& config) const;
};

#endif