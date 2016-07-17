
#include "xorshiftstar.h"
#include "Common.h"
#include "Vector.h"
#include "GameState.h"

#define NUM_CHOICE 8

static const Vector2 reloptions[8] = {
    { 1, 0 },
    { ROOT2_2, ROOT2_2 },
    { 0, 1 },
    { -ROOT2_2, ROOT2_2 },
    { -1, 0 },
    { -ROOT2_2, -ROOT2_2 },
    { 0, -1 },
    { ROOT2_2, -ROOT2_2 }
};

Component GetComponentFrom( Entity host, AbilityType ability, int strength, Vector2 relativepos );

void AddEnemy( GameState* game, int totalstrength ) {
    Enemy* result = NULL;
    int i;
    for ( i = 0; i < game->numenemy; i++ ) {
        Enemy* test = &game->enemies[i];
        if ( !test->alive ) {
            result = test;
            break;
        }
    }

    // Didn't find a dead enemy to replace.
    if ( result == NULL ) {
        Enemy* newenemies = realloc( game->enemies, sizeof(*newenemies) * ( game->numenemy + 1 ));
        if (!newenemies) {
            gamelog( "Ran out of memory allocating enemy ..." );
            return;
        }
        game->enemies = newenemies;
        game->numenemy++;
        result = &game->enemies[game->numenemy - 1];
    }

    int eighthwidth = game->world.width / 8;
    int eighthheight = game->world.height / 8;

    result->entity.type = Foe;
    result->entity.components = NULL;
    result->entity.numcomponent = 0;
    result->entity.angacc = 0;
    result->entity.angvel = 0;
    result->entity.MOI = 0;
    result->entity.totalmass = 5 + xorshift64star_uniform( 10 );
    result->entity.body.mass = result->entity.totalmass;
    result->entity.body.health = 5 + xorshift64star_uniform( 10 );
    result->entity.body.strength = 1 + xorshift64star_uniform( 2 );
    result->entity.body.ability = Booster;
    result->entity.body.shape.rad = 15 + xorshift64star_uniform( 10 );
    result->entity.body.shape.pos.x = xorshift64star_uniform( eighthwidth ) + 5 * eighthwidth;
    result->entity.body.shape.pos.y = xorshift64star_uniform( eighthheight ) + 5 * eighthheight;
    result->entity.body.shape.vel.x = -15 - xorshift64star_uniform( 5 );
    result->entity.body.shape.vel.y = -15 - xorshift64star_uniform( 5 );
    result->entity.body.shape.acc.x = 0.0f;
    result->entity.body.shape.acc.y = 0.0f;

    // We now have a valid enemy to manipulate.
    int currentstrength = 0;
    int count = 0;

    int options[ NUM_CHOICE ] = { 0 };

    while ( currentstrength < totalstrength ) {
        int max = totalstrength - currentstrength;
        int choices = (max * (max + 1)) >> 1;
        uint64_t choice = xorshift64star_uniform( choices );
        int cutoff = 0;
        int strength = 1;
        while ( max > 0 ) {
            cutoff += max;
            if ( cutoff > choice ) {
                break;
            }
            max--;
            strength++;
        }
        AbilityType ability = xorshift64star_uniform( NumAbilities );
        uint64_t positionchoice = xorshift64star_uniform( NUM_CHOICE );
        Vector2 position = reloptions[ positionchoice ];
        position = VectorScale( position, result->entity.body.shape.rad + options[ positionchoice ]);

        Component newcomponent = GetComponentFrom( result->entity, ability, strength, position );
        options[ positionchoice ] += newcomponent.shape.rad * 2;
        Attach( &result->entity, newcomponent );

        count++;
        currentstrength += strength;
    }

    result->alive = 1;
}

void FillNoneData( Component* component );
void FillRocketData( Component* component );
void FillBoosterData( Component* component );

Component GetComponentFrom( Entity host, AbilityType ability, int strength, Vector2 touchpos ) {
    Component result;
    result.strength = strength;
    result.ability = ability;
    result.frameused = 0;
    result.shape.vel.x = 0;
    result.shape.vel.y = 0;
    result.shape.acc.x = 0;
    result.shape.acc.y = 0;

    switch ( ability ) {
        case None:
            FillNoneData( &result );
            break;
        case Rocket:
            FillRocketData( &result );
            break;
        case Booster:
            FillBoosterData( &result );
            break;
        default:
            break;
    }

    float len = VectorLength( touchpos );
    result.relativepos = VectorScale( touchpos, (len + result.shape.rad) / len );

    result.shape.pos.x = host.body.shape.pos.x + result.relativepos.x;
    result.shape.pos.y = host.body.shape.pos.y + result.relativepos.y;

    return result;
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

void AddComponent( GameState* state, Component toAdd ) {
    int i;
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
