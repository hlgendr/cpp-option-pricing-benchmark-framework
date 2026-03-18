#include "ResumeCore/Adapters/FDMPricer.hpp"

#include "VI.5/FDMDirector.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace
{
VanillaOptionSpec gSpec;
double gSmax = 0.0;

double Sigma(double x, double)
{
    return 0.5 * gSpec.volatility * gSpec.volatility * x * x;
}

double Mu(double x, double)
{
    return gSpec.costOfCarry * x;
}

double B(double, double)
{
    return -gSpec.rate;
}

double F(double, double)
{
    return 0.0;
}

double BoundaryLeft(double tau)
{
    if (gSpec.type == VanillaOptionType::Call)
    {
        return 0.0;
    }

    return gSpec.strike * std::exp(-gSpec.rate * tau);
}

double BoundaryRight(double tau)
{
    if (gSpec.type == VanillaOptionType::Call)
    {
        return (gSmax * std::exp((gSpec.costOfCarry - gSpec.rate) * tau)) - (gSpec.strike * std::exp(-gSpec.rate * tau));
    }

    return 0.0;
}

double InitialCondition(double x)
{
    if (gSpec.type == VanillaOptionType::Call)
    {
        return std::max(x - gSpec.strike, 0.0);
    }

    return std::max(gSpec.strike - x, 0.0);
}

double Interpolate(const std::vector<double>& grid, const std::vector<double>& values, double x)
{
    if (grid.empty() || values.empty())
    {
        throw std::invalid_argument("FDM interpolation requires a non-empty grid.");
    }

    if (x <= grid.front())
    {
        return values.front();
    }

    if (x >= grid.back())
    {
        return values.back();
    }

    for (std::size_t index = 1; index < grid.size(); ++index)
    {
        if (x <= grid[index])
        {
            const double x0 = grid[index - 1];
            const double x1 = grid[index];
            const double y0 = values[index - 1];
            const double y1 = values[index];
            const double weight = (x - x0) / (x1 - x0);
            return y0 + weight * (y1 - y0);
        }
    }

    return values.back();
}
}

PricingResult FDMPricer::Price(const VanillaOptionSpec& spec, const MethodConfig& config) const
{
    if (spec.spot <= 0.0 || spec.strike <= 0.0 || spec.maturity <= 0.0 || spec.volatility <= 0.0)
    {
        throw std::invalid_argument("FDM adapter requires positive spot, strike, maturity, and volatility.");
    }

    if (config.fdmSpaceSteps < 3 || config.fdmTimeSteps < 1 || config.fdmSmaxMultiplier <= 1.0)
    {
        throw std::invalid_argument("FDM configuration must have J >= 3, N >= 1, and SmaxMultiplier > 1.");
    }

    const auto start = std::chrono::steady_clock::now();

    gSpec = spec;
    gSmax = config.fdmSmaxMultiplier * std::max(spec.spot, spec.strike);

    using namespace ParabolicIBVP;
    sigma = Sigma;
    mu = Mu;
    b = B;
    f = F;
    BCL = BoundaryLeft;
    BCR = BoundaryRight;
    IC = InitialCondition;

    FDMDirector director(gSmax, spec.maturity, config.fdmSpaceSteps, config.fdmTimeSteps);
    director.doit();

    PricingResult result;
    result.method = "FDM";
    result.optionType = ToString(spec.type);
    {
        std::ostringstream configStream;
        configStream << "scheme=ExplicitEuler;spaceSteps=" << config.fdmSpaceSteps
                     << ";timeSteps=" << config.fdmTimeSteps
                     << ";smaxMultiplier=" << config.fdmSmaxMultiplier;
        result.configSummary = configStream.str();
    }
    result.notes = "Legacy explicit-Euler PDE core via adapter";
    result.price = Interpolate(director.xarr, director.current(), spec.spot);

    const auto end = std::chrono::steady_clock::now();
    result.runtimeMs = std::chrono::duration<double, std::milli>(end - start).count();

    return result;
}