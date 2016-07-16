#ifndef COMMON_H
#define COMMON_H

#ifdef DEBUG
#include "stdio.h"
#define gamelog(...) printf(__VA_ARGS__); printf("\n")
#else
#define gamelog(...)
#endif

#endif
