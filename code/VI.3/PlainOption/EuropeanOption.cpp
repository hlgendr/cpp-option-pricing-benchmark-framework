// EurpeanOption.cpp
//
//	Author: Daniel Duffy
//
// (C) Datasim Component Technology BV 2003-2011
//


#include "EuropeanOption.hpp"
#include <cmath>
#include <iostream>
#include <boost/math/distributions/normal.hpp>


//////////// Gaussian functions /////////////////////////////////

// In general, we would use similar functions in Boost::Math Toolkit

double EuropeanOption::n(double x) const
{
	static const boost::math::normal_distribution<> nd(0.0, 1.0);
	return boost::math::pdf(nd, x);
}

double EuropeanOption::N(double x) const
{
	static const boost::math::normal_distribution<> nd(0.0, 1.0);
	return boost::math::cdf(nd, x);
}



// Kernel Functions (Haug)
double EuropeanOption::CallPrice(double U) const
{

	double tmp = sig * sqrt(T);

	double d1 = ( log(U/K) + (b+ (sig*sig)*0.5 ) * T )/ tmp;
	double d2 = d1 - tmp;


	return (U * exp((b-r)*T) * N(d1)) - (K * exp(-r * T)* N(d2));

}

double EuropeanOption::PutPrice(double U) const
{

	double tmp = sig * sqrt(T);
	double d1 = ( log(U/K) + (b+ (sig*sig)*0.5 ) * T )/ tmp;
	double d2 = d1 - tmp;

	return (K * exp(-r * T)* N(-d2)) - (U * exp((b-r)*T) * N(-d1));

}

double EuropeanOption::CallDelta(double U) const
{
	double tmp = sig * sqrt(T);

	double d1 = ( log(U/K) + (b+ (sig*sig)*0.5 ) * T )/ tmp;


	return exp((b-r)*T) * N(d1);
}

double EuropeanOption::PutDelta(double U) const
{
	double tmp = sig * sqrt(T);

	double d1 = ( log(U/K) + (b+ (sig*sig)*0.5 ) * T )/ tmp;

	return exp((b-r)*T) * (N(d1) - 1.0);
}

double EuropeanOption::CallGamma(double U) const
{
	double tmp = sig * sqrt(T);
	double d1 = (log(U / K) + (b + 0.5 * sig * sig) * T) / tmp;

	// Gamma same for call/put under BS-type model
	return exp((b - r) * T) * n(d1) / (U * tmp);
}

double EuropeanOption::PutGamma(double U) const
{
	// same as call gamma
	return CallGamma(U);
}

double EuropeanOption::Gamma(double U) const
{
	if (optType == "C")
		return CallGamma(U);
	else
		return PutGamma(U);
}

double EuropeanOption::DeltaApprox(double U, double h) const
{
	// Central difference: (V(U+h)-V(U-h)) / (2h)
	if (h <= 0.0) return 0.0;
	if (U - h <= 0.0) return (Price(U + h) - Price(U)) / h; // fallback forward diff

	return (Price(U + h) - Price(U - h)) / (2.0 * h);
}

double EuropeanOption::GammaApprox(double U, double h) const
{
	// Central second derivative: (V(U+h)-2V(U)+V(U-h)) / h^2
	if (h <= 0.0) return 0.0;
	if (U - h <= 0.0) return 0.0; // keep it simple & safe

	return (Price(U + h) - 2.0 * Price(U) + Price(U - h)) / (h * h);
}


/////////////////////////////////////////////////////////////////////////////////////

void EuropeanOption::init()
{	// Initialise all default values

	// Default values
	r = 0.05;
	sig= 0.2;

	K = 110.0;
	T = 0.5;

	b = r;			// Black and Scholes stock option model (1973)
	
	optType = "C";		// European Call Option (this is the default type)
}

void EuropeanOption::copy( const EuropeanOption& o2)
{

	r	= o2.r;
	sig = o2.sig;	
	K	= o2.K;
	T	= o2.T;
	b	= o2.b;
	
	optType = o2.optType;
	
}

EuropeanOption::EuropeanOption() 
{ // Default call option

	init();
}

EuropeanOption::EuropeanOption(const EuropeanOption& o2)
{ // Copy constructor

	copy(o2);
}

EuropeanOption::EuropeanOption (const string& optionType)
{	// Create option type

	init();
	optType = optionType;

	if (optType == "c")
		optType = "C";

}



EuropeanOption::~EuropeanOption()
{

}


EuropeanOption& EuropeanOption::operator = (const EuropeanOption& option2)
{

	if (this == &option2) return *this;

	copy (option2);

	return *this;
}

// Functions that calculate option price and sensitivities
double EuropeanOption::Price(double U) const
{


	if (optType == "C")
	{	
		//cout << "calling call\n";
		return CallPrice(U);
	}
	else
	{
		//cout << "calling put\n";
		return PutPrice(U);
	}
}	

double EuropeanOption::Delta(double U) const 
{
	if (optType == "C")
		return CallDelta(U);
	else
		return PutDelta(U);

}


// Modifier functions
void EuropeanOption::toggle()
{ // Change option type (C/P, P/C)

	if (optType == "C")
		optType = "P";
	else
		optType = "C";
}


