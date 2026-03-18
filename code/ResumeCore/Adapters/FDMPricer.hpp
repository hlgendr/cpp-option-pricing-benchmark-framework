#ifndef RESUMECORE_ADAPTERS_FDMPRICER_HPP
#define RESUMECORE_ADAPTERS_FDMPRICER_HPP

#include "ResumeCore/Common/MethodConfig.hpp"
#include "ResumeCore/Common/PricingResult.hpp"
#include "ResumeCore/Common/VanillaOptionSpec.hpp"

class FDMPricer
{
public:
    PricingResult Price(const VanillaOptionSpec& spec, const MethodConfig& config) const;
};

#endif