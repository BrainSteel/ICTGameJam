//
// Entity.c
// Functions to handle drawing, updating, and manipulating entities.
//

#include "Common.h"
#include "Vector.h"
#include "GameState.h"

#define MAX_ENTITY_SPEED 10
#define MAX_ANG_VELOCITY 0.2

void DrawEntity( SDL_Renderer* winrend, Entity* entity, Vector2 offset ) {
    // Maintain previous draw color
    Uint8 r, g, b, a;
    SDL_GetRenderDrawColor( winrend, &r, &g, &b, &a );

    // If friend, draw the body in dark green.
    // Else, red.
    if ( entity->type == Friend ) {
        SDL_SetRenderDrawColor( winrend, 10, 128, 30, SDL_ALPHA_OPAQUE );
    }
    else {
        SDL_SetRenderDrawColor( winrend, 255, 0, 0, SDL_ALPHA_OPAQUE );
    }

    Circle withoffset = entity->body.shape;
    withoffset.pos.x += offset.x;
    withoffset.pos.y += offset.y;
    DrawCircle( winrend, withoffset, 1 );

    // Draw all components
    int i;
    for ( i = 0; i < entity->components.num; i++ ) {
        Component* comp = &entity->components.items[i];
        if (!comp->active)
            continue;
        DrawComponent( winrend, comp, offset );
    }

    SDL_SetRenderDrawColor( winrend, r, g, b, a );
}

void UpdateEntity( GameState* game, Entity* entity, float elapsedtime ) {
    float elapsedtime2 = elapsedtime * elapsedtime;
    Circle* circ = &entity->body.shape;

    circ->pos.x += circ->vel.x * elapsedtime + 0.5 * circ->acc.x * elapsedtime2;
    circ->pos.y += circ->vel.y * elapsedtime + 0.5 * circ->acc.y * elapsedtime2;

    if ( circ->pos.x < 0 ) {
        circ->pos.x = 0;
        circ->vel.x = 0;
    }
    else if ( circ->pos.x > game->world.width - circ->rad) {
        circ->pos.x = game->world.width - circ->rad;
        circ->vel.x = 0;
    }

    if ( circ->pos.y < 0 ) {
        circ->pos.y = 0;
        circ->vel.y = 0;
    }
    else if ( circ->pos.y > game->world.height - circ->rad ) {
        circ->pos.y = game->world.height - circ->rad;
        circ->vel.y = 0;
    }

    float ang = entity->angvel * elapsedtime + 0.5 * entity->angacc * elapsedtime2;
    float cosang = cos( ang );
    float sinang = sin( ang );

    int i;
    for ( i = 0; i < entity->components.num; i++ ) {
        Component* comp = &entity->components.items[i];
        if ( comp->active ) {
            // Rotation matrix
            Vector2 rotated;
            rotated.x = cosang * comp->relativepos.x + sinang * comp->relativepos.y;
            rotated.y = cosang * comp->relativepos.y - sinang * comp->relativepos.x;

            comp->relativepos.x = rotated.x;
            comp->relativepos.y = rotated.y;
            comp->shape.pos.x = circ->pos.x + rotated.x;
            comp->shape.pos.y = circ->pos.y + rotated.y;
        }
    }

    entity->angvel += entity->angacc * elapsedtime;
    if (entity->angvel > MAX_ANG_VELOCITY) {
        entity->angvel = MAX_ANG_VELOCITY;
    }
    else if (entity->angvel < -MAX_ANG_VELOCITY) {
        entity->angvel = -MAX_ANG_VELOCITY;
    }

    circ->vel.x += circ->acc.x * elapsedtime;
    circ->vel.y += circ->acc.y * elapsedtime;

    if ( circ->vel.x * circ->vel.x + circ->vel.y * circ->vel.y > MAX_ENTITY_SPEED * MAX_ENTITY_SPEED ) {
        circ->vel = VectorScale( VectorNormalize( circ->vel ), MAX_ENTITY_SPEED );
    }

    entity->updateleft -= elapsedtime;
}

void Attach( Entity* entity, Component pickup ) {
    Component* comp = ManagedListUseFirstInactive( Component, &entity->components );
    *comp = pickup;
    comp->active = 1;
    comp->relativepos = VectorSubtract( pickup.shape.pos, entity->body.shape.pos );
    float r2 = VectorLength2( comp->relativepos );
    entity->MOI += comp->mass * r2;
    entity->totalmass += comp->mass;
}

void Detach( Entity* entity, int index ) {
    Component* comp = &entity->components.items[index];
    float r2 = VectorLength2( comp->relativepos );
    entity->MOI -= comp->mass * r2;
    entity->totalmass -= comp->mass;
    ManagedListDeactivate( Component, &entity->components, index );
}
