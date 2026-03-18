#ifndef RESUMECORE_ADAPTERS_EXACTPRICER_HPP
#define RESUMECORE_ADAPTERS_EXACTPRICER_HPP

#include "ResumeCore/Common/MethodConfig.hpp"
#include "ResumeCore/Common/PricingResult.hpp"
#include "ResumeCore/Common/VanillaOptionSpec.hpp"

class ExactPricer
{
public:
    PricingResult Price(const VanillaOptionSpec& spec, const MethodConfig& config) const;
};

#endif