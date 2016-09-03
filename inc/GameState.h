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
#include "Textures.h"

DeclareManagedListType( Bullet );
DeclareManagedListType( Component );
DeclareManagedListType( Enemy );

typedef struct EnemyStruct Enemy;
typedef struct GameStateStruct GameState;

typedef void (*EnemyFunction)( Enemy* enemy, GameState* state );

typedef enum AbilityTypeEnum {
    None = 0,
    Rocket,
    Booster,

    // TODO > 1. Make it easy to add more abilities
    //      > 2. (future) Add more abilities

    NumAbilities
} AbilityType;

typedef enum EntityTypeEnum {
    // TODO > Evaluate whether we really need this enumeration.
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

    // TODO > (future) Bullets with status effects?
} Bullet;

DefineManagedListType( Bullet );

typedef struct ComponentStruct {
    Circle shape;
    Vector2 relativepos;
    int active;
    float invinceframes;
    float health;
    float mass;
    float strength;
    uint64_t frameused;
    AbilityType ability;
} Component;

DefineManagedListType( Component );

typedef struct EntityStruct {
    Component body;

    ManagedList( Component ) components;

    EntityType type;

    float updateleft;

    float totalmass;

    float angvel;
    float angacc;
    float MOI;
} Entity;

struct EnemyStruct {
    Entity entity;
    int active;

    int sniping;
    int rushing;
    int circling;
    int scavenger;
    int random;

    // TODO > We should really measure this time in seconds, not in frames
    int phase_duration;
    uint64_t phase_start;

    EnemyFunction func;
};

DefineManagedListType( Enemy );

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

    ManagedList( Bullet ) bullets;
    int SCORE;
} Player;

typedef struct WorldStruct {
    int width, height;
    int centerX, centerY;

    SDL_Rect viewableWorld;

    SDL_Texture* globalBackground;
    SDL_Texture* hiddenBackground;
    SDL_Texture* helpScreen;
    SDL_Texture* componentHelpScreen;
    SDL_Texture* gameoverScreen;
} World;

struct GameStateStruct {
    uint64_t frames;

    Player player;
    World world;

    // TODO > I'm thinking all of our textures should go in a structure here.
    //      > Textures should *not* be within world or player. Textures have
    //      > potentially different lifetimes from players or worlds.

    int quit;

    ManagedList( Bullet ) bullets;
    ManagedList( Enemy ) enemies;
    ManagedList( Component ) components;

    TextureSet textures;
};

DeclareManagedListFunctions( Bullet );
DeclareManagedListFunctions( Component );
DeclareManagedListFunctions( Enemy );

// Initialization functions
// TODO > These functions were a good idea when I was tired and didn't want to deal with
//      > uninitialized values. They are no longer a good idea.
GameState* GME_InitializeDefault( );
void PLR_InitializeDefault( Player* ref );
void CMP_InitializeDefault( Component* component );
void BLT_InitializeDefault( Bullet* bullet );
void CRC_InitializeDefault( Circle* circ );
void VCT_InitializeDefault( Vector2* vect );

// Primary game loop
// TODO > (future) More game modes?
// TODO > Is GameState really an argument here? Can Run just construct a game state?
// TODO > We need to break this game loop into several functions like Update, Render, etc.
//      > it's way too long right now.
int Run( SDL_Window* window, SDL_Renderer* winrend, GameState* game );

// Circle functions
void DrawCircle( SDL_Renderer* winrend, Circle circ, int fill );
// TODO > Separate functions for updating velocity and position?
void UpdateCircle( Circle* circ, float elapsedtime );
// TODO > This function is fast but perhaps not accurate enough.
//      > Extend this function to include acceleration, angular velocity,
//      > angular acceleration, etc. by using its current output as the
//      > initial guess of a more accurate newton's method approximation.
//      > Perhaps make this extension optional.
CollisionData GetCollision( Circle one, Circle two, float elapsedtime );

// Component functions
void DrawComponent( SDL_Renderer* winrend, Component* comp, Vector2 offset );
Component GetComponentFrom( Entity host, AbilityType ability, int strength, Vector2 touchpos );
void AddComponent( GameState* state, Component toAdd );
// TODO > Evaluate our current formulas for strength-scaling in components.
//      > we can make more sense than we currently do.
void FillDataForAbility( Component* component );
void FillNoneData( Component* component );
void FillRocketData( Component* component );
void FillBoosterData( Component* component );

// Entity functions
void DrawEntity( SDL_Renderer* winrend, Entity* entity, Vector2 offset );
void UpdateEntity( GameState* game, Entity* entity, float elapsedtime );
void Attach( Entity* entity, Component pickup );
void Detach( Entity* entity, int index );

// Player functions
void CaptureInput( GameState* state );
void PerformAction( GameState* game, AbilityType action );
void FreePlayer( Player* player );

// Enemy functions
void PerformEnemyAction( GameState* game, Enemy* enemy );
void AddEnemy( GameState* state, int totalstrength );
void FreeEnemy( Enemy* enemy );

void PerformEntityComponentCollisions( GameState* game, Entity* entity,
                                       ManagedList( Component )* components, float elapsedtime );
void PerformEntityBulletCollisions( GameState* game, Entity* entity,
                                    ManagedList( Bullet )* bullets, float elapsedtime );

#endif
