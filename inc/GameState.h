/*

    GameState.h
    Defines global structures for
    important game objects.

 */

#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "SDL.h"

typedef enum AbilityTypeEnum {
    None,
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
    Vector2 acc;
} Circle;

typedef struct ComponentStruct {
    float health;
    float weight;
    float strength;
    Circle shape;
    AbilityType ability;
} Component;

typedef struct EntityStruct {
    Component body;
    Component* components;
    int component_num;
} Entity;

typedef struct InputStruct {
    int numkeys;
    const Uint8* keyboard;
    Vector2 mouse;
} Input;

typedef struct PlayerStruct {
    Entity entity;
    Input input;
} Player;

typedef struct WorldStruct {
    int width;
    int height;

    SDL_Texture* back;
} World;

typedef struct GameStateStruct {
    Player player;
    World world;

    int quit;

    Entity* enemies;
    int numenemy;

    Component* pickups;
    int numpickups;
} GameState;

int run( );

GameState* GME_InitializeDefault( );
void PLR_InitializeDefault( Player* ref );
void CRC_InitializeDefault( Circle* circ );
void VCT_InitializeDefault( Vector2* vect );

int Run( SDL_Window* window, SDL_Renderer* winrend );
void CaptureInput( GameState* state );

Vector2 GetCollision( Circle one, Circle two );
Vector2 Attach( Player* ref, Component pickup, Vector2 relplayerloc);

#endif
