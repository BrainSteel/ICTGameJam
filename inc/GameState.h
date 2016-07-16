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
    None = 0,
    Rocket,
    Booster,
    NumAbilities
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

typedef struct BulletStruct {
    Circle shape;
    float lifetime;
    int active;
    int damage;
} Bullet;

typedef struct ComponentStruct {
    Circle shape;
    Vector2 relativepos;
    float health;
    float mass;
    float strength;
    uint64_t frameused;
    AbilityType ability;
} Component;

typedef struct EntityStruct {
    Component body;
    Component* components;
    int numcomponent;

    float totalmass;

    float angvel;
    float angacc;
    float MOI;
} Entity;

typedef struct EnemyStruct {
    Entity entity;
    int alive;
} Enemy;

typedef struct InputStruct {
    int numkeys;
    const Uint8* keyboard;
    Vector2 mouseloc;
    Uint32 mousebutton;
} Input;

typedef struct PlayerStruct {
    Entity entity;
    Input input;
    SDL_Texture* Player_TEX;

    Bullet* playerbullets;
    int firstinactivebullet;
    int numbullet;
} Player;

typedef struct WorldStruct {
    int width;
    int height;

    SDL_Texture* background;
} World;

typedef struct ViewableWorldStruct {
    int width, height;
    int centerX, centerY;
} ViewableWorld;

typedef struct GameStateStruct {
    uint64_t frames;

    Player player;
    World world;
    ViewableWorld viewableWorld;

    int quit;

    Circle* bullets;
    int numbullets;

    Enemy* enemies;
    int numenemy;

    Component* pickups;
    int numpickups;
} GameState;

// Initialization functions
GameState* GME_InitializeDefault( );
void PLR_InitializeDefault( Player* ref );
void CMP_InitializeDefault( Component* component );
void BLT_InitializeDefault( Bullet* bullet );
void CRC_InitializeDefault( Circle* circ );
void VCT_InitializeDefault( Vector2* vect );

// Primary game loop
int Run( SDL_Window* window, SDL_Renderer* winrend, GameState* game );

// Circle functions
void DrawCircle( SDL_Renderer* winrend, Circle circ, int fill );
void UpdateCircle( Circle* circ, float elapsedtime );
CollisionData GetCollision( Circle one, Circle two, float elapsedtime );

// Player functions
void CaptureInput( GameState* state );
void DrawPlayer( SDL_Renderer* winrend, Player* player, Vector2 offset );
void UpdatePlayer( Player* player, float elapsedtime );
void Attach( Player* ref, Component pickup );
void PerformAction( GameState* game, AbilityType action );
void FreePlayer( Player* player );

void AddComponent( GameState* state, AbilityType ability, int strengthmax, Vector2* position );

void AddEnemy( GameState* state, int totalstrength );

#endif
