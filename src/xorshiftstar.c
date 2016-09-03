//
// xorshiftstar.c
// Random number generator.
// Reference: https://en.wikipedia.org/wiki/Xorshift
//

#include "stdint.h"
#include "time.h"
#include "xorshiftstar.h"

static uint64_t x; /* The state must be seeded with a nonzero value. */
static uint8_t seeded = 0;

uint64_t xorshift64star( ) {
    // Autoseed to avoid 0 value
    if ( !seeded ) {
        x = time( NULL );
        seeded = 1;
    }

	x ^= x >> 12; // a
	x ^= x << 25; // b
	x ^= x >> 27; // c
	return x * UINT64_C(2685821657736338717);
}

uint64_t xorshift64star_uniform( uint64_t range ) {
    if (range == 0 || range == 1) { return 0; }
    uint64_t cutoff = UINT64_MAX - ( UINT64_MAX % range );
    uint64_t result;
    while (( result = xorshift64star( )) > cutoff ) ;
    return result % range;
}

uint64_t xorshift64star_range( int64_t low, int64_t high ) {
    uint64_t range = high - low + 1;
    return xorshift64star_uniform( range ) + low;
}

void xorshiftseed( uint64_t seed ) {
    x = seed;
    seeded = 1;
}
