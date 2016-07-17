
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
void Dummy( Enemy* entity, GameState* game ) { }
void FunctionSelector( Enemy* enemy, GameState* state );

#define ATTRIBUTE_MAX 10
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
    result->random = 0;
    result->running = 0;
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

#define UNINTERRUPT_FRAMES 10
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

static void UseEnemyBoosters( Enemy* enemy, GameState* state, Vector2 direction ) {

    direction = VectorNormalize( direction );

    Vector2 linearforce = {0.0f, 0.0f};
    float moment = 0.0f;

    int i;
    for ( i = 0; i < enemy->entity.numcomponent; i++ ) {
        Component* comp = &enemy->entity.components[i];
        if ( comp->health >= 0 ) {
            linearforce.x += direction.x * comp->strength;
            linearforce.y += direction.y * comp->strength;

            moment -= direction.y * comp->relativepos.x * comp->strength;
            moment += direction.x * comp->relativepos.y * comp->strength;
        }
    }

    enemy->entity.body.shape.acc.x = linearforce.x / enemy->entity.totalmass;
    enemy->entity.body.shape.acc.y = linearforce.y / enemy->entity.totalmass;
    enemy->entity.angacc = moment / enemy->entity.MOI;
}

#define ENEMY_BATCH_SIZE 40
static void AllocEnemyBulletBatch( GameState* state ) {
    Bullet* bullets = realloc( state->bullets, (state->numbullets + ENEMY_BATCH_SIZE) * sizeof(*bullets) );
    if ( bullets ) {
        state->bullets = bullets;
        int i;
        for ( i = state->numbullets; i < state->numbullets + ENEMY_BATCH_SIZE; i++ ) {
            BLT_InitializeDefault( &state->bullets[i] );
        }
        state->numbullets += ENEMY_BATCH_SIZE;
    }
}

static Bullet* UseFirstInactiveEnemyBullet( GameState* state ) {
    if ( state->numbullets <= state->firstinactivebullet ) {
        AllocEnemyBulletBatch( state );
        Bullet* result = &state->bullets[state->firstinactivebullet];
        state->firstinactivebullet++;
        return result;
    }
    else {
        Bullet* result = &state->bullets[state->firstinactivebullet];
        for ( state->firstinactivebullet++; state->firstinactivebullet < state->numbullets; state->firstinactivebullet++ ) {
            if ( !state->bullets[state->firstinactivebullet].active ) {
                break;
            }
        }

        if ( state->firstinactivebullet == state->numbullets ) {
            AllocEnemyBulletBatch( state );
        }

        return result;
    }
}

static void UseEnemyRockets( Enemy* enemy, GameState* state, Vector2 location ) {
    int i;
    for ( i = 0; i < enemy->entity.numcomponent; i++ ) {
        Component* comp = &enemy->entity.components[i];
        if ( comp->ability == Rocket &&
             comp->health > 0 &&
             (state->frames - comp->frameused) > ROCKET_CONSTANT - ROCKET_CONSTANT_FACTOR * comp->strength ) {
            Bullet* newbullet = UseFirstInactiveEnemyBullet( state );
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
static void Running( Enemy* enemy, GameState* state );
static void Random( Enemy* enemy, GameState* state );

void FunctionSelector( Enemy* enemy, GameState* state ) {
#define TIME_FACTOR 60

    int total = enemy->sniping + enemy->rushing +
                enemy->circling + enemy->running + enemy->random;

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
    else if ( enemy->sniping + enemy->rushing + enemy->circling + enemy->running > choice ) {
        enemy->func = Running;
        enemy->phase_duration = enemy->running * TIME_FACTOR;
    }
    else {
        enemy->func = Random;
        enemy->phase_duration = enemy->random * TIME_FACTOR;
    }
}

void Sniping( Enemy* enemy, GameState* state ) {

#define SNIPING_RANGE 350
    Player* player = &state->player;
    Vector2 sub = VectorSubtract( enemy->entity.body.shape.pos, player->entity.body.shape.pos );

    float len = VectorLength( sub );
    if ( len < SNIPING_RANGE ) {
        UseEnemyBoosters( enemy, state, sub );
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
    UseEnemyBoosters(enemy, state, sub);
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

void Running( Enemy* enemy, GameState* state ) {
    Player* player = &state->player;
    Vector2 sub = VectorSubtract( enemy->entity.body.shape.pos, player->entity.body.shape.pos );
    UseEnemyBoosters( enemy, state, sub );
}

void Random( Enemy* enemy, GameState* state ) {
#define RAND_RADIUS 100
    Vector2 random1, random2;
    random1.x = xorshift64star_uniform( 2 * RAND_RADIUS + 1) - RAND_RADIUS;
    random1.y = xorshift64star_uniform( 2 * RAND_RADIUS + 1) - RAND_RADIUS;
    random2.x = xorshift64star_uniform( 2 * RAND_RADIUS + 1) - RAND_RADIUS;
    random2.y = xorshift64star_uniform( 2 * RAND_RADIUS + 1) - RAND_RADIUS;
    UseEnemyBoosters( enemy, state, random1 );
    UseEnemyRockets( enemy, state, random2 );
}

void UpdateEnemy( GameState* state, Enemy* enemy, float elapsedtime ) {

    if ( enemy->phase_duration + enemy->phase_start < state->frames ) {
        enemy->func = FunctionSelector;
    }

    enemy->func( enemy, state );

    UpdateEntity( state, &enemy->entity, elapsedtime );
}
