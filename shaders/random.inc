#ifndef RANDOM_
#define RANDOM_

#define LCG_MUL 1103515245
#define LCG_INC 12345
#define LCG_MOD 0xffffffff

uint lcgRandom(uint prev, uint range)
{
    return ((LCG_MUL * prev + LCG_INC) % LCG_MOD) % range;
}

#endif // RANDOM_
