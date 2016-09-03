//
// Defaults.c
// This file contains the default initialization procedures
// for every major type.
// TODO > This file should probably go. We should handle these kinds of things elsewhere.
//

#include "Common.h"
#include "GameState.h"

#define PLAYER_COMPONENT_BATCH 1
#define GAME_COMPONENT_BATCH 1

#define GAME_ENEMY_BATCH 1

#define PLAYER_BULLET_BATCH 5
#define GAME_BULLET_BATCH 40

GameState* GME_InitializeDefault( ) {
    GameState* result = malloc( sizeof(*result) );

    result->quit = 0;

    result->world.globalBackground = NULL;
    result->world.height = 0;
    result->world.width = 0;

    PLR_InitializeDefault( &result->player );

    result->components = ManagedListInit( Component, GAME_COMPONENT_BATCH );
    result->enemies = ManagedListInit( Enemy, GAME_ENEMY_BATCH );
    result->bullets = ManagedListInit( Bullet, GAME_BULLET_BATCH );

    return result;
}

void PLR_InitializeDefault( Player* ref ) {
    ref->entity.type = Friend;
    ref->entity.components = ManagedListInit( Component, PLAYER_COMPONENT_BATCH );
    ref->entity.totalmass = 0.0f;
    ref->entity.angacc = 0.0f;
    ref->entity.angvel = 0.0f;
    ref->entity.MOI = 0.0f;

    ref->bullets = ManagedListInit( Bullet, PLAYER_BULLET_BATCH );

    ref->Player_TEX = NULL;
    ref->input.keyboard = NULL;
    ref->input.numkeys = 0;
    ref->input.mousebutton = 0;
    VCT_InitializeDefault( &ref->input.mouseloc );
    CMP_InitializeDefault( &ref->entity.body );
}

void CMP_InitializeDefault( Component* component ) {
    component->ability = None;
    component->health = 0.0f;
    component->strength = 0.0f;
    component->mass = 0.0f;
    component->invinceframes = 0.0f;
    component->frameused = 0.0f;
    VCT_InitializeDefault( &component->relativepos );
    CRC_InitializeDefault( &component->shape );
}

void BLT_InitializeDefault( Bullet* bullet ) {
    bullet->active = 0;
    bullet->damage = 0;
    bullet->lifetime = 0;
    CRC_InitializeDefault( &bullet->shape );
}

void CRC_InitializeDefault( Circle* circ ) {
    circ->rad = 0;
    VCT_InitializeDefault( &circ->pos );
    VCT_InitializeDefault( &circ->vel );
    VCT_InitializeDefault( &circ->acc );
}

void VCT_InitializeDefault( Vector2* vect ) {
    vect->x = 0.0f;
    vect->y = 0.0f;
}
