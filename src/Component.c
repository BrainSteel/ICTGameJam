//
// Component.c
// Functions to manage creation of components.
//

#include "Common.h"
#include "GameState.h"

DefineManagedListFunctions( Component );

void DrawComponent( SDL_Renderer* winrend, Component* comp, Vector2 offset ) {
    if (!comp->active)
        return;

    // TODO > What about using actual textures here?
    switch ( comp->ability ) {
        case None:
            SDL_SetRenderDrawColor( winrend, 255, 255, 255, SDL_ALPHA_OPAQUE );
            break;
        case Rocket:
            SDL_SetRenderDrawColor( winrend, 0, 0, 255, SDL_ALPHA_OPAQUE );
            break;
        case Booster:
            SDL_SetRenderDrawColor( winrend, 0, 255, 0, SDL_ALPHA_OPAQUE );
            break;
        default:
            SDL_SetRenderDrawColor( winrend, 255, 255, 255, SDL_ALPHA_OPAQUE );
    }

    Circle withoffset = comp->shape;
    withoffset.pos.x += offset.x;
    withoffset.pos.y += offset.y;
    DrawCircle( winrend, withoffset, 1 );
}

Component GetComponentFrom( Entity host, AbilityType ability, int strength, Vector2 touchpos ) {
    Component result;
    result.strength = strength;
    result.ability = ability;
    result.frameused = 0;
    result.shape.vel.x = 0;
    result.shape.vel.y = 0;
    result.shape.acc.x = 0;
    result.shape.acc.y = 0;

    FillDataForAbility( &result );

    float len = VectorLength( touchpos );
    result.relativepos = VectorScale( touchpos, (len + result.shape.rad) / len );

    result.shape.pos.x = host.body.shape.pos.x + result.relativepos.x;
    result.shape.pos.y = host.body.shape.pos.y + result.relativepos.y;

    return result;
}

void FillDataForAbility( Component* component ) {
    switch (component->ability) {
        case None:
            FillNoneData(component);
            break;
        case Rocket:
            FillRocketData(component);
            break;
        case Booster:
            FillBoosterData(component);
            break;
        default:
            break;
    }
}

// TODO > Most of these functions are entirely arbitrary
//      > we could make more reasonable formulae.
void FillNoneData( Component* component ) {
    component->ability = None;
    component->mass = 1.0;
    component->health = 2 + 8 * component->strength;
    component->shape.rad = 6 + 4 * component->strength;
}

void FillRocketData( Component* component ) {
    component->ability = Rocket;
    component->mass = 1.6 + 0.4 * component->strength;
    component->health = 5 + 5 * component->strength;
    component->shape.rad = 10 * component->strength;
}

void FillBoosterData( Component* component ) {
    component->ability = Booster;
    component->mass = 1.5 + 0.2 * component->strength;
    component->health = 8 + 2 * component->strength;
    component->shape.rad = 5 + 5 * component->strength;
}

#define UNINTERRUPT_FRAMES 30
void AddComponent( GameState* state, Component toAdd ) {
    Component* comp = ManagedListUseFirstInactive(Component, &state->components);
    *comp = toAdd;
    comp->invinceframes = UNINTERRUPT_FRAMES;

    switch( toAdd.ability ) {
        case None:
            FillNoneData( comp );
            break;
        case Rocket:
            FillRocketData( comp );
            break;
        case Booster:
            FillBoosterData( comp );
            break;
        default:
            break;
    }
}
