#ifndef RESUMECORE_COMMON_METHODCONFIG_HPP
#define RESUMECORE_COMMON_METHODCONFIG_HPP

#include <string>

enum class BinomialScheme
{
    CRR,
    JR,
    TRG,
    EQP,
    ModifiedCRR,
    PadeJR,
    PadeCRR
};

enum class MonteCarloVariant
{
    Crude,
    Antithetic,
    ControlVariate
};

struct MethodConfig
{
    int binomialSteps = 400;
    BinomialScheme binomialScheme = BinomialScheme::CRR;
    long mcTimeSteps = 100;
    long mcSimulations = 30000;
    unsigned long mcSeed = 42UL;
    MonteCarloVariant mcVariant = MonteCarloVariant::ControlVariate;
    int fdmSpaceSteps = 120;
    long fdmTimeSteps = 80000;
    double fdmSmaxMultiplier = 4.0;
};

inline std::string ToString(BinomialScheme scheme)
{
    switch (scheme)
    {
    case BinomialScheme::CRR:
        return "CRR";
    case BinomialScheme::JR:
        return "JR";
    case BinomialScheme::TRG:
        return "TRG";
    case BinomialScheme::EQP:
        return "EQP";
    case BinomialScheme::ModifiedCRR:
        return "ModifiedCRR";
    case BinomialScheme::PadeJR:
        return "PadeJR";
    case BinomialScheme::PadeCRR:
        return "PadeCRR";
    default:
        return "Unknown";
    }
}

inline std::string ToString(MonteCarloVariant variant)
{
    switch (variant)
    {
    case MonteCarloVariant::Crude:
        return "Crude";
    case MonteCarloVariant::Antithetic:
        return "Antithetic";
    case MonteCarloVariant::ControlVariate:
        return "ControlVariate";
    default:
        return "Unknown";
    }
}

#endif
