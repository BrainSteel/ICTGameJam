//
// Enemy.c
// This file contains enemy-specific functionality,
// including their automated decision patterns and
// construction process.
//

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

DefineManagedListFunctions( Enemy );

Component GetComponentFrom( Entity host, AbilityType ability, int strength, Vector2 relativepos );
void Dummy( Enemy* entity, GameState* game ) { }
void FunctionSelector( Enemy* enemy, GameState* state );

#define ENEMY_COMPONENT_BATCH 1
#define ATTRIBUTE_MAX 10
#define ATTRIBUTE_MED 6
#define ATTRIBUTE_LITE 3
void AddEnemy( GameState* game, int totalstrength ) {
    Enemy* result = ManagedListUseFirstInactive( Enemy, &game->enemies );

    int eighthwidth = game->world.width / 8;
    int eighthheight = game->world.height / 8;

    result->entity.type = Foe;
    result->entity.components = ManagedListInit( Component, ENEMY_COMPONENT_BATCH );
    result->entity.angacc = 0;
    result->entity.angvel = 0;
    result->entity.MOI = 0;
    result->entity.totalmass = 5 + xorshift64star_uniform( 10 );
    result->entity.body.mass = result->entity.totalmass;
    result->entity.body.health = totalstrength * 4 + xorshift64star_uniform( totalstrength );
    result->entity.body.strength = 1 + xorshift64star_uniform( 2 );
    result->entity.body.ability = Booster;
    result->entity.body.shape.rad = 15 + xorshift64star_uniform( 10 );
    result->entity.body.shape.pos.x = xorshift64star_uniform( eighthwidth ) + 5 * eighthwidth;
    result->entity.body.shape.pos.y = xorshift64star_uniform( eighthheight ) + 5 * eighthheight;
    result->entity.body.shape.vel.x = -15 - xorshift64star_uniform( 5 );
    result->entity.body.shape.vel.y = -15 - xorshift64star_uniform( 5 );
    result->entity.body.shape.acc.x = 0.0f;
    result->entity.body.shape.acc.y = 0.0f;

    // Attributes
    result->circling = xorshift64star_uniform( ATTRIBUTE_MAX ) + 1;
    result->random = xorshift64star_uniform( ATTRIBUTE_LITE ) + 1;
    result->scavenger = xorshift64star_uniform( ATTRIBUTE_MAX ) + 1;
    result->rushing = xorshift64star_uniform( ATTRIBUTE_MAX ) + 1;
    result->sniping = xorshift64star_uniform( ATTRIBUTE_MAX ) + 1;
    result->func = FunctionSelector;

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

    result->active = 1;
}

static void UseEnemyBoosters( Enemy* enemy, GameState* state, Vector2 direction ) {

    direction = VectorNormalize( direction );

    Vector2 linearforce = {0.0f, 0.0f};
    float moment = 0.0f;

    int i;
    for ( i = 0; i < enemy->entity.components.num; i++ ) {
        Component* comp = &enemy->entity.components.items[i];
        if ( comp->active ) {
            linearforce.x += direction.x * comp->strength;
            linearforce.y += direction.y * comp->strength;

            moment -= direction.y * comp->relativepos.x * comp->strength;
            moment += direction.x * comp->relativepos.y * comp->strength;
        }
    }

    if (enemy->entity.body.ability == Booster) {
        linearforce.x += direction.x * enemy->entity.body.strength;
        linearforce.y += direction.y * enemy->entity.body.strength;
    }

    enemy->entity.body.shape.acc.x = linearforce.x / enemy->entity.totalmass;
    enemy->entity.body.shape.acc.y = linearforce.y / enemy->entity.totalmass;
    enemy->entity.angacc = moment / enemy->entity.MOI;
}

static void UseEnemyRockets( Enemy* enemy, GameState* state, Vector2 location ) {
    int i;
    for ( i = 0; i < enemy->entity.components.num; i++ ) {
        Component* comp = &enemy->entity.components.items[i];
        if ( comp->ability == Rocket &&
             comp->active &&
             (state->frames - comp->frameused) > ROCKET_CONSTANT - ROCKET_CONSTANT_FACTOR * comp->strength ) {
            Bullet* newbullet = ManagedListUseFirstInactive( Bullet, &state->bullets );
            newbullet->active = 1;
            newbullet->damage = PLAYER_BULLET_DAMAGE;
            newbullet->lifetime = PLAYER_BULLET_LIFETIME;
            newbullet->shape.pos = comp->shape.pos;
            Vector2 dir = VectorNormalize( VectorSubtract( location, comp->shape.pos ));
            newbullet->shape.vel = VectorScale( dir, ENEMY_BULLET_SPEED );
            newbullet->shape.vel.x += enemy->entity.body.shape.vel.x;
            newbullet->shape.vel.y += enemy->entity.body.shape.vel.y;

            comp->frameused = state->frames;

            newbullet->shape.rad = PLAYER_BULLET_RADIUS * sqrt( comp->strength );
            newbullet->shape.acc.x = 0.0f;
            newbullet->shape.acc.y = 0.0f;
        }
    }
}

static void Sniping( Enemy* enemy, GameState* state );
static void Rushing( Enemy* enemy, GameState* state );
static void Circling( Enemy* enemy, GameState* state );
static void Scavenger( Enemy* enemy, GameState* state );
static void Random( Enemy* enemy, GameState* state );

void FunctionSelector( Enemy* enemy, GameState* state ) {
#define TIME_FACTOR 60

    int total = enemy->sniping + enemy->rushing +
                enemy->circling + enemy->scavenger + enemy->random;

    enemy->phase_start = state->frames;

    uint64_t choice = xorshift64star_uniform( total );
    if ( enemy->sniping > choice ) {
        enemy->func = Sniping;
        enemy->phase_duration = enemy->sniping * TIME_FACTOR;
    }
    else if ( enemy->sniping + enemy->rushing > choice ) {
        enemy->func = Rushing;
        enemy->phase_duration = enemy->rushing * TIME_FACTOR;
    }
    else if ( enemy->sniping + enemy->rushing + enemy->circling > choice ) {
        enemy->func = Circling;
        enemy->phase_duration = enemy->circling * TIME_FACTOR;
    }
    else if ( enemy->sniping + enemy->rushing + enemy->circling + enemy->scavenger > choice ) {
        enemy->func = Scavenger;
        enemy->phase_duration = enemy->scavenger * TIME_FACTOR;
    }
    else {
        enemy->func = Random;
        enemy->phase_duration = enemy->random * TIME_FACTOR;
    }
}

void Sniping( Enemy* enemy, GameState* state ) {

#define SNIPING_RANGE 500
#define TOO_FAR 800
    Player* player = &state->player;
    Vector2 sub = VectorSubtract( enemy->entity.body.shape.pos, player->entity.body.shape.pos );

    float len = VectorLength( sub );
    if ( len < SNIPING_RANGE ) {
        UseEnemyBoosters( enemy, state, sub );
    }
    if ( len > TOO_FAR ) {
        Vector2 neg;
        neg.x = -sub.x;
        neg.y = -sub.y;
        UseEnemyBoosters( enemy, state, neg );
    }

#define FUTURE_STEPS 2.0
    Circle pcirc = player->entity.body.shape;
    Vector2 future;
    future.x = pcirc.pos.x + FUTURE_STEPS * pcirc.vel.x + 0.5 * FUTURE_STEPS * FUTURE_STEPS * pcirc.acc.x;
    future.y = pcirc.pos.y + FUTURE_STEPS * pcirc.vel.y + 0.5 * FUTURE_STEPS * FUTURE_STEPS * pcirc.acc.y;
    UseEnemyRockets( enemy, state, future );
}

void Rushing( Enemy* enemy, GameState* state ) {
    Player* player = &state->player;
    Vector2 sub = VectorSubtract( player->entity.body.shape.pos, enemy->entity.body.shape.pos );

    #define MIN_DISTANCE 40
    if (VectorLength( sub ) > MIN_DISTANCE ) {
        UseEnemyBoosters(enemy, state, sub);
    }

    UseEnemyRockets(enemy, state, player->entity.body.shape.pos );
}

void Circling( Enemy* enemy, GameState* state ) {
    Player* player = &state->player;
    Vector2 sub = VectorSubtract( player->entity.body.shape.pos, enemy->entity.body.shape.pos );
    Vector2 perp;
    perp.x = -sub.y;
    perp.y = sub.x;
    UseEnemyBoosters( enemy, state, sub );
    UseEnemyRockets(enemy, state, player->entity.body.shape.pos );
}

void Scavenger( Enemy* enemy, GameState* state ) {
    Component* closest = NULL;
    float length = state->world.width;

    int i;
    for ( i = 0; i < state->components.num; i++ ) {
        if ( closest == NULL ) {
            closest = &state->components.items[i];
            length = VectorLength( VectorSubtract( enemy->entity.body.shape.pos, closest->shape.pos ));
            continue;
        }
        else {
            Component* testcomp = &state->components.items[i];
            float testlen = VectorLength( VectorSubtract( enemy->entity.body.shape.pos, testcomp->shape.pos ));
            if (testlen < length) {
                length = testlen;
                closest = testcomp;
            }
        }
    }

    if ( closest )
    {
        Vector2 sub = VectorSubtract( closest->shape.pos, enemy->entity.body.shape.pos );
        UseEnemyBoosters( enemy, state, sub );
    }

#define RANGE 200

    Player* player = &state->player;
    Vector2 pvect = VectorSubtract( enemy->entity.body.shape.pos, player->entity.body.shape.pos );
    if ( VectorLength( pvect ) <= RANGE ) {
        UseEnemyRockets( enemy, state, player->entity.body.shape.pos );
    }
}

void Random( Enemy* enemy, GameState* state ) {
#define RAND_RADIUS 100
    Vector2 random1, random2;
    // TODO > This doesn't make any sense.
    random1.x = xorshift64star_uniform( 2 * RAND_RADIUS + 1) - RAND_RADIUS;
    random1.y = xorshift64star_uniform( 2 * RAND_RADIUS + 1) - RAND_RADIUS;
    random2.x = xorshift64star_uniform( 2 * RAND_RADIUS + 1) - RAND_RADIUS;
    random2.y = xorshift64star_uniform( 2 * RAND_RADIUS + 1) - RAND_RADIUS;
    UseEnemyBoosters( enemy, state, random1 );
    UseEnemyRockets( enemy, state, random2 );
}

void PerformEnemyAction( GameState* state, Enemy* enemy ) {
    if ( enemy->phase_duration + enemy->phase_start < state->frames ) {
        enemy->func = FunctionSelector;
    }

    enemy->func( enemy, state );
}

void FreeEnemy( Enemy* enemy ) {
    ManagedListFree( Component, &enemy->entity.components );
}
