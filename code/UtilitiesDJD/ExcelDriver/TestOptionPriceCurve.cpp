// TestOptionPriceCurve.cpp
//
// Archived Excel demo that prices a Black-Scholes European call
// over a monotone spot mesh and writes the curve to Excel.
//
// (C) Datasim Education BV 2005-2017 (original framework)

#include "ExcelDriverLite.hpp"
#include "Utilities.hpp"
#include <cmath>
#include <vector>
#include <string>

// Standard normal CDF using erfc (stable and no extra libs)
static double N(double x)
{
    return 0.5 * std::erfc(-x / std::sqrt(2.0));
}

// Black¨CScholes European Call closed-form (exact)
static double BSCall(double S, double K, double r, double sig, double T)
{
    if (S <= 0.0 || K <= 0.0 || sig <= 0.0 || T <= 0.0) return 0.0;

    const double sqrtT = std::sqrt(T);
    const double d1 = (std::log(S / K) + (r + 0.5 * sig * sig) * T) / (sig * sqrtT);
    const double d2 = d1 - sig * sqrtT;

    return S * N(d1) - K * std::exp(-r * T) * N(d2);
}

int main()
{
    // Create a monotone spot mesh for S.
    // S = 10, 11, ..., 50 with step size 1.
    const double A = 10.0;
    const double B = 50.0;
    const long Nsteps = 40;            // (B - A) / Nsteps = 1
    auto Smesh = CreateMesh(Nsteps, A, B);

    // Choose a compact reference parameter set.
    const double K = 40.0;           // strike
    const double r = 0.05;           // risk-free rate
    const double sig = 0.20;           // volatility
    const double T = 1.0;            // maturity (years)

    // Compute the exact price for each spot level.
    auto priceFun = [=](double S) { return BSCall(S, K, r, sig, T); };
    auto PriceMesh = CreateDiscreteFunction<std::vector<double>>(Smesh, priceFun);

    // Export the curve to Excel.
    ExcelDriver xl;
    xl.MakeVisible(true);

    std::string title = "BS European Call Price vs S (K=40, r=0.05, sig=0.20, T=1.0)";
    xl.CreateChart(Smesh, PriceMesh, title);

    return 0;
}
