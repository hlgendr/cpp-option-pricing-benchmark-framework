#include <iostream>
#include <fstream>

// defined in TestEuropeanOption.cpp
void RunEuropeanReferenceTests();

// defined in TestPerpetualAmerican.cpp
void RunPerpetualReferenceTests();

int main()
{
    //std::ofstream out("AB_output.txt");
    //std::streambuf* old = std::cout.rdbuf(out.rdbuf());

    std::cout << "================ Reference Suite: European Options (Exact) ================\n";
    RunEuropeanReferenceTests();

    std::cout << "\n================ Reference Suite: Perpetual American Options ================\n";
    RunPerpetualReferenceTests();

    std::cout << "\nAll tests completed.\n";

    //std::cout.rdbuf(old);
    //std::cout << "Saved to AB_output.txt\n";
    return 0;
}
