#include "ResumeCore/Adapters/ExactPricer.hpp"

#include "VI.3/PlainOption/EuropeanOption.hpp"

#include <chrono>
#include <stdexcept>
#include <string>

namespace
{
std::string ToEuropeanOptionType(VanillaOptionType type)
{
    return (type == VanillaOptionType::Call) ? "C" : "P";
}
}

PricingResult ExactPricer::Price(const VanillaOptionSpec& spec, const MethodConfig&) const
{
    if (spec.spot <= 0.0)
    {
        throw std::invalid_argument("Spot must be positive.");
    }

    if (spec.strike <= 0.0)
    {
        throw std::invalid_argument("Strike must be positive.");
    }

    if (spec.maturity <= 0.0)
    {
        throw std::invalid_argument("Maturity must be positive.");
    }

    if (spec.volatility <= 0.0)
    {
        throw std::invalid_argument("Volatility must be positive.");
    }

    const auto start = std::chrono::steady_clock::now();

    EuropeanOption option;
    option.K = spec.strike;
    option.T = spec.maturity;
    option.r = spec.rate;
    option.sig = spec.volatility;
    option.b = spec.costOfCarry;
    option.optType = ToEuropeanOptionType(spec.type);

    PricingResult result;
    result.method = "Exact";
    result.optionType = ToString(spec.type);
    result.configSummary = "model=BlackScholes;mode=closed_form";
    result.notes = "Legacy Black-Scholes closed-form core";
    result.price = option.Price(spec.spot);
    result.delta = option.Delta(spec.spot);
    result.gamma = option.Gamma(spec.spot);

    const auto end = std::chrono::steady_clock::now();
    result.runtimeMs = std::chrono::duration<double, std::milli>(end - start).count();

    return result;
}