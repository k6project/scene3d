#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef uint32_t PRNG[4];

void Random_Init(PRNG prng, uint32_t seed);

uint32_t Random_UInt32(PRNG prng);

float Random_Float(PRNG prng);

#ifdef __cplusplus
}
#endif
