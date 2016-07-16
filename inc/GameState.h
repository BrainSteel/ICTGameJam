/*

    GameState.h
    Defines global structures for
    important game objects.

 */

#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "SDL.h"

#include "Common.h"
#include "Vector.h"

typedef enum AbilityTypeEnum {
    None,
    Rocket,
    Booster,
    Shield,
    Trash
} AbilityType;

typedef struct CollisionDataStruct {
    Vector2 loc;
    Vector2 oneloc;
    Vector2 twoloc;
    float elapsedtime;
    int didoccur;
} CollisionData;

typedef struct CircleStruct {
    float rad;
    Vector2 pos;
    Vector2 vel;
    Vector2 acc;
} Circle;

typedef struct ComponentStruct {
    Circle shape;
    Vector2 relativepos;
    float health;
    float weight;
    float strength;
    AbilityType ability;
} Component;

typedef struct EntityStruct {
    Component body;
    Component* components;
    int numcomponent;
} Entity;

typedef struct InputStruct {
    int numkeys;
    const Uint8* keyboard;
    Vector2 mouse;
} Input;

typedef struct PlayerStruct {
    Entity entity;
    Input input;
    SDL_Texture* Player_TEX;
} Player;

/*
typedef struct WorldStruct {
    int width;
    int height;

    SDL_Texture* background;
} World;
*/

typedef struct WorldStruct {
    int width, height;
    int centerX, centerY;

    SDL_Rect viewableWorld;

    SDL_Texture* globalBackground;
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
void CMP_InitializeDefault( Component* component );
void CRC_InitializeDefault( Circle* circ );
void VCT_InitializeDefault( Vector2* vect );

int Run( SDL_Window* window, SDL_Renderer* winrend, GameState* game );
void CaptureInput( GameState* state );

void DrawCircle( SDL_Renderer* winrend, Circle circ, int fill );

CollisionData GetCollision( Circle one, Circle two, float elapsedtime );

void UpdateCircle( Circle* circ, float elapsedtime );

void Attach( Player* ref, Component pickup );

#endif
