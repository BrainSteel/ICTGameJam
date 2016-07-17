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

typedef struct EnemyStruct Enemy;
typedef struct GameStateStruct GameState;

typedef void (*EnemyFunction)( Enemy* enemy, GameState* state );

typedef enum AbilityTypeEnum {
    None = 0,
    Rocket,
    Booster,
    NumAbilities
} AbilityType;

typedef enum EntityTypeEnum {
    Friend,
    Foe
} EntityType;

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
    float invinceframes;
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

    EntityType type;

    float totalmass;

    float angvel;
    float angacc;
    float MOI;
} Entity;

struct EnemyStruct {
    Entity entity;
    int alive;

    int sniping;
    int rushing;
    int circling;
    int running;
    int random;

    int phase_duration;
    uint64_t phase_start;

    EnemyFunction func;
};

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
    int width, height;
    int centerX, centerY;

    SDL_Rect viewableWorld;

    SDL_Texture* globalBackground;
    SDL_Texture* hiddenBackground;
    SDL_Texture* helpScreen;
} World;

struct GameStateStruct {
    uint64_t frames;

    Player player;
    World world;

    int quit;

    Bullet* bullets;
    int numbullets;
    int firstinactivebullet;

    Enemy* enemies;
    int numenemy;

    Component* pickups;
    int numpickups;
};

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
void DrawEntity( SDL_Renderer* winrend, Entity* entity, Vector2 offset );
void DrawComponent( SDL_Renderer* winrend, Component* comp, Vector2 offset );
void UpdateEntity( GameState* game, Entity* entity, float elapsedtime );
void UpdatePlayer( GameState* game, float elapsedtime );
void UpdateEnemy( GameState* game, Enemy* enemy, float elapsedtime );
void Attach( Entity* entity, Component pickup );
void PerformAction( GameState* game, AbilityType action );
void FreePlayer( Player* player );

void AddComponent( GameState* state, Component toAdd );

void AddEnemy( GameState* state, int totalstrength );

#endif
