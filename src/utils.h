#pragma once

#include "macros.h"

#include <stdint.h>

typedef uint64_t THash;

C_API THash MurmurHash2(const void * key, size_t len);
