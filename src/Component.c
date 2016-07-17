#include "GameState.h"

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

#define UNINTERRUPT_FRAMES 10
void AddComponent( GameState* state, Component toAdd ) {
    int i;
    gamelog("checking for good place");
    for ( i = 0; i < state->numpickups; i++ ) {
        if ( state->pickups[i].health <= 0) {
            break;
        }
    }

    if ( i == state->numpickups ) {
        Component* newlist = realloc( state->pickups, (state->numpickups + 1) * sizeof( *newlist ));
        if ( newlist ) {
            state->pickups = newlist;
            state->numpickups++;
        }
        else {
            gamelog( "Failed to allocate memory for a component." );
            return;
        }
    }

    state->pickups[i] = toAdd;
    state->pickups[i].invinceframes = UNINTERRUPT_FRAMES;

    switch( toAdd.ability ) {
        case None:
            FillNoneData( &state->pickups[i] );
            break;
        case Rocket:
            FillRocketData( &state->pickups[i] );
            break;
        case Booster:
            FillBoosterData( &state->pickups[i] );
            break;
        default:
            break;
    }
}
