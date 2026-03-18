#ifndef PerpetualAmericanOption_hpp
#define PerpetualAmericanOption_hpp

#include <string>
using namespace std;

// Perpetual American option
// b is cost of carry (b = r - q for dividend-paying stock)

class PerpetualAmericanOption
{
private:
    double CallPrice(double S) const;
    double PutPrice(double S) const;

public:
    // parameters (public for consistency with EuropeanOption)
    double K;
    double sig;
    double r;
    double b;
    string optType; // "C" or "P"

public:
    PerpetualAmericanOption();
    PerpetualAmericanOption(const string& optionType);

    double Price(double S) const;
    void toggle();
};

#endif
