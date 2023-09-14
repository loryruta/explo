#include "misc.hpp"

#include <fmt/format.h>

#include <chrono>

std::string explo::stringify_byte_size(size_t byte_size)
{
    float v = float(byte_size);
    size_t i = 0;

    char const *units[]{"", "Kb", "Mb", "Gb", "Tb"};

    while (v > 1000)
    {
        v /= 1024;
        i++;
    }

    std::string out{};
    out += fmt::format("{:0.2f}", v);
    out += units[i];
    return out;
}

uint64_t explo::get_nanos_since_epoch()
{
    using namespace std::chrono;

    return duration_cast<nanoseconds>(system_clock::now().time_since_epoch()).count();
}
