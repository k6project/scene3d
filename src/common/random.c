#include "random.h"

static const float UINT32_NORM = 2.3283064370807974e-10f;

void Random_Init(PRNG prng, uint32_t seed)
{
    prng[0] = seed;
}

uint32_t Random_UInt32(PRNG prng)
{
    uint32_t x = prng[0];
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    prng[0] = x;
    return x;
}

float Random_Float(PRNG prng)
{
    return PCGRandomU32(prng) * UINT32_NORM;
}
