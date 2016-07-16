#include "Common.h"
#include "GameState.h"

GameState* GME_InitializeDefault( ) {
    GameState* result = malloc( sizeof(*result) );

    result->quit = 0;

    result->world.background = NULL;
    result->world.height = 0;
    result->world.width = 0;

    PLR_InitializeDefault( &result->player );

    result->pickups = NULL;
    result->numpickups = 0;

    result->enemies = NULL;
    result->numenemy = 0;

    return result;
}

void PLR_InitializeDefault( Player* ref ) {
    ref->entity.component_num = 0;
    ref->entity.components = NULL;
    ref->entity.body.ability = None;
    ref->entity.body.health = 0.0f;
    ref->entity.body.strength = 0.0f;
    ref->entity.body.weight = 0.0f;

    CRC_InitializeDefault( &ref->entity.body.shape );
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
