// TestMC.cpp
//
// Legacy standalone Monte Carlo reference driver.
// Uses an explicit Euler stock path simulation and reports
// the discounted option price together with SD and SE estimates.
//
// (C) Datasim Education BC 2008-2011

#include "OptionData.hpp"
#include "UtilitiesDJD/RNG/NormalGenerator.hpp"
#include "UtilitiesDJD/Geometry/Range.cpp"
#include <cmath>
#include <iostream>
#include <vector>

// ===== MC reference utilities: generic SD/SE functions =====
double DiscountFactor(double r, double T)
{
    return std::exp(-r * T);
}

// Sample standard deviation (Bessel corrected): sqrt( 1/(M-1) * sum (xi - mean)^2 )
// Input: raw payoffs (undiscounted). We discount inside to match the estimator.
double StandardDeviation(const std::vector<double>& payoffs, double r, double T)
{
    const std::size_t M = payoffs.size();
    if (M < 2) return 0.0;

    const double df = DiscountFactor(r, T);

    // mean of discounted payoffs
    double sum = 0.0;
    for (std::size_t i = 0; i < M; ++i)
    {
        sum += df * payoffs[i];
    }
    const double mean = sum / static_cast<double>(M);

    // sample variance
    double sq = 0.0;
    for (std::size_t i = 0; i < M; ++i)
    {
        const double xi = df * payoffs[i];
        const double diff = xi - mean;
        sq += diff * diff;
    }

    const double var = sq / static_cast<double>(M - 1);
    return std::sqrt(var);
}

double StandardError(const std::vector<double>& payoffs, double r, double T)
{
    const std::size_t M = payoffs.size();
    if (M == 0) return 0.0;

    const double sd = StandardDeviation(payoffs, r, T);
    return sd / std::sqrt(static_cast<double>(M));
}

// ===== Existing helper =====
template <class T> void print(const std::vector<T>& myList)
{  // A generic print function for vectors

    std::cout << std::endl << "Size of vector is " << myList.size() << "\n[";

    // We must use a const iterator here, otherwise we get a compiler error.
    std::vector<T>::const_iterator i;
    for (i = myList.begin(); i != myList.end(); ++i)
    {
        std::cout << *i << ",";
    }

    std::cout << "]\n";
}

namespace SDEDefinition
{ // Defines drift + diffusion + data

    OptionData* data;				// The data for the option MC

    double drift(double t, double X)
    { // Drift term
        return (data->r) * X; // r - D
    }

    double diffusion(double t, double X)
    { // Diffusion term
        double betaCEV = 1.0;
        return data->sig * std::pow(X, betaCEV);
    }

    double diffusionDerivative(double t, double X)
    { // Diffusion term, needed for the Milstein method
        double betaCEV = 1.0;
        return 0.5 * (data->sig) * (betaCEV)*std::pow(X, 2.0 * betaCEV - 1.0);
    }

} // End of namespace


int main()
{
    std::cout << "1 factor MC with explicit Euler\n";

    OptionData myOption;
    myOption.K = 155.0;
    myOption.T = 1.25;
    myOption.r = 0.03;
    myOption.sig = 0.25;
    myOption.type = 1;	// Put -1, Call +1

    double S_0 = 150.0;

    long N = 100;
    std::cout << "Number of subintervals in time: ";
    std::cin >> N;

    // Create the basic SDE (Context class)
    Range<double> range(0.0, myOption.T);
    double VOld = S_0;
    double VNew;

    std::vector<double> x = range.mesh(N);

    long NSim = 50000;
    std::cout << "Number of simulations: ";
    std::cin >> NSim;

    const double k = myOption.T / double(N);
    const double sqrk = std::sqrt(k);

    // Normal random number
    double dW;

    // NormalGenerator is a base class
    NormalGenerator* myNormal = new BoostNormal();

    using namespace SDEDefinition;
    SDEDefinition::data = &myOption;

    int coun = 0; // Number of times S hits origin

    // ===== MC reference workflow: store raw payoffs for SD/SE =====
    std::vector<double> payoffs;
    payoffs.reserve(static_cast<std::size_t>(NSim));

    for (long i = 1; i <= NSim; ++i)
    {
        if ((i / 10000) * 10000 == i)
        {
            std::cout << i << std::endl;
        }

        VOld = S_0;
        for (unsigned long index = 1; index < x.size(); ++index)
        {
            // Create a random number
            dW = myNormal->getNormal();

            // The FDM (in this case explicit Euler)
            VNew = VOld + (k * drift(x[index - 1], VOld))
                + (sqrk * diffusion(x[index - 1], VOld) * dW);

            VOld = VNew;

            // Spurious values
            if (VNew <= 0.0) coun++;
        }

        const double tmp = myOption.myPayOffFunction(VNew);
        payoffs.push_back(tmp);
    }

    // Discount the average payoff to obtain the present value estimate.
    double price = 0.0;
    for (std::size_t i = 0; i < payoffs.size(); ++i)
    {
        price += payoffs[i];
    }
    price = (price / static_cast<double>(payoffs.size())) * std::exp(-myOption.r * myOption.T);

    // ===== MC reference outputs: SD and SE =====
    const double sd = StandardDeviation(payoffs, myOption.r, myOption.T);
    const double se = StandardError(payoffs, myOption.r, myOption.T);

    // Cleanup
    delete myNormal;

    std::cout << "Price, after discounting: " << price << std::endl;
    std::cout << "Standard deviation (discounted): " << sd << std::endl;
    std::cout << "Standard error: " << se << std::endl;
    std::cout << "Number of times origin is hit: " << coun << std::endl;

    return 0;
}
