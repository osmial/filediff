#include "adler32.h"

constexpr uint32_t MOD_ADLER = 65521;

uint32_t adler32(std::string data)
{
    auto a = 1, b = 0;
    for (auto elem : data) {
        a += elem;
        b += a;
    }
    a %= MOD_ADLER;
    b %= MOD_ADLER;

    return (b * 65536) + a;
}
