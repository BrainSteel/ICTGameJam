#ifndef COMMON_H
#define COMMON_H

#ifdef DEBUG
#include "stdio.h"
#define gamelog(...) printf(__VA_ARGS__); printf("\n")
#else
#define gamelog(...)
#endif

#define ROCKET_CONSTANT 10
#define ROCKET_CONSTANT_FACTOR 1
#define PLAYER_BULLET_LIFETIME 100
#define PLAYER_BULLET_SPEED 16
#define PLAYER_BULLET_DAMAGE 3
#define PLAYER_BULLET_RADIUS 5
#define ENEMY_BULLET_LIFETIME 100
#define ENEMY_BULLET_SPEED 13
#define ENEMY_BULLET_DAMAGE 4
#define ENEMY_BULLET_RADIUS 8

#endif
