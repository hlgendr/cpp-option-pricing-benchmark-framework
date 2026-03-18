// TestEuropeanOption.cpp
//
// Archived exact reference driver for European Black-Scholes validation cases.
//
// (C) Datasim Component Technology BV 2003-2011
//

#include "EuropeanOption.hpp"
#include <iostream>
#include <cmath>
#include <iomanip>


/* Cost of carry factor b must be included in formulae depending on the
   derivative type. These are used in the generalised Black-Scholes formula. 
   If r is the risk-free interest and q is the continuous dividend yield then 
   the cost-of-carry b per derivative type is:

	a) Black-Scholes (1973) stock option model: b = r
	b) b = r - q Merton (1973) stock option model with continuous dividend yield
	c) b = 0 Black (1976) futures option model
	d) b = r - rf Garman and Kohlhagen (1983) currency option model, where rf is the 
	   'foreign' interest rate
*/

void RunEuropeanReferenceTests()
{
	// =================== European reference case ===================
	{
		// ---- 1) INPUT ----
		double S = 102.0;      // Spot
		double K = 122.0;      // Strike
		double T = 1.65;      // Expiry (years)
		double sig = 0.43;    // Volatility
		double r = 0.045;      // Risk-free rate

		// Cost of carry b:
		// stock (no dividend): b = r
		// dividend yield q:    b = r - q
		// futures:             b = 0
		double b = 0;

		// ---- 2) Price (direct) ----
		EuropeanOption opt;
		opt.K = K; opt.T = T; opt.sig = sig; opt.r = r; opt.b = b;

		opt.optType = "C";
		double C = opt.Price(S);

		opt.optType = "P";
		double P = opt.Price(S);

		// ---- 3) Put-Call Parity (two directions + residual) ----
		// Parity: C - P = S*exp((b-r)T) - K*exp(-rT)
		auto PutFromCall = [](double C, double S, double K, double r, double b, double T)
			{
				return C - S * std::exp((b - r) * T) + K * std::exp(-r * T);
			};

		auto CallFromPut = [](double P, double S, double K, double r, double b, double T)
			{
				return P + S * std::exp((b - r) * T) - K * std::exp(-r * T);
			};

		auto ParityResidual = [](double C, double P, double S, double K, double r, double b, double T)
			{
				double lhs = C - P;
				double rhs = S * std::exp((b - r) * T) - K * std::exp(-r * T);
				return lhs - rhs; // should be ~0
			};

		double P_from_C = PutFromCall(C, S, K, r, b, T);
		double C_from_P = CallFromPut(P, S, K, r, b, T);
		double resid = ParityResidual(C, P, S, K, r, b, T);

		// ---- 4) Greeks (Exact Delta/Gamma) ----
		opt.optType = "C";
		double callDelta = opt.Delta(S);
		double callGamma = opt.Gamma(S);

		opt.optType = "P";
		double putDelta = opt.Delta(S);
		double putGamma = opt.Gamma(S);

		// ---- 5) Divided differences check for Delta/Gamma ----
		double h = 0.1;
		opt.optType = "C";
		double callDeltaApprox = opt.DeltaApprox(S, h);
		double callGammaApprox = opt.GammaApprox(S, h);

		// ---- PRINT----
		std::cout << std::fixed << std::setprecision(6);

		std::cout << "================== EUROPEAN REFERENCE CASE: European (BS) ==================\n";
		std::cout << "INPUT: S=" << S
			<< " K=" << K
			<< " T=" << T
			<< " r=" << r
			<< " sig=" << sig
			<< " b=" << b << "\n";

		std::cout << "\n[Prices]\n";
		std::cout << "Call (direct) = " << C << "\n";
		std::cout << "Put  (direct) = " << P << "\n";

		std::cout << "\n[Put-Call Parity]\n";
		std::cout << "Put  (from Call) = " << P_from_C << "    diff vs direct = " << (P_from_C - P) << "\n";
		std::cout << "Call (from Put ) = " << C_from_P << "    diff vs direct = " << (C_from_P - C) << "\n";
		std::cout << "Parity residual (should be ~0) = " << resid << "\n";

		std::cout << "\n[Greeks]\n";
		std::cout << "Call Delta = " << callDelta << "    Call Gamma = " << callGamma << "\n";
		std::cout << "Put  Delta = " << putDelta << "    Put  Gamma = " << putGamma << "\n";

		std::cout << "\n[Divided differences check] (optional)\n";
		std::cout << "h=" << h
			<< "  Call DeltaApprox=" << callDeltaApprox
			<< "  err=" << (callDeltaApprox - callDelta) << "\n";
		std::cout << "h=" << h
			<< "  Call GammaApprox=" << callGammaApprox
			<< "  err=" << (callGammaApprox - callGamma) << "\n";

		std::cout << "======================================================================\n\n";
	}
	return;
}

int main()
{
	RunEuropeanReferenceTests();
	return 0;
}