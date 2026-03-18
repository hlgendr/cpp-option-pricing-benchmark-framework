#include "BenchmarkCore/Adapters/MCPricer.hpp"

#include "BenchmarkCore/Adapters/ExactPricer.hpp"
#include "UtilitiesDJD/RNG/NormalGenerator.hpp"
#include "VI.4/OptionData.hpp"

#include <chrono>
#include <cmath>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace
{
struct SampleSummary
{
    double mean = std::numeric_limits<double>::quiet_NaN();
    double standardError = std::numeric_limits<double>::quiet_NaN();
    double confidence95Lower = std::numeric_limits<double>::quiet_NaN();
    double confidence95Upper = std::numeric_limits<double>::quiet_NaN();
    long sampleCount = 0;
    long pathCount = 0;
};

struct EulerPathState
{
    double terminalSpot = 0.0;
    double brownianTerminal = 0.0;
};

double AdvanceEuler(double spot, double drift, double volatility, double dt, double sqrtDt, double normalShock)
{
    const double nextSpot = spot + (dt * drift * spot) + (sqrtDt * volatility * spot * normalShock);
    return (nextSpot > 0.0) ? nextSpot : 0.0;
}

EulerPathState SimulateEulerPath(const VanillaOptionSpec& spec, long timeSteps, BoostNormal& normalGenerator)
{
    const double dt = spec.maturity / static_cast<double>(timeSteps);
    const double sqrtDt = std::sqrt(dt);

    EulerPathState state;
    state.terminalSpot = spec.spot;

    for (long step = 0; step < timeSteps; ++step)
    {
        const double normalShock = normalGenerator.getNormal();
        state.brownianTerminal += sqrtDt * normalShock;
        state.terminalSpot = AdvanceEuler(state.terminalSpot, spec.costOfCarry, spec.volatility, dt, sqrtDt, normalShock);
    }

    return state;
}

SampleSummary SummarizeSamples(const std::vector<double>& samples, long pathCount)
{
    SampleSummary summary;
    summary.sampleCount = static_cast<long>(samples.size());
    summary.pathCount = pathCount;

    if (samples.empty())
    {
        return summary;
    }

    double sum = 0.0;
    for (double value : samples)
    {
        sum += value;
    }
    summary.mean = sum / static_cast<double>(samples.size());

    if (samples.size() == 1U)
    {
        summary.standardError = 0.0;
        summary.confidence95Lower = summary.mean;
        summary.confidence95Upper = summary.mean;
        return summary;
    }

    double squaredDeviationSum = 0.0;
    for (double value : samples)
    {
        const double deviation = value - summary.mean;
        squaredDeviationSum += deviation * deviation;
    }

    const double sampleVariance = squaredDeviationSum / static_cast<double>(samples.size() - 1U);
    summary.standardError = std::sqrt(sampleVariance / static_cast<double>(samples.size()));
    const double halfWidth95 = 1.96 * summary.standardError;
    summary.confidence95Lower = summary.mean - halfWidth95;
    summary.confidence95Upper = summary.mean + halfWidth95;

    return summary;
}

double EstimateControlVariateBeta(const std::vector<double>& targets, const std::vector<double>& controls)
{
    if (targets.size() != controls.size() || targets.size() < 2U)
    {
        return 0.0;
    }

    double targetMean = 0.0;
    double controlMean = 0.0;
    for (std::size_t index = 0; index < targets.size(); ++index)
    {
        targetMean += targets[index];
        controlMean += controls[index];
    }
    targetMean /= static_cast<double>(targets.size());
    controlMean /= static_cast<double>(controls.size());

    double covariance = 0.0;
    double controlVariance = 0.0;
    for (std::size_t index = 0; index < targets.size(); ++index)
    {
        const double centeredTarget = targets[index] - targetMean;
        const double centeredControl = controls[index] - controlMean;
        covariance += centeredTarget * centeredControl;
        controlVariance += centeredControl * centeredControl;
    }

    if (controlVariance <= 1.0e-14)
    {
        return 0.0;
    }

    return covariance / controlVariance;
}

double ExactLognormalTerminalSpot(const VanillaOptionSpec& spec, double brownianTerminal)
{
    return spec.spot * std::exp(((spec.costOfCarry - 0.5 * spec.volatility * spec.volatility) * spec.maturity)
                                + (spec.volatility * brownianTerminal));
}

std::string BuildMCConfigSummary(const MethodConfig& config, long sampleCount, long pathCount)
{
    std::ostringstream configStream;
    configStream << "scheme=Euler"
                 << ";variant=" << ToString(config.mcVariant)
                 << ";timeSteps=" << config.mcTimeSteps
                 << ";samples=" << sampleCount
                 << ";paths=" << pathCount
                 << ";seed=" << config.mcSeed;
    return configStream.str();
}
}

PricingResult MCPricer::Price(const VanillaOptionSpec& spec, const MethodConfig& config) const
{
    if (spec.spot <= 0.0 || spec.strike <= 0.0 || spec.maturity <= 0.0 || spec.volatility <= 0.0)
    {
        throw std::invalid_argument("MC adapter requires positive spot, strike, maturity, and volatility.");
    }

    if (config.mcTimeSteps < 1 || config.mcSimulations < 1)
    {
        throw std::invalid_argument("MC time steps and simulations must both be positive.");
    }

    const auto start = std::chrono::steady_clock::now();

    OptionData option;
    option.K = spec.strike;
    option.T = spec.maturity;
    option.r = spec.rate;
    option.sig = spec.volatility;
    option.type = (spec.type == VanillaOptionType::Call) ? 1 : -1;

    const double discount = std::exp(-spec.rate * spec.maturity);

    BoostNormal normalGenerator(config.mcSeed);
    std::vector<double> estimatorSamples;
    estimatorSamples.reserve(static_cast<std::size_t>(config.mcSimulations));

    long pathCount = config.mcSimulations;

    if (config.mcVariant == MonteCarloVariant::Crude)
    {
        for (long simulation = 0; simulation < config.mcSimulations; ++simulation)
        {
            const EulerPathState state = SimulateEulerPath(spec, config.mcTimeSteps, normalGenerator);
            estimatorSamples.push_back(discount * option.myPayOffFunction(state.terminalSpot));
        }
    }
    else if (config.mcVariant == MonteCarloVariant::Antithetic)
    {
        const double dt = spec.maturity / static_cast<double>(config.mcTimeSteps);
        const double sqrtDt = std::sqrt(dt);
        pathCount = config.mcSimulations * 2L;

        for (long simulation = 0; simulation < config.mcSimulations; ++simulation)
        {
            double plusSpot = spec.spot;
            double minusSpot = spec.spot;

            for (long step = 0; step < config.mcTimeSteps; ++step)
            {
                const double normalShock = normalGenerator.getNormal();
                plusSpot = AdvanceEuler(plusSpot, spec.costOfCarry, spec.volatility, dt, sqrtDt, normalShock);
                minusSpot = AdvanceEuler(minusSpot, spec.costOfCarry, spec.volatility, dt, sqrtDt, -normalShock);
            }

            const double pairedPayoff = 0.5 * (option.myPayOffFunction(plusSpot) + option.myPayOffFunction(minusSpot));
            estimatorSamples.push_back(discount * pairedPayoff);
        }
    }
    else
    {
        ExactPricer exactPricer;
        const double exactAnchorPrice = exactPricer.Price(spec, config).price;

        std::vector<double> targetSamples;
        std::vector<double> controlSamples;
        targetSamples.reserve(static_cast<std::size_t>(config.mcSimulations));
        controlSamples.reserve(static_cast<std::size_t>(config.mcSimulations));

        for (long simulation = 0; simulation < config.mcSimulations; ++simulation)
        {
            const EulerPathState state = SimulateEulerPath(spec, config.mcTimeSteps, normalGenerator);
            const double targetPayoff = discount * option.myPayOffFunction(state.terminalSpot);
            const double controlSpot = ExactLognormalTerminalSpot(spec, state.brownianTerminal);
            const double controlPayoff = discount * option.myPayOffFunction(controlSpot);

            targetSamples.push_back(targetPayoff);
            controlSamples.push_back(controlPayoff);
        }

        const double beta = EstimateControlVariateBeta(targetSamples, controlSamples);
        for (std::size_t index = 0; index < targetSamples.size(); ++index)
        {
            estimatorSamples.push_back(targetSamples[index] + (beta * (exactAnchorPrice - controlSamples[index])));
        }
    }

    const SampleSummary summary = SummarizeSamples(estimatorSamples, pathCount);

    PricingResult result;
    result.method = "MC";
    result.optionType = ToString(spec.type);
    result.configSummary = BuildMCConfigSummary(config, summary.sampleCount, summary.pathCount);
    result.notes = "Legacy Euler MC workflow upgraded with fixed-seed variance-reduction support";
    result.price = summary.mean;
    result.standardError = summary.standardError;
    result.confidence95Lower = summary.confidence95Lower;
    result.confidence95Upper = summary.confidence95Upper;
    result.sampleCount = summary.sampleCount;
    result.pathCount = summary.pathCount;

    const auto end = std::chrono::steady_clock::now();
    result.runtimeMs = std::chrono::duration<double, std::milli>(end - start).count();

    return result;
}
