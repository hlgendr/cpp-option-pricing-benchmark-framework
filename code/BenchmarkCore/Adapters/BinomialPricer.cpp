#include "BenchmarkCore/Adapters/BinomialPricer.hpp"

#include "VI.6/BinomialLatticeStrategy.hpp"
#include "VI.6/BinomialMethod.hpp"
#include "VI.6/Option.hpp"

#include <chrono>
#include <cmath>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <streambuf>

namespace
{
class NullBuffer : public std::streambuf
{
public:
    int overflow(int c) override
    {
        return c;
    }
};

class CoutSilencer
{
public:
    CoutSilencer()
        : original_(std::cout.rdbuf(&nullBuffer_))
    {
    }

    ~CoutSilencer()
    {
        std::cout.rdbuf(original_);
    }

private:
    NullBuffer nullBuffer_;
    std::streambuf* original_;
};

using StrategyDeleter = void(*)(BinomialLatticeStrategy*);
using StrategyPtr = std::unique_ptr<BinomialLatticeStrategy, StrategyDeleter>;

void DeleteCRR(BinomialLatticeStrategy* ptr) { delete static_cast<CRRStrategy*>(ptr); }
void DeleteJR(BinomialLatticeStrategy* ptr) { delete static_cast<JRStrategy*>(ptr); }
void DeleteTRG(BinomialLatticeStrategy* ptr) { delete static_cast<TRGStrategy*>(ptr); }
void DeleteEQP(BinomialLatticeStrategy* ptr) { delete static_cast<EQPStrategy*>(ptr); }
void DeleteModifiedCRR(BinomialLatticeStrategy* ptr) { delete static_cast<ModCRRStrategy*>(ptr); }
void DeletePadeJR(BinomialLatticeStrategy* ptr) { delete static_cast<PadeJRStrategy*>(ptr); }
void DeletePadeCRR(BinomialLatticeStrategy* ptr) { delete static_cast<PadeCRRStrategy*>(ptr); }

StrategyPtr CreateStrategy(const VanillaOptionSpec& spec, const MethodConfig& config, double deltaT)
{
    switch (config.binomialScheme)
    {
    case BinomialScheme::CRR:
        return StrategyPtr(new CRRStrategy(spec.volatility, spec.rate, deltaT), DeleteCRR);
    case BinomialScheme::JR:
        return StrategyPtr(new JRStrategy(spec.volatility, spec.rate, deltaT), DeleteJR);
    case BinomialScheme::TRG:
        return StrategyPtr(new TRGStrategy(spec.volatility, spec.rate, deltaT), DeleteTRG);
    case BinomialScheme::EQP:
        return StrategyPtr(new EQPStrategy(spec.volatility, spec.rate, deltaT), DeleteEQP);
    case BinomialScheme::ModifiedCRR:
        return StrategyPtr(new ModCRRStrategy(spec.volatility, spec.rate, deltaT, spec.spot, spec.strike, config.binomialSteps), DeleteModifiedCRR);
    case BinomialScheme::PadeJR:
        return StrategyPtr(new PadeJRStrategy(spec.volatility, spec.rate, deltaT), DeletePadeJR);
    case BinomialScheme::PadeCRR:
        return StrategyPtr(new PadeCRRStrategy(spec.volatility, spec.rate, deltaT), DeletePadeCRR);
    default:
        throw std::invalid_argument("Unsupported binomial scheme.");
    }
}

Vector<double, int> CalculatePayoffVector(const Vector<double, int>& nodes, const Option& option)
{
    Vector<double, int> payoff(nodes);
    for (int index = nodes.MinIndex(); index <= nodes.MaxIndex(); ++index)
    {
        payoff[index] = option.payoff(nodes[index]);
    }

    return payoff;
}

Vector<double, int> BuildTerminalStates(double spot, int steps, const BinomialLatticeStrategy& strategy, BinomialMethod& method)
{
    method.modifyLattice(spot);
    Vector<double, int> terminalStates = method.BasePyramidVector();

    if (strategy.binomialType() == Additive)
    {
        terminalStates[terminalStates.MinIndex()] = spot * std::exp(static_cast<double>(steps) * strategy.downValue());
        for (int index = terminalStates.MinIndex() + 1; index <= terminalStates.MaxIndex(); ++index)
        {
            terminalStates[index] = terminalStates[index - 1] * std::exp(strategy.upValue() - strategy.downValue());
        }
    }

    return terminalStates;
}
}

PricingResult BinomialPricer::Price(const VanillaOptionSpec& spec, const MethodConfig& config) const
{
    if (spec.spot <= 0.0 || spec.strike <= 0.0 || spec.maturity <= 0.0 || spec.volatility <= 0.0)
    {
        throw std::invalid_argument("Binomial adapter requires positive spot, strike, maturity, and volatility.");
    }

    if (std::fabs(spec.costOfCarry - spec.rate) > 1.0e-10)
    {
        throw std::invalid_argument("Binomial adapter V1 currently assumes stock options with b = r.");
    }

    if (config.binomialSteps < 1)
    {
        throw std::invalid_argument("Binomial steps must be at least 1.");
    }

    const auto start = std::chrono::steady_clock::now();

    const double deltaT = spec.maturity / static_cast<double>(config.binomialSteps);
    const double discount = std::exp(-spec.rate * deltaT);

    Option option;
    option.K = spec.strike;
    option.T = spec.maturity;
    option.r = spec.rate;
    option.sig = spec.volatility;
    option.type = (spec.type == VanillaOptionType::Call) ? 1 : 2;

    double price = 0.0;
    {
        CoutSilencer silencer;
        StrategyPtr strategy = CreateStrategy(spec, config, deltaT);
        BinomialMethod method(discount, *strategy, config.binomialSteps);
        const Vector<double, int> terminalStates = BuildTerminalStates(spec.spot, config.binomialSteps, *strategy, method);
        const Vector<double, int> payoff = CalculatePayoffVector(terminalStates, option);
        price = method.getPrice(payoff);
    }

    PricingResult result;
    result.method = "Binomial";
    result.optionType = ToString(spec.type);
    {
        std::ostringstream configStream;
        configStream << "scheme=" << ToString(config.binomialScheme)
                     << ";steps=" << config.binomialSteps;
        result.configSummary = configStream.str();
    }
    result.notes = "Legacy lattice core via adapter";
    result.price = price;

    const auto end = std::chrono::steady_clock::now();
    result.runtimeMs = std::chrono::duration<double, std::milli>(end - start).count();

    return result;
}