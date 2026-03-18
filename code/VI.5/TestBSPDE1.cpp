// TestBSPDE1.cpp
//
// Legacy standalone Black-Scholes PDE reference driver.
// Preserved for archived finite-difference experiments.
//
// (C) Datasim Education BV 2005-2011

#include "FdmDirector.hpp"

#include <iostream>
#include <string>
using namespace std;

#include "UtilitiesDJD/ExcelDriver/ExcelDriverLite.hpp"

namespace BS // Black Scholes
{
	double sig = 0.25;
	double K = 75.0;
	double T = 0.55;
	double r = 0.06;
	double D = 0.0; // aka q

	double mySigma (double x, double t)
	{

		double sigmaS = sig*sig;

		return 0.5 * sigmaS * x * x;
	}

	double myMu (double x, double t)
	{
		
		return (r - D) * x;
	
	}

	double myB (double x, double t)
	{	
	
		return  -r;
	}

	double myF (double x, double t)
	{
		return 0.0;
	}

	double myBCL(double t)
	{
		// Put boundary at S=0: V(0,t) = K*exp(-r*t)
		return K * exp(-r * t);
	}

	double myBCR(double t)
	{
		// Put boundary at S=Smax ~ 0
		return 0.0;
	}

	double myIC(double x)
	{
		// Put payoff
		return max(K - x, 0.0);
	}


}


int main()
{
	using namespace ParabolicIBVP;

	// Bind the Black-Scholes PDE coefficient functions.
	sigma = BS::mySigma;
	mu = BS::myMu;
	b = BS::myB;
	f = BS::myF;
	BCL = BS::myBCL;
	BCR = BS::myBCR;
	IC = BS::myIC;

	int J = static_cast<int>(5*BS::K); int N = 400000; // explicit Euler stability uses a fine time grid

	double Smax = 5*BS::K;			// heuristic spatial cutoff

	cout << "start FDM\n";
	FDMDirector fdir(Smax, BS::T, J, N);

	fdir.doit();

	// Sample the computed grid near S0.
	double S0 = 59.0;

	// find index closest to S0
	int idx = 0;
	double best = fabs(fdir.xarr[0] - S0);
	for (int i = 1; i < (int)fdir.xarr.size(); ++i)
	{
		double d = fabs(fdir.xarr[i] - S0);
		if (d < best) { best = d; idx = i; }
	}

	cout << "Closest S to " << S0 << " is " << fdir.xarr[idx]
		<< ", FDM price = " << fdir.current()[idx] << endl;

	cout << "Finished\n";

	// Optional Excel export for the archived demo workflow.
	printOneExcel(fdir.xarr, fdir.current(), string("Value"));

	return 0;
}
