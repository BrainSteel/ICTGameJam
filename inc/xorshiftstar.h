
// Random number generator.
// Reference: https://en.wikipedia.org/wiki/Xorshift

#ifndef XORSHIFTSTAR_H
#define XORSHIFTSTAR_H

#include "stdint.h"

uint64_t xorshift64star( );
void xorshiftseed( uint64_t seed );

#endif
