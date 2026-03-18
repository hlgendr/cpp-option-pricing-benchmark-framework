#ifndef RESUMECORE_COMMON_BENCHMARKCASE_HPP
#define RESUMECORE_COMMON_BENCHMARKCASE_HPP

#include "ResumeCore/Common/VanillaOptionSpec.hpp"

#include <string>

struct BenchmarkCase
{
    std::string name;
    VanillaOptionSpec spec;
};

#endif