/*

    Vector.h
    File to declare functions on 2D vectors


 */

#ifndef VECTOR_H
#define VECTOR_H

#include "math.h"
#include "Common.h"

typedef struct Vector2Struct {
    float x, y;
} Vector2;

// Declarations

// Scales the vector by a given factor.
static Vector2 VectorScale( Vector2 vect, float scalefactor );

// Subtracts vector two from vector one.
static inline Vector2 VectorSubtract( Vector2 one, Vector2 two );

// Returns the length of the vector, squared.
static inline float VectorLength2( Vector2 vect );

// Returns the length of the vector.
static inline float VectorLength( Vector2 vect );

// Returns a vector with the same direction as the given vector,
// but with magnitude 1.
static inline Vector2 VectorNormalize( Vector2 vect );

// Computes the dot product of the given vectors.
static inline float VectorDot( Vector2 one, Vector2 two );

// Returns the result of projecting vector src onto vector base.
static inline Vector2 VectorProject( Vector2 src, Vector2 base );

// Definitions

static inline Vector2 VectorScale( Vector2 vect, float scalefactor ) {
    Vector2 result;
    result.x = vect.x * scalefactor;
    result.y = vect.y * scalefactor;
    return result;
}

static inline Vector2 VectorSubtract( Vector2 one, Vector2 two ) {
    Vector2 result;
    result.x = one.x - two.x;
    result.y = one.y - two.y;
    return result;
}

static inline float VectorLength2( Vector2 vect ) {
    return vect.x * vect.x + vect.y * vect.y;
}

static inline float VectorLength( Vector2 vect ) {
    return sqrt( VectorLength2( vect ));
}

inline Vector2 VectorNormalize( Vector2 vect ) {
    float len = VectorLength( vect );
    return VectorScale( vect, 1.0f / len );
}

inline float VectorDot( Vector2 one, Vector2 two ) {
    return one.x * two.x + one.y * two.y;
}

inline Vector2 VectorProject( Vector2 src, Vector2 base ) {
    base = VectorNormalize( base );
    float newlen = VectorDot( src, base );
    return VectorScale( base, newlen );
}

#endif
