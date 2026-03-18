// TestPerpetualAmerican.cpp
//
// Archived exact reference driver for perpetual American option checks.
//
// (C) Datasim Education BV 2005-2011
//

#include <cmath>
#include <iostream>
#include <iomanip>
#include "PerpetualAmericanOption.hpp"

using namespace std;

void RunPerpetualReferenceTests()
{
    // =================== Perpetual American reference case ===================
    {
        // ---- 1) INPUT ----
        double K = 100.0;
        double S = 110.0;
        double sig = 0.10;
        double r = 0.10;
        double b = 0.02;   // reference carry setting

        // ---- 2) Price (direct closed-form) ----
        PerpetualAmericanOption pa;
        pa.K = K;
        pa.sig = sig;
        pa.r = r;
        pa.b = b;

        pa.optType = "P";
        double P = pa.Price(S);

        pa.optType = "C";
        double C = pa.Price(S);

        // ---- PRINT----
        std::cout << std::fixed << std::setprecision(6);
        std::cout << "================== PERPETUAL AMERICAN REFERENCE CASE ==================\n";
        std::cout << "INPUT: S=" << S
            << " K=" << K
            << " r=" << r
            << " sig=" << sig
            << " b=" << b << "\n";

        std::cout << "\n[Prices]\n";
        std::cout << "Put  (direct) = " << P << "\n";
        std::cout << "Call (direct) = " << C << "\n";
        std::cout << "=========================================================================\n\n";
    }
    return;
}

int main()
{
    RunPerpetualReferenceTests();
    return 0;
}