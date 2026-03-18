#ifndef RESUMECORE_COMMON_VANILLAOPTIONSPEC_HPP
#define RESUMECORE_COMMON_VANILLAOPTIONSPEC_HPP

#include <string>

enum class VanillaOptionType
{
    Call,
    Put
};

struct VanillaOptionSpec
{
    double spot;
    double strike;
    double maturity;
    double rate;
    double volatility;
    double costOfCarry;
    VanillaOptionType type;
};

inline std::string ToString(VanillaOptionType type)
{
    return (type == VanillaOptionType::Call) ? "Call" : "Put";
}

#endif