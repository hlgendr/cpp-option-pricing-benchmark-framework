/* File: PerpetualAmericanOption.cpp
   Purpose: Implementation of perpetual American option pricing formulas.
   Author: Jiaqi Hu
   Notes: Closed-form solution; requires <cmath>. Avoid changing the formula unless instructed.
*/

#include "PerpetualAmericanOption.hpp"
#include <cmath>

PerpetualAmericanOption::PerpetualAmericanOption()
{
    // Sensible defaults
    K = 100.0;
    sig = 0.2;
    r = 0.05;
    b = 0.0;
    optType = "C";
}

PerpetualAmericanOption::PerpetualAmericanOption(const string& optionType)
    : PerpetualAmericanOption()
{
    // Normalise to uppercase "C"/"P"
    optType = optionType;
    if (optType == "c") optType = "C";
    if (optType == "p") optType = "P";
}

/*
  Perpetual American Call (closed-form)
  Uses exponent y1 derived from quadratic relation in the model
*/
double PerpetualAmericanOption::CallPrice(double S) const
{
    double sig2 = sig * sig;

    double fac = b / sig2 - 0.5;
    fac *= fac;

    double y1 = 0.5 - b / sig2 + std::sqrt(fac + 2.0 * r / sig2);

    // Guard: avoid division by zero in degenerate cases
    if (y1 == 1.0) return S;

    // Price formula
    double fac2 = ((y1 - 1.0) * S) / (y1 * K);
    double c = K * std::pow(fac2, y1) / (y1 - 1.0);
    return c;
}

/*
  Perpetual American Put (closed-form)
  Uses exponent y2 (the negative root).
*/
double PerpetualAmericanOption::PutPrice(double S) const
{
    double sig2 = sig * sig;

    double fac = b / sig2 - 0.5;
    fac *= fac;

    double y2 = 0.5 - b / sig2 - std::sqrt(fac + 2.0 * r / sig2);

    // Guard: avoid division by zero in degenerate cases
    if (y2 == 0.0) return S;

    double fac2 = ((y2 - 1.0) * S) / (y2 * K);
    double p = K * std::pow(fac2, y2) / (1.0 - y2);
    return p;
}

double PerpetualAmericanOption::Price(double S) const
{
    // Single entry point: decide by optType
    if (optType == "C") return CallPrice(S);
    return PutPrice(S);
}

void PerpetualAmericanOption::toggle()
{
    optType = (optType == "C" ? "P" : "C");
}
