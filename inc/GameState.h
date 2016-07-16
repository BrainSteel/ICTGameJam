/*

    GameState.h
    Defines global structures for
    important game objects.

 */

#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "SDL.h"

typedef enum AbilityTypeEnum {
    Rocket,
    Booster,
    Shield,
    Trash
} AbilityType;

typedef struct Vector2Struct {
    float x, y;
} Vector2;

typedef struct CircleStruct {
    float rad;
    Vector2 pos;
    Vector2 vel;
} Circle;

typedef struct ComponentStruct {
    float health;
    float weight;
    float strength;
    Vector2 position;
    AbilityType ability;
} Component;

typedef struct EntityStruct {
    Component body;
    Component* components;
    int component_num;
} Entity;

typedef struct PlayerStruct {
    Entity object;

} Player;

typedef struct WorldStruct {
    int width;
    int height;

    SDL_Texture* back;
} World;

typedef struct GameStateStruct {
    Player player;
    World world;
} GameState;


#endif
