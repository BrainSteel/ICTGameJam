#include "Common.h"
#include "GameState.h"

GameState* GME_InitializeDefault( ) {
    GameState* result = malloc( sizeof(*result) );

    result->quit = 0;

    result->world.globalBackground = NULL;
    result->world.height = 0;
    result->world.width = 0;

    PLR_InitializeDefault( &result->player );

    result->pickups = NULL;
    result->numpickups = 0;

    result->enemies = NULL;
    result->numenemy = 0;

    result->bullets = NULL;
    result->numbullets = 0;
    result->firstinactivebullet = 0;

    return result;
}

void PLR_InitializeDefault( Player* ref ) {
    ref->entity.type = Friend;
    ref->entity.numcomponent = 0;
    ref->entity.components = NULL;
    ref->entity.totalmass = 0.0f;
    ref->entity.angacc = 0.0f;
    ref->entity.angvel = 0.0f;
    ref->entity.MOI = 0.0f;

    ref->firstinactivebullet = 0;
    ref->numbullet = 0;
    ref->playerbullets = NULL;

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
